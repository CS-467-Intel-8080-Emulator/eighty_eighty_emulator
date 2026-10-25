# 8080 Spec Test Suite (`tests/spec`)

A comprehensive spec-based "gray-box" CPU test suite written against official
Intel documentation. It tests all 256 opcodes for result, flags and cycle count.
Two manuals are used, plus a later Intel manual for a few AC flag rules (see
[Sources](#sources)):

- [_Intel 8080 Microcomputer Systems User's Manual_](https://archive.org/download/bitsavers_intelMCS80ocomputerSystemsUsersManual197509_43049640/98-153B_Intel_8080_Microcomputer_Systems_Users_Manual_197509.pdf)
  (98-153B, Sept. 1975): the primary source for instruction behavior, flags,
  cycle counts, and interrupt, reset and halt behavior.
- [_Intel 8080 Assembly Language Programming Manual_](https://archive.org/download/intel-8080-assembly-language-programming-manual-1975/Intel%208080%20Assembly%20Language%20Programming%20Manual%20%281975%29.pdf)
  (1975, Rev. B), called "ALP" in the tests: the source of the worked examples,
  and cited where it is more explicit than the User's Manual.
- [_Intel 8080/8085 Assembly Language Programming Manual_](https://archive.org/download/bitsavers_intelISISIssemblyLanguageProgrammingManualMay81_7150831/9800301-04_8080_8085_Assembly_Language_Programming_Manual_May81.pdf)
  (9800301-04, May 1981), called "8080/8085 ALP" here: cited only for the AC
  flag, where the two 1975 manuals are unclear (the logical instructions and
  subtraction).

This suite is independent of the existing unit tests in `tests/`. It shares no
helpers or structure with them, apart from a few borrowed patterns (e.g. the
`CPUTestAccess` friend struct for register/flag access). CTest registers each
test with a `spec:` prefix (e.g. `spec:PUSH PSW (0xF5)`).

These are core-only tests: one CPU, flat memory, and a fake I/O device that
implements `IOBus`. Future tests for the whole machine (memory map, real I/O
devices, ROMs, frontend) belong in a separate integration suite, e.g.
`tests/integration/`.

## Building and running

The suite builds as its own executable, `cpu_spec_tests`. Any `.cpp` file in
this folder is picked up automatically (CMake
`file(GLOB ... CONFIGURE_DEPENDS)`).

```sh
cmake --build build --config Release --parallel
ctest --test-dir build -R spec: -C Release --output-on-failure  # through ctest
./build/cpu_spec_tests                                          # directly
./build/cpu_spec_tests "[branch]"                               # one group, by tag
./build/cpu_spec_tests "PUSH PSW (0xF5)"                        # one test, by name
```

Some tests fail until the CPU implements the opcodes they cover; see
[Known failures](#known-failures).

## Design

### Gray box

Tests only use the `CPU`'s public interface: `step()`, `interrupt()`, and
`set_io()`. The one exception is reading and setting registers, which goes
through the `CPUTestAccess` friend struct. Tests never call helpers inside the
CPU or check internal fields such as `halted` or `interrupts_enabled`; instead,
tests observe behavior: a halted CPU doesn't run the next instruction, and a
disabled interrupt is ignored. This keeps tests independent of implementation
details.

Expected results come from the manuals. Where the manuals don't state a result
directly, the value the tests expect and what it is based on are listed under
[Assumptions](#assumptions).

### The harness: `test_machine.hpp`

The harness is one header:

- **`CpuState`**: a struct representing CPU registers, flags, SP, and PC. The
  `==` operator compares only the five real flags (S Z AC P CY), and the `<<`
  operator prints a readable line for failure messages, e.g.
  `A=9C B=12 C=34 D=56 E=78 H=20 L=50 SP=2400 PC=0840 F=00`.
- **`CpuState::initial(flags = 0)`**: returns a generic starting state
  containing unique values in each register.
    - The flags are whatever you pass in (clear by default), e.g.
      `CpuState::initial(flag::ALL)` or `CpuState::initial(flag::Z | flag::CY)`.
    - The values returned by `initial()` are technically arbitrary; tests don't
      depend on any particular initial register values. However, initial state
      should follow four conventions: all 8-bit values should be unique, PC's
      two bytes should be different from each other, PC, SP, BC, DE and HL
      should point at separate areas of memory, and those areas should stay
      clear of the fixed addresses tests use. These properties make the default
      state good at catching mistakes. If `MOV B,C` copied from D by mistake, C
      and D holding the same value would hide the bug.
        - This is not a rule for tests; a test may set equal registers or
          overlapping addresses on purpose. Same-register cases such as `ADD A`,
          `MOV B,B` and `DAD H` have their own tests, and `overlap.cpp` covers
          code, data and stack sharing memory.
- **`RecordingIO`**: a fake I/O device (implements `IOBus`).
    - `inputs[port]` sets what IN returns.
    - `reads` and `writes` are vectors that record every IN and OUT in order.
- **`TestMachine`**: minimal wrapper around a CPU wired to its own flat `Memory`
  and a test `RecordingIO` instance. No concept of "frames"; each call to
  `step()` executes a single instruction with a variable cycle count depending
  on the instruction.
    - `start(state, {bytes})` sets the registers and loads the bytes at
      `state.pc`.
    - `step()` runs one instruction and returns its cycle count.
    - `run(state, {bytes})` is `start()` followed by `step()`.
    - `interrupt(n)` sends a hardware interrupt to the CPU with RST n.
    - `state()`, `read(addr)`, `write(addr, value)` and `io()` let tests set up
      and inspect the machine.
    - `write_word(addr, value)` writes a 16-bit value in 8080 byte order (low
      byte at `addr`). It is for **setup only**, such as putting a return
      address on the stack before a RET test. Results are always checked byte by
      byte with `read()` (see "Check stack contents" below). It is deliberately
      separate from `Memory`'s own 16-bit helpers, so tests never rely on the
      emulator's own internal byte logic.
    - A new `TestMachine`'s memory is assumed to be all zeros. A few tests rely
      on this: STA's "neighbors untouched" check, Ccc's "stack untouched" check,
      the "interrupts disabled at power-on" stack check, and the block copy's
      "nothing past the end" check.

### How a test is written

```cpp
TestMachine m;
CpuState start = CpuState::initial(ALL);  // every flag set

CHECK(m.run(start, {0x41}) == 5);         // MOV B,C: 5 cycles

CpuState expected = start;
expected.b = start.c;                     // B gets whatever C held
expected.pc += 1;
CHECK(m.state() == expected);             // every register, SP, PC and flag
```

#### Conventions:

- **Don't depend on `initial()`'s values.** A test either derives its expected
  values in terms of the start state (e.g. `expected.b = start.c`,
  `expected.sp = start.sp - 2`, a return address of `start.pc + 3`), or sets its
  own values explicitly (e.g. `start.a = 0x6C; start.d = 0x2E;` then expects
  `0x9A`). ALU results are never computed with C++ arithmetic in the test, since
  that would mean re-implementing the flag logic that is being tested; instead,
  inputs and outputs are both hardcoded into the test as a worked example.
- **Compare the whole state.** Each opcode's main test builds the full expected
  state and compares all of it, not just the parts the manual says are affected,
  so a side effect on any register, SP, PC or flag fails the test. A few extra
  tests that target one detail check only that detail, since the main test
  already covers the rest: the PUSH PSW flag-byte sections, the POP PSW flag
  sections, the interrupt vector loop, the I/O port 00/FF test and the MOV
  cycle-count test. The multi-instruction tests in `programs.cpp` also check
  only the end results that matter.
- **Start flags opposite to the expected result.**
    - Instructions that shouldn't touch flags start with all flags set. A stray
      flag update (e.g. code copied from ADD that still sets flags from the
      result) can never leave them all set, because no result has both S (bit 7
      = 1) and Z (result = 0). So any stray update changes at least one flag and
      the test fails. Starting with all flags clear would miss it whenever the
      result is positive, non-zero and has odd parity. The trade-off is that a
      bug that only _sets_ a flag isn't caught from an all-set start. A few
      tests also start from all clear (e.g. CMA, the rotates), but most
      flag-free instructions are only tested from all set.
    - Instructions that set flags start with each flag in the state that would
      expose a mistake. For example, ADD starts with CY=1 to prove it ignores
      the incoming carry.
- **Isolate the tested flag in conditional branches.** For Jcc, Ccc and Rcc, the
  "true" case sets only the tested flag to its jump-taken value and every other
  flag the opposite way; the "false" case does the reverse. A CPU that checks
  the wrong flag then gets the wrong answer, instead of passing because the
  other flags happened to match. For example, JZ is tested with Z=1 and
  everything else 0, so a JZ that checks CY by mistake doesn't jump.
- **Check stack contents byte by byte.** PUSH, CALL, RST and most interrupt
  tests read each pushed byte to confirm the high byte is at SP-1 and the low
  byte at SP-2, so the manual's byte order is visible in the test and a failure
  shows the individual bytes. The one exception is setup: tests that need a
  return address already on the stack (RET, Rcc, undocumented RET) may place it
  with `write_word()`, because that is input, not a result being checked. POP,
  XTHL and POP PSW tests write their stack bytes one at a time, since which byte
  lands in which register is what they test.
- **Give each opcode its own test.** Every opcode has its own `TEST_CASE` or
  `SECTION`, named with its mnemonic and hex value, so a failure points straight
  at one opcode.
- **Use the manual's examples.** Every ALP worked example that shows an
  arithmetic or logical result is its own test or section, with the page in the
  name, e.g. `ADD example: 6C + 2E = 9A (ALP p. 18)`; see
  [Worked examples](#worked-examples-alp). That covers the instruction
  descriptions (pp. 5–29) and the multibyte and decimal arithmetic examples in
  the programming chapter (pp. 55–57); the one exception is the multiplication
  example (pp. 53–54). Many of these tests check more than the example shows.
  For example, the DCR M example gives only the result, but the test also checks
  the flags. Tests whose example gives no flags say "flags not stated" in the
  title; tests whose example gives only some flags name those flags in the title
  (e.g. the CMP and CPI examples).

## Coverage

| File                   | Opcodes                                    | Test cases | Contents                                                                                                                                                        |
| ---------------------- | ------------------------------------------ | ---------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `data_transfer.cpp`    | 84                                         | 22         | MOV (63), MVI, LXI, LDA/STA, LHLD/SHLD (with the FFFF→0000 wraparound), LDAX/STAX, XCHG                                                                         |
| `arithmetic.cpp`       | 65                                         | 29         | ADD/ADC/SUB/SBB (each register), ADI/ACI/SUI/SBI, INR/DCR with edge cases, INX/DCX, DAD, DAA                                                                    |
| `logical.cpp`          | 43                                         | 23         | ANA/XRA/ORA/CMP (each register), ANI/XRI/ORI/CPI, RLC/RRC/RAL/RAR, CMA/CMC/STC                                                                                  |
| `branch.cpp`           | 36                                         | 40         | JMP, Jcc ×8, CALL, Ccc ×8, RET, Rcc ×8, RST 0–7, PCHL. Each condition is tested both true and false; stack wraparound for CALL and RET                          |
| `stack_io_control.cpp` | 16                                         | 29         | PUSH/POP (incl. the PSW flag byte layout and stack wraparound), XTHL, SPHL, IN/OUT, NOP, EI, DI, HLT, hardware interrupt behavior                               |
| `undocumented.cpp`     | 12                                         | 13         | Undocumented copies: 08–38 → NOP, CB → JMP, D9 → RET, DD/ED/FD → CALL                                                                                           |
| `programs.cpp`         | —                                          | 11         | Multi-instruction programs: loops, 16-bit add with carry, block copy, subroutines, stack swap, interrupt handler; ALP multibyte and decimal arithmetic examples |
| `overlap.cpp`          | —                                          | 5          | Code, data and stack sharing memory: self-modifying code (opcode and data byte), PUSH over code, instructions reading their own bytes                           |
| **Total**              | **256** (244 documented + 12 undocumented) | **172**    |                                                                                                                                                                 |

### Opcode tests

- **Result:** destination register, memory, SP and PC.
- **Flags:** all five, including flags that should stay unchanged.
- **Cycle count:** the value from the manual. Where it depends on the operand
  (register vs. M) or on the outcome (taken vs. not taken), every case is
  checked.
- **Memory side effects:** for stores and stack operations.
- **I/O side effects:** for IN and OUT (port numbers and values, in order).

### Interrupt behavior

- Interrupts are disabled at power-on.
- EI takes effect after the _next_ instruction, so an interrupt right after EI
  is ignored.
- DI blocks interrupts.
- An accepted interrupt pushes PC, jumps to n×8, takes 11 cycles, and disables
  further interrupts.
- An interrupt wakes a halted CPU and pushes the address after HLT.
- HLT with interrupts disabled stays halted.

### Not covered

- Cycle counts while halted and what `interrupt()` returns when the interrupt is
  ignored. The manual doesn't define either; they are emulator design choices.
- `IN` with no I/O device attached. The harness always attaches one.
- What the CPU keeps in the PSW flag byte's unused bits after POP PSW. PUSH PSW
  is checked for bit 1 = 1 and bits 3 and 5 = 0 (p. 4-13); POP PSW tests check
  only the five real flags.
- Timing between steps, such as interrupts arriving partway through an
  instruction. The emulator only interrupts between steps.
- Full CPU test ROMs (TST8080, 8080EXM, etc.). Those are separate work.

## Assumptions

Most expected values come straight from the manuals; the tables under
[Sources](#sources) cite the page for each. The cases below are the exceptions:
each says what the tests expect and what that is based on.

### Settled by a later Intel manual

The 1975 manuals are unclear or disagree; the 1981 8080/8085 ALP states the
rule.

1. **AC after ANA/ANI.** The tests expect AC = bit 3 of (A OR operand). The two
   1975 manuals don't say this:
    - User's Manual: ANA "affects" AC (p. 4-8); ANI clears it (p. 4-9).
    - ALP: AC isn't listed for ANA (p. 19) or ANI (p. 28).

    Intel's later 8080/8085 ALP states it (p. 1-12): "The 8080 logical AND
    instructions set the flag to reflect the logical OR of bit 3 of the values
    involved in the AND operation." It adds that the 8085 always sets AC
    instead.

    For ANI this contradicts the User's Manual, which says "The CY and AC flags
    are cleared" (p. 4-9).

2. **AC after XRA/ORA/XRI/ORI.** The tests expect AC cleared. This matches the
   User's Manual (p. 4-9) and the 8080/8085 ALP, which says each of these resets
   "the carry and auxiliary carry flags" (pp. 3-39, 3-40, 3-66, 3-67). The 1975
   ALP manual is inconsistent: it lists AC as affected for XRA (p. 19) but not
   for ORA, XRI or ORI (pp. 20, 29).

### Stated only in the ALP

The User's Manual is vague here; the 1975 ALP states the rule.

3. **DAA flags.**
    - The tests follow the two-step rule (User's Manual p. 4-8, ALP p. 15).
    - CY is only ever set by DAA, never cleared; a CY that was already set stays
      set. The User's Manual only says "all flags are affected", but the ALP
      manual (p. 16) says CY is set if step 2 carries and "otherwise it is
      unaffected".
    - AC is the carry out of bit 3 from the low-nibble adjustment (ALP p. 15).

### Derived from the manuals' general rules or examples

No manual states these directly; the tests follow the general rules, the worked
examples or the hardware description.

4. **AC after subtraction (SUB/SBB/SUI/SBI/CMP/CPI/DCR).** The tests compute A −
   operand as A + (NOT operand) + 1 (+ 0 instead of + 1 for SBB/SBI with a
   borrow in) and take AC from the carry out of bit 3 of that addition. Put
   another way, the tests expect AC _set_ when there is **no** borrow out of
   bit 3. None of the manuals states this as a general rule, but the ALP
   manual's examples follow it both ways: SUB A (p. 18) and SBB (p. 19) have no
   borrow out of bit 3 and set AC, while SUI 00−01 (pp. 27–28) and both SBI
   examples (p. 28) borrow out of bit 3 and clear AC. The 8080/8085 ALP's SUB A
   example (p. 3-64) comes closest to stating it: SUB "complements the carry
   generated by the two's complement addition to form a 'borrow' flag", while AC
   is set by "a carry out of bit 3" of that same addition. In other words, CY is
   inverted to mean borrow, but AC is not.
    - With a borrow-in (SBB/SBI), AC is set when the low nibble of A > the low
      nibble of the operand.
    - For DCR, AC is set unless the result's low nibble is F. No example in any
      of the manuals shows DCR's AC.
    - The exact form matters when the operand's low nibble is 0. Adding the
      finished two's-complement byte instead (e.g. 9C + E0 for 9C − 20) gives no
      carry out of bit 3, so AC would be 0; the tests expect 1, since nothing is
      borrowed. The 8080/8085 ALP's SUB A example (p. 3-64) writes the one's
      complement and the "add one" as separate steps, which matches the tests.
      The 1975 ALP draws its CPI example as 4A + C0 (p. 29) but doesn't state
      AC, so it doesn't settle this. Tests that depend on it: SUB H, SUB L, CMP
      H, CMP L and the CPI example.
5. **Address wraparound.** LHLD/SHLD at FFFF, CALL/RET and PUSH/POP with the
   stack crossing 0000, and INX/DCX wrapping (other than INX SP FFFF, ALP p. 24)
   are expected to wrap around to the other end of memory. None of these is
   stated; it follows from the 16-bit address space (User's Manual p. 4-1).
6. **Flags the ALP examples don't state.** Many worked examples give only the
   result, or only some flags, so the test's other flags come from the general
   rules. The test titles mark these ("flags not stated", or the flags the
   manual names). For ANA and ANI, AC follows assumption 1. The CMP "equal" case
   isn't one of the manual's three CMP examples.
7. **Interrupt cycle count (11).** The tests expect an accepted interrupt to
   cost the same as an RST instruction (11 states, p. 4-12). The manual gives no
   separate count for it, but p. 2-11 describes the interrupt machine cycle as
   "otherwise indistinguishable from an ordinary FETCH machine cycle. The
   processor itself takes no further special action": the CPU fetches the RST
   from the interrupting device instead of from memory, then runs it as usual.
   Wait states that the interrupting hardware adds through the READY line are
   outside the CPU and aren't modeled.

### From an outside source

8. **Undocumented opcodes.** These aren't in the manual at all. The tests assume
   each one behaves exactly like its documented copy, cycle count included (NOP
   4, JMP 10, RET 10, CALL 17). No Intel document covers them; the mapping
   matches the widely used
   [pastraiser 8080 opcode table](https://pastraiser.com/cpu/i8080/i8080_opcodes.html)
   ([archived copy](https://web.archive.org/web/20261013110514/https://www.pastraiser.com/cpu/i8080/i8080_opcodes.html)),
   which lists them as "alternative opcodes for existing instructions". You
   could also argue for halting or logging on these opcodes instead of executing
   them and update these tests accordingly.

## Known failures

As of writing and confirmed by a run on 2026-10-24: of 172 test cases, 143 pass
and 29 fail. The failing tests cover opcodes the CPU doesn't implement yet or
gets wrong, and will keep failing until it does. Everything else passes,
including all arithmetic and logical flag tests (AC rules included), branches,
RST, I/O, interrupts, and the undocumented JMP (CB).

| Failing tests (29)                                                                                                                     | Why                                                                                                     |
| -------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| `MOV r1,r2 takes 5 cycles`                                                                                                             | `op_mov` returns 7 for every form; the manual says 5 for register-to-register (7 only for forms with M) |
| PUSH, POP, XTHL, SPHL (13 tests, including the `PUSH PSW then POP PSW` round trip), the "swap pairs" program and the PUSH overlap test | Not implemented. These opcodes fall through to the branching placeholder, which does nothing            |
| Undocumented NOPs 08–38 (7 tests) and "Undocumented NOP does not stop the CPU"                                                         | Mapped to `op_unimplemented`, which halts the CPU                                                       |
| Undocumented D9, DD, ED, FD (4 tests)                                                                                                  | Fall through to the branching placeholder                                                               |
| `XCHG (0xEB)`                                                                                                                          | `cpu.cpp` maps 0xEB to `op_xchg`, but the 0xC0–0xFF placeholder loop that runs later overwrites it      |

When one of these opcodes is implemented, update this table.

## Adding tests

1. Add a `TEST_CASE` to the file for the right group, or make a new `.cpp` file
   in this folder. New files are picked up on the next build.
2. Start from `CpuState::initial(flags)`, set any inputs whose values matter,
   and build `expected` from `start`, changing only what the instruction should
   change. Never hard-code a value that came from `initial()`.
3. Name the test with the mnemonic and hex opcode, and tag it with its group
   (`[arithmetic]`, `[branch]`, ...).
4. Take expected values and cycle counts from the manuals, not from `cpu.cpp`.

### Where helpers go

- **Used by more than one file:** put it in `test_machine.hpp`.
- **Used by one file only:** keep it in that file's anonymous namespace (e.g.
  `check_jcc()` in `branch.cpp`, `check_call()` in `undocumented.cpp`,
  `run_to()` in `programs.cpp`).
- **Needed by another suite** (test ROMs, Machine tests): move `TestMachine` and
  `RecordingIO` to a shared folder such as `tests/support/` at that point,
  rather than copying them.

## Sources

Where each test's expected behavior comes from. Page numbers are the printed
page numbers in each manual.

- **User's Manual** (_Intel 8080 Microcomputer Systems User's Manual_, 98-153B,
  Sept. 1975): the primary source, and the one cited in the tables below unless
  a row says "ALP". Almost everything comes from Chapter 4, "Instruction Set"
  (pp. 4-1 to 4-15). Interrupt and halt behavior comes from Chapter 2 and the
  8080A data sheet in Chapter 5.
- **ALP** (_Intel 8080 Assembly Language Programming Manual_, 1975, Rev. B):
  instructions are described in Chapter 2. Its flag lists for the logical
  instructions are inconsistent (see assumptions 1 and 2 under
  [Assumptions](#assumptions)), so they aren't used.
- **8080/8085 ALP** (_Intel 8080/8085 Assembly Language Programming Manual_,
  9800301-04, May 1981): cited only for the AC flag: after the logical
  instructions (p. 1-12 for ANA/ANI; pp. 3-39, 3-40, 3-66, 3-67 for ORA, ORI,
  XRA, XRI) and after subtraction (p. 3-64, the SUB A example).

All cycle counts in the suite match both manuals: the User's Manual's "States"
line for each instruction and its summary table on p. 4-15, and ALP Appendix B
(pp. xvi–xviii).

### Worked examples (ALP)

| Test                                   | Page  | Notes                                                                         |
| -------------------------------------- | ----- | ----------------------------------------------------------------------------- |
| Condition bits: AE+74, 2E+74, A7+59    | 5–6   | Run as ADD B; flags the manual names are in each title                        |
| CMA 51 → AE                            | 15    |                                                                               |
| DCR M 40 → 3F                          | 15    | The manual gives no flags; see assumption 6                                   |
| INR C 99 → 9A                          | 15    | The manual gives no flags; see assumption 6                                   |
| DAA 9B → 01, AC and CY set             | 16    |                                                                               |
| ADD 6C + 2E = 9A                       | 18    | Flags as stated (S, AC, P set; Z, CY clear)                                   |
| ADC 42 + 3D, carry clear and carry set | 18    | Both cases and their flags are in the manual                                  |
| SUB A, 3E − 3E = 0                     | 18    |                                                                               |
| SBB 04 − 02 − 1 = 01                   | 19    | The manual uses register L, as the test does                                  |
| ANA FC AND 0F = 0C                     | 19    | The manual gives only the result; AC per assumption 1                         |
| ORA 33 OR 0F = 3F                      | 20    | The manual gives only the result                                              |
| CMP 0A, 02, E5 vs 05                   | 20–21 | The manual frames E5 as a signed compare (−1B vs 5)                           |
| Status change: 78 XOR 5C, then AND 5C  | 20    | XRA B then ANA B; results only; AC on ANA per assumption 1                    |
| RLC / RRC of F2                        | 21    |                                                                               |
| RAL B5 → 6A, RAR 6A → B5               | 22    |                                                                               |
| PUSH PSW: A=1F, CY Z P set → 1F, 47    | 23    | Currently fails (PUSH not implemented)                                        |
| POP PSW: C3 FF → A=FF, S Z CY set      | 23    | Currently fails (POP not implemented)                                         |
| DAD A17B + 339F = D51A                 | 24    |                                                                               |
| INX D 38FF → 3900, DCX H 9800 → 97FF   | 24    |                                                                               |
| INX SP FFFF → 0000                     | 24    |                                                                               |
| SUI 00 − 01 = FF                       | 27–28 | AC clear on a borrow out of bit 3; see assumption 4                           |
| ADI 14 + 42, then 56 + BE              | 27    | Both steps' flags are in the manual                                           |
| ACI 56 + BE + 0, then 14 + 42 + 1      | 27    | Step 1: A and CY given, other flags via ADI example; step 2: result only      |
| SBI 00 − 01, carry clear and carry set | 28    | AC clear on a borrow out of bit 3; see assumption 4                           |
| ANI 3A AND 0F = 0A                     | 28    | The manual gives only the result; AC per assumption 1; A set directly         |
| CPI 4A vs 40                           | 29    | The manual names only Z and CY                                                |
| XRI 3B XOR 81 = BA                     | 29    | The manual gives only the result                                              |
| ORI B5 OR 0F = BF                      | 29    | The manual gives only the result; A set directly                              |
| Multibyte add 32AF8A + 84BA90 = B76A1A | 55    | The manual's MADD loop, with the byte count moved from C to E (see the test)  |
| Multibyte subtract 1301 − 0503 = 0DFE  | 56    | MADD with SBB M in place of ADC M, as the manual describes                    |
| Decimal add 2985 + 4936 = 7921         | 56    | A and flags checked after each ADI/ACI and DAA; the manual gives A, CY and AC |
| Decimal subtract 4358 − 1362 = 2996    | 57    | The manual's steps, run with immediates; A and flags checked after each       |

Examples that only move data (MOV, LDAX, STAX, LXI, MVI, PUSH D, POP H, XCHG,
SPHL, XTHL; pp. 17 and 23–27) aren't separate tests: the per-opcode tests
already check the same moves. Examples without concrete values (ADD A doubling
A, XRA A clearing it, DAD H doubling HL) are covered by those opcodes' own
tests. The multiplication example (2A × 3C = 09D8, pp. 53–54) isn't tested.

### Shared rules

| Rule                                        | Source  |
| ------------------------------------------- | ------- |
| Standard flag rules for Z, S, P, CY         | p. 4-2  |
| AC = carry out of bit 3                     | p. 4-3  |
| Immediates and addresses: low byte first    | p. 4-2  |
| Register and register-pair bit encodings    | p. 4-3  |
| Opcode encodings and cycle counts (summary) | p. 4-15 |

### Data transfer (`data_transfer.cpp`)

| Test / opcode                              | Source                        | Cycles    |
| ------------------------------------------ | ----------------------------- | --------- |
| No flags change (whole group)              | p. 4-4, group intro           | —         |
| MOV r1,r2 / MOV r,M / MOV M,r              | p. 4-4                        | 5 / 7 / 7 |
| MVI r / MVI M                              | p. 4-4                        | 7 / 10    |
| LXI (byte 3 → high register, byte 2 → low) | p. 4-4                        | 10        |
| LDA, STA                                   | p. 4-5                        | 13        |
| LHLD, SHLD (L ↔ addr, H ↔ addr+1)          | p. 4-5                        | 16        |
| LDAX / STAX (B and D only)                 | p. 4-5                        | 7         |
| XCHG                                       | p. 4-5                        | 4         |
| LHLD / SHLD at FFFF wrapping to 0000       | Not stated; see assumption 5. | —         |

### Arithmetic (`arithmetic.cpp`)

| Test / opcode                                          | Source                                                                                                                                                                                                           | Cycles              |
| ------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------- |
| ADD / ADC / ADI / ACI                                  | p. 4-6                                                                                                                                                                                                           | 4, M 7, immediate 7 |
| SUB / SBB / SUI / SBI                                  | pp. 4-6, 4-7. "Two's complement; CY = borrow": p. 4-6 group intro                                                                                                                                                | 4, M 7, immediate 7 |
| AC after subtraction (set when no borrow out of bit 3) | Derived: A + NOT operand + 1 (p. 4-6) plus p. 4-3 (AC rule). Shown in ALP's SUB A, SBB, SUI and SBI examples (pp. 18, 19, 27–28) and explained in the 8080/8085 ALP's SUB A example (p. 3-64). See assumption 4. | —                   |
| INR / DCR (all flags except CY)                        | p. 4-7; DCR M on p. 4-8                                                                                                                                                                                          | 5, M 10             |
| INX / DCX (no flags)                                   | p. 4-8. INX SP FFFF→0000: ALP p. 24. Other wraparound: assumption 5.                                                                                                                                             | 5                   |
| DAD (only CY)                                          | p. 4-8                                                                                                                                                                                                           | 10                  |
| DAA (two-step rule)                                    | p. 4-8. CY kept when step 2 doesn't carry: ALP p. 16. See assumption 3.                                                                                                                                          | 4                   |
| ALP worked examples                                    | See [Worked examples](#worked-examples-alp).                                                                                                                                                                     | —                   |

### Logical (`logical.cpp`)

| Test / opcode                                          | Source                                                    | Cycles              |
| ------------------------------------------------------ | --------------------------------------------------------- | ------------------- |
| ANA r / ANA M (CY cleared, AC "affected")              | p. 4-8. AC rule: 8080/8085 ALP p. 1-12; see assumption 1. | 4 / 7               |
| ANI                                                    | p. 4-9. The tests contradict it on AC; see assumption 1.  | 7                   |
| XRA / XRI / ORA / ORI (CY and AC cleared)              | p. 4-9. See assumption 2.                                 | 4, M 7, immediate 7 |
| CMP / CPI (Z if equal, CY if A < operand, A unchanged) | pp. 4-9, 4-10. AC is derived the same way as for SUB.     | 4, M 7, immediate 7 |
| RLC / RRC / RAL / RAR (only CY)                        | p. 4-10                                                   | 4                   |
| CMA (no flags)                                         | p. 4-10                                                   | 4                   |
| CMC, STC                                               | p. 4-11                                                   | 4                   |
| ALP worked examples                                    | See [Worked examples](#worked-examples-alp).              | —                   |

### Branch (`branch.cpp`)

| Test / opcode                                          | Source               | Cycles                  |
| ------------------------------------------------------ | -------------------- | ----------------------- |
| No flags change; condition codes (NZ/Z/NC/C/PO/PE/P/M) | p. 4-11, group intro | —                       |
| JMP, Jcc                                               | p. 4-11              | 10, taken or not        |
| CALL (high byte at SP−1, low byte at SP−2)             | p. 4-11              | 17                      |
| Ccc                                                    | p. 4-12; ALP p. xvi  | 17 taken / 11 not taken |
| RET                                                    | p. 4-12              | 10                      |
| Rcc                                                    | p. 4-12; ALP p. xvi  | 11 taken / 5 not taken  |
| RST n → 8 × NNN                                        | p. 4-12; also p. 4-2 | 11                      |
| PCHL                                                   | p. 4-12              | 5                       |
| CALL / RET stack wraparound                            | See assumption 5.    | —                       |

### Stack, I/O and control (`stack_io_control.cpp`)

| Test                                                       | Source                                                               | Cycles   |
| ---------------------------------------------------------- | -------------------------------------------------------------------- | -------- |
| PUSH / POP byte order                                      | p. 4-13                                                              | 11 / 10  |
| PUSH PSW flag byte (bit 1 = 1, bits 3 and 5 = 0), POP PSW  | p. 4-13, which gives the exact bit layout. Examples: ALP p. 23       | 11 / 10  |
| XTHL, SPHL                                                 | p. 4-14                                                              | 18 / 5   |
| IN, OUT                                                    | p. 4-14                                                              | 10       |
| NOP, HLT (registers and flags unaffected)                  | p. 4-14                                                              | 4 / 7    |
| EI takes effect after the next instruction; DI immediately | p. 4-14                                                              | 4        |
| Interrupt: PC not advanced, device supplies RST, PC pushed | p. 2-11, "Interrupt Sequences"; see assumption 7                     | 11 (RST) |
| Accepting an interrupt disables further interrupts         | p. 5-14, INTE pin description                                        | —        |
| An interrupt wakes HLT only if INTE was set                | p. 2-13, "Halt Sequences"                                            | —        |
| Interrupts disabled at power-on                            | p. 5-14 (RESET clears INTE) and p. 2-13 (start-up begins with RESET) | —        |
| Return address after HLT points past the HLT               | ALP p. 39: "The program counter is incremented..."                   | —        |
| PUSH / POP stack wraparound                                | See assumption 5.                                                    | —        |

### Undocumented (`undocumented.cpp`)

Not in any Intel manual. The summary table on p. 4-15 leaves these 12 opcodes
out, so every expected value comes from outside sources (see assumption 8 under
[Assumptions](#assumptions)).

### Programs (`programs.cpp`)

These combine the per-opcode rules above. Four of them (count-down loop, sum 1
to 10, subroutine called twice, register swap) also check the total cycle count,
which is the sum of the manual's per-instruction counts. The other three (16-bit
add, block copy, interrupt handler) check only end results.

Four more are the ALP's programming-chapter examples (pp. 55–57). Multibyte
addition and subtraction run the manual's MADD loop and check the result bytes
and the final carry. Decimal addition and subtraction check A and every flag
after each arithmetic step and DAA, against the values the manual gives.
