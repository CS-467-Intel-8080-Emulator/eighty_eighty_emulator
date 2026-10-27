#include "cpu.hpp"
#include "memory.hpp"
#include "io_bus.hpp"
#include "iostream"
#include "iomanip"


// Unimplmented catcher
uint32_t CPU::op_unimplemented(Memory&) {
    std::cerr << "Unimplemented opcode: 0x"
              << std::hex << std::uppercase << (int)current_opcode
              << " at PC=0x" << pc << std::dec << std::endl;

    halted = true;
    return 0;
}

/*
Constructor for the CPU class
 */
CPU::CPU()
    : a(0), b(0), c(0), d(0), e(0), h(0), l(0),
      pc(0), sp(0), flags(0),
      interrupts_enabled(false), ei_pending(false), halted(false),
      io(nullptr)

    {
        //
        table.fill(&CPU::op_unimplemented);

        
        // --- Explicit opcodes ---
        table[0x00] = &CPU::op_nop;
        table[0x08] = &CPU::op_nop;
        table[0x10] = &CPU::op_nop;
        table[0x18] = &CPU::op_nop;
        table[0x20] = &CPU::op_nop;
        table[0x28] = &CPU::op_nop;
        table[0x30] = &CPU::op_nop;
        table[0x38] = &CPU::op_nop;
        table[0x3A] = &CPU::op_lda;
        table[0x32] = &CPU::op_sta;
        table[0x2A] = &CPU::op_lhld;
        table[0x22] = &CPU::op_shld;
        table[0xEB] = &CPU::op_xchg;

        // --- MVI group (0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x36, 0x3E) ---
        for (uint8_t opcode = 0x06; opcode <= 0x3E; opcode += 0x08) 
            table[opcode] = &CPU::op_mvi;
            
        // --- MOV group (0x40 - 0x7F) ---
        for (uint8_t opcode = 0x40; opcode <= 0x7F; opcode++) 
            table[opcode] = &CPU::op_mov;
        table[0x76] = &CPU::op_hlt;     // must follow MOV loop

        // --- LXI group (0x01, 0x11, 0x21, 0x31) ---
        for (uint8_t opcode = 0; opcode <= 3; opcode++)
            table[(opcode << 4) | 0x1] = &CPU::op_lxi;

        // --- INX group (0x03, 0x13, 0x23, 0x33) ---
        for (uint8_t opcode = 0; opcode <= 3; opcode++)
            table[(opcode << 4) | 0x3] = &CPU::op_inx;
        
        // --- DCX group (0x0B, 0x1B, 0x2B, 0x3B) ---
         for (uint8_t opcode = 0; opcode <= 3; opcode++)
            table[(opcode << 4) | 0xB] = &CPU::op_dcx;

        // --- LDAX group (0x0A, 0x1A) ---
        for (uint8_t opcode = 0; opcode <= 1; opcode++)
            table[(opcode << 4) | 0xA] = &CPU::op_ldax;

        // --- STAX group (0x02, 0x12) ---
        for (uint8_t opcode = 0; opcode <= 1; opcode++)
            table[(opcode << 4) | 0x2] = &CPU::op_stax;

        // --- PUSH group (C5, D5, E5, F5)---
        for (uint8_t opcode = 0xC; opcode <= 0xF; opcode++){
            table[(opcode << 4) | 0x5] = &CPU::op_push;
        }

        // -- POP group (0xC1, 0xD1, 0xE1, 0xF1) ---
        for (uint8_t opcode = 0xC; opcode <= 0xF; opcode++) {
            table[(opcode << 4) | 0x1] = &CPU::op_pop;
        }

        // --- Arithmetic group (0x80 - 0x9F) ---
        for (uint8_t opcode = 0x80; opcode <= 0x87; opcode++) 
            table[opcode] = &CPU::op_add;
        for (uint8_t opcode = 0x88; opcode <= 0x8F; opcode++)
            table[opcode] = &CPU::op_adc;
        for (uint8_t opcode = 0x90; opcode <= 0x97; opcode++)
            table[opcode] = &CPU::op_sub;
        for (uint8_t opcode = 0x98; opcode <= 0x9F; opcode++)
            table[opcode] = &CPU::op_sbb;

        // --- Logical group (0xA0 - 0xBF) ---
        for (uint8_t opcode = 0xA0; opcode <= 0xA7; opcode++)
            table[opcode] = &CPU::op_ana;
        for (uint8_t opcode = 0xA8; opcode <= 0xAF; opcode++)
            table[opcode] = &CPU::op_xra;
        for (uint8_t opcode = 0xB0; opcode <= 0xB7; opcode++)
            table[opcode] = &CPU::op_ora;
        for (uint8_t opcode = 0xB8; opcode <= 0xBF; opcode++)
            table[opcode] = &CPU::op_cmp;

        // --- INR (0x04 - 0x3C) / DCR (0x05 - 0x3D) groups ---
        for (uint8_t opcode = 0x04; opcode <= 0x3C; opcode += 0x08)
            table[opcode] = &CPU::op_inr;
        for (uint8_t opcode = 0x05; opcode <= 0x3D; opcode += 0x08)
            table[opcode] = &CPU::op_dcr;

        // --- DAD group (0x09, 0x19, 0x29, 0x39) ---
        for (uint8_t opcode = 0x09; opcode <= 0x39; opcode += 0x10)
            table[opcode] = &CPU::op_dad;

        // --- Rotates, DAA, CMA, STC, CMC ---
        table[0x07] = &CPU::op_rlc;
        table[0x0F] = &CPU::op_rrc;
        table[0x17] = &CPU::op_ral;
        table[0x1F] = &CPU::op_rar;
        table[0x27] = &CPU::op_daa;
        table[0x2F] = &CPU::op_cma;
        table[0x37] = &CPU::op_stc;
        table[0x3F] = &CPU::op_cmc;

        // --- Control flow: conditional (one opcode per condition code) ---
        for (uint16_t op = 0xC2; op <= 0xFA; op += 0x08) table[op] = &CPU::op_jcc;

        // --- Control flow: unconditional ---
        table[0xC3] = &CPU::op_jmp;
        table[0xCB] = &CPU::op_jmp;     // undocumented alias for JMP

        // --- Calls and returns ---
        for (uint16_t op = 0xC4; op <= 0xFC; op += 0x08) table[op] = &CPU::op_ccc;
        for (uint16_t op = 0xC0; op <= 0xF8; op += 0x08) table[op] = &CPU::op_rcc;
        for (uint16_t op = 0xC7; op <= 0xFF; op += 0x08) table[op] = &CPU::op_rst;
        table[0xCD] = &CPU::op_call;
        table[0xDD] = &CPU::op_call;
        table[0xED] = &CPU::op_call;
        table[0xFD] = &CPU::op_call;

        table[0xD9] = &CPU::op_ret;
        table[0xC9] = &CPU::op_ret;
        table[0xE9] = &CPU::op_pchl;

        // --- Machine control (must follow the branching loop) ---
        table[0xF3] = &CPU::op_di;
        table[0xFB] = &CPU::op_ei;

        // --- I/O (must follow the branching loop) ---
        table[0xDB] = &CPU::op_in;
        table[0xD3] = &CPU::op_out;

        // --- Immediates (must follow the branching loop) ---
        table[0xC6] = &CPU::op_adi;
        table[0xCE] = &CPU::op_aci;
        table[0xD6] = &CPU::op_sui;
        table[0xDE] = &CPU::op_sbi;
        table[0xE6] = &CPU::op_ani;
        table[0xEE] = &CPU::op_xri;
        table[0xF6] = &CPU::op_ori;
        table[0xFE] = &CPU::op_cpi;
        table[0xE3] = &CPU::op_xthl;
        table[0xf9] = &CPU::op_sphl;
    }

 uint32_t CPU::step(Memory& memory) {
    if (ei_pending) {
        interrupts_enabled = true;
        ei_pending = false;
    }

    if (halted) {
        return 4;
    }

    uint8_t opcode = memory.read(pc);
    pc = (pc +1) & 0xFFFF;
    current_opcode = opcode;
    return (this->*table[current_opcode])(memory);
}

/*
Helper functions for opcode handlers can be implemented here
*/

// Fetch a byte
uint8_t CPU::fetch_byte(Memory& memory) {
    return memory.read(pc++);
}

// Fetch a word (2 bytes)
uint16_t CPU::fetch_word(Memory& memory) {
    uint8_t lo = memory.read(pc);
    pc = (pc + 1) & 0xFFFF;

    uint8_t hi = memory.read(pc);
    pc = (pc + 1) & 0xFFFF;

    return (hi << 8) | lo;
}

// Push a word (SP wraps at 0x0000)
void CPU::push_word(Memory& memory, uint16_t value)
{
    uint8_t hi = value >> 8;
    uint8_t lo = value & 0xFF;

    sp = (sp - 1) & 0xFFFF;
    memory.write(sp, hi);

    sp = (sp - 1) & 0xFFFF;
    memory.write(sp, lo);
}

// Pop a word (SP wraps at 0xFFFF)
uint16_t CPU::pop_word(Memory& memory)
{
    uint8_t lo = memory.read(sp);
    sp = (sp + 1) & 0xFFFF;

    uint8_t hi = memory.read(sp);
    sp = (sp + 1) & 0xFFFF;

    return (hi << 8) | lo;
}

// Evaluate 3-bit condition code (cc) used by Jcc, Ccc, Rcc
bool CPU::check_condition(uint8_t opcode) const {
    uint8_t cc = (opcode >> 3) & 0x07;
    switch(cc) {
        case 0: return !(flags & FLAG_ZERO);    // NZ:  Not Zero
        case 1: return (flags & FLAG_ZERO);     // Z:   Zero
        case 2: return !(flags & FLAG_CARRY);   // NC:  Not Carry
        case 3: return (flags & FLAG_CARRY);    // C:   Carry
        case 4: return !(flags & FLAG_PARITY);  // PO:  Parity Odd
        case 5: return (flags & FLAG_PARITY);   // PE:  Parity Even
        case 6: return !(flags & FLAG_SIGN);    // P:   Plus
        case 7: return (flags & FLAG_SIGN);     // M:   Minus
    }
    return false;
}

// NOP instruction handler
uint32_t CPU::op_nop(Memory& memory) {
    return 4;
}

// LXI instruction handler
uint32_t CPU::op_lxi(Memory& memory) {
    uint16_t value = fetch_word(memory);
    // Determine which register pair to load based on the opcode
    switch (current_opcode) {
        case 0x01: // LXI B
            c = value & 0xFF;
            b = value >> 8;
            break;
        case 0x11: // LXI D
            e = value & 0xFF;
            d = value >> 8;
            break;
        case 0x21: // LXI H
            l = value & 0xFF;
            h = value >> 8;
            break;
        case 0x31: // LXI SP
            sp = value;
            break;
    }
    return 10;
}

// MOV instruction handler
uint32_t CPU::op_mov(Memory& memory) {
    uint8_t src = current_opcode & 0x07; 
    uint8_t dst = (current_opcode >> 3) & 0x07;

    uint8_t value = read_reg_or_memory(src, memory);
    write_reg_or_memory(dst, value, memory);

    return (src == 6 || dst == 6) ? 7 : 5;
}

// MVI instruction handler
uint32_t CPU::op_mvi(Memory& memory) {
    uint8_t dst = (current_opcode >> 3) & 0x07;
    uint8_t value = fetch_byte(memory);
    write_reg_or_memory(dst, value, memory);

    // MVI r takes 7 cycles, MVI M takes 10 cycles
    return (dst == 6) ? 10 : 7;
}

// LDA instruction handler
uint32_t CPU::op_lda(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    a = memory.read(addr);
    return 13;
}

// STA instruction handler
uint32_t CPU::op_sta(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    memory.write(addr, a);
    return 13;
}

// LHLD instruction handler
uint32_t CPU::op_lhld(Memory& memory){
    uint16_t addr = fetch_word(memory);
    // Load L then H from memory
    l = memory.read(addr);
    h = memory.read(addr + 1) & 0xFFFF;
    return 16;
}

// SHLD instruction handler
uint32_t CPU::op_shld(Memory& memory){
    uint16_t addr = fetch_word(memory);
    // Store L then H to memory
    memory.write(addr, l);
    memory.write((addr + 1) & 0xFFFF, h);
    return 16;
}

// LDAX instruction handler
uint32_t CPU::op_ldax(Memory& memory) {
    uint16_t adder;
    if (current_opcode == 0x0A) {
        adder = (b <<8) | c;
    } else {
        adder = (d << 8) | e;
    }
    a = memory.read(adder);
    return 7;
}

// STAX B & D instruction handler
uint32_t CPU::op_stax(Memory& memory) {
    uint16_t adder;
    // 0x02 is STAX B, 0x12 is STAX D
    if (current_opcode == 0x02) {
        adder = (b << 8) | c;
    } else {
        adder = (d << 8) | e;
    }
    memory.write(adder, a);
    return 7;
}

// XCHG instruction handler
uint32_t CPU::op_xchg(Memory& memory) {
    std::swap(d, h);
    std::swap(e, l);
    return 4;
}

// INX instruction handler
uint32_t CPU::op_inx(Memory& memory) {
    // Determine which register pair to increment based on the opcode
    switch (current_opcode) {
        case 0x03: // INX B
        {
            uint16_t bc = (b << 8) | c;
            bc++;
            b = (bc >> 8) & 0xFF;
            c = bc & 0xFF;
        }
        break;
        case 0x13: // INX D
        {
            uint16_t de = (d << 8) | e;
            de++;
            d = (de >> 8) & 0xFF;
            e = de & 0xFF;
        }
        break;
        case 0x23: // INX H
        {
            uint16_t hl = (h << 8) | l;
            hl++;
            h = (hl >> 8) & 0xFF;
            l = hl & 0xFF;
        }
        break;
        case 0x33: // INX SP
        {
            sp++;
        }
        break;
    }
    return 5;
}

// DCX instruction handler
uint32_t CPU::op_dcx(Memory& memory) {
    switch (current_opcode) {
        case 0x0B: // DCX B
            {
                uint16_t bc = (b << 8) | c;
                bc--;
                b = (bc >> 8) & 0xFF;
                c = bc & 0xFF;
            }
            break;
        case 0x1B: // DCX D
            {
                uint16_t de = (d << 8) | e;
                de--;
                d = (de >> 8) & 0xFF;
                e = de & 0xFF;
            }
            break;
        case 0x2B: // DCX H
            {
                uint16_t hl = (h << 8) | l;
                hl--;
                h = (hl >> 8) & 0xFF;
                l = hl & 0xFF;
            }
            break;
        case 0x3B: // DCX SP
            sp--;
            break;
    }
    return 5;
}

// PUSH instruction handler (Push B, D, H, PSW)
uint32_t CPU::op_push(Memory& memory){
    uint16_t value = 0;

    switch (current_opcode) {
        case 0xC5: // Push B
            value = (b << 8) | c;
            break;

        case 0xD5: // Push D
            value = (d << 8) | e;
            break;

        case 0xE5: // PUSH H
            value = (h << 8) | l;
            break;

        case 0xF5: // PUSH PSW
            value = (a << 8) | (flags | 0x02);
            break;
    }

    push_word(memory, value);
    return 11;

}

// POP instruction handler
uint32_t CPU::op_pop(Memory& memory){
    uint16_t value = pop_word(memory);

    switch (current_opcode) {
        case 0xC1: // POP B
            b = (value >> 8) & 0xFF;
            c = value & 0xFF;
            break;

        case 0xD1: // POP D
            d = (value >> 8) & 0xFF;
            e = value & 0xFF;
            break;

        case 0xE1: // POP H
            h = (value >> 8) & 0xFF;
            l = value & 0xFF;
            break;

        case 0xF1: // POP PSW
            a = (value >> 8) & 0xFF;
            flags = (value & 0xFF) | 0x02;
            break;
    }

    return 10;
}

// XTHL instruction handler
uint32_t CPU::op_xthl(Memory& memory){
    uint8_t temp_l = l;
    uint8_t temp_h = h;

    l = memory.read(sp);
    h = memory.read(sp + 1) & 0xFFFF;

    memory.write(sp, temp_l);
    memory.write(sp +1, temp_h);

    return 18;
}

// SPHL instruction handler
uint32_t CPU::op_sphl (Memory& memory) {
    sp = (h << 8) | l;
    return 5;
}

// JMP addr instruction handler (0xC3)
uint32_t CPU::op_jmp(Memory& memory) {
    pc = fetch_word(memory);
    return 10;
}

// Jcc addr instruction handler (0xC2 + 8*cc: JNZ JZ JNC JC JPO JPE JP JM)
uint32_t CPU::op_jcc(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    if (check_condition(current_opcode)) {
        pc = addr;
    }
    return 10;
}

// CALL addr instruction handler (0xCD)
uint32_t CPU::op_call(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    push_word(memory, pc);
    pc = addr;
    return 17;
}

// Ccc addr instruction handler (0xC4 + 8*cc: CNZ CZ CNC CC CPO CPE CP CM)
uint32_t CPU::op_ccc(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    if (check_condition(current_opcode)) {
        push_word(memory, pc);
        pc = addr;
        return 17;
    }
    return 11;
}

// RET instruction handler (0xC9)
uint32_t CPU::op_ret(Memory& memory) {
    pc = pop_word(memory);
    return 10;
}

// Rcc instruction handler (0xC0 + 8*cc: RNZ RZ RNC RC RPO RPE RP RM)
uint32_t CPU::op_rcc(Memory& memory) {
    if (check_condition(current_opcode)) {
        pc = pop_word(memory);
        return 11;
    }
    return 5;
}

// RST n instruction handler (0xC7 + 8*n): call to n*8
uint32_t CPU::op_rst(Memory& memory) {
    push_word(memory, pc);
    pc = current_opcode & 0x38;
    return 11;
}

// PCHL instruction handler (0xE9): PC = HL
uint32_t CPU::op_pchl(Memory&) {
    pc = (h << 8) | l;
    return 5;
}

// HLT instruction handler (0x76)
uint32_t CPU::op_hlt(Memory&) {
    halted = true;
    return 7;
}

// EI instruction handler (0xFB)
uint32_t CPU::op_ei(Memory&) {
    ei_pending = true;
    return 4;
}

// DI instruction handler (0xF3)
uint32_t CPU::op_di(Memory&) {
    interrupts_enabled = false;
    ei_pending = false;
    return 4;
}

// Attach the machine's port handlers
void CPU::set_io(IOBus* bus) {
    io = bus;
}

// IN port instruction handler (0xDB)
uint32_t CPU::op_in(Memory& memory) {
    uint8_t port = fetch_byte(memory);
    a = io ? io->in(port) : 0xFF;
    return 10;
}

// OUT port instruction handler (0xD3)
uint32_t CPU::op_out(Memory& memory) {
    uint8_t port = fetch_byte(memory);
    if (io) {
        io->out(port, a);
    }
    return 10;
}

// Hardware interrupt: ignored unless enabled, then runs RST rst_num
uint32_t CPU::interrupt(Memory& memory, uint8_t rst_num) {
    if (!interrupts_enabled) {
        return 0;
    }
    interrupts_enabled = false;
    halted = false;

    // The interrupting device supplies an RST n instruction (0xC7 + 8*n)
    current_opcode = 0xC7 | (rst_num << 3);
    return op_rst(memory);
}

/*
Register and memory helper functions
*/
uint8_t CPU::read_reg_or_memory(uint8_t index, Memory& memory) {
    switch (index) {
        case 0: return b;
        case 1: return c;
        case 2: return d;
        case 3: return e;
        case 4: return h;
        case 5: return l;
        case 6: return memory.read((h << 8) | l);
        case 7: return a;
        }
    return 0;
}

void CPU::write_reg_or_memory(uint8_t index, uint8_t value, Memory& memory) {
    switch (index) {
        case 0: b = value; break;
        case 1: c = value; break;
        case 2: d = value; break;
        case 3: e = value; break;
        case 4: h = value; break;
        case 5: l = value; break;
        case 6: memory.write((h << 8) | l, value); break;
        case 7: a = value; break;
    }
}


/*
Flag helper functions

Flag byte layout (bit 7 -> 0): S Z 0 AC 0 P 1 CY
*/

void CPU::set_flag(uint8_t mask, bool value) {
    if (value)
        flags |= mask;
    else
        flags &= ~mask;
}

bool CPU::get_flag(uint8_t mask) const {
    return (flags & mask) != 0;
}

// True if an even number of bits are set
bool CPU::parity_even(uint8_t value) {
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 0x01) == 0;
}

void CPU::update_flags_zsp(uint8_t result) {
    set_flag(FLAG_ZERO, result == 0);
    set_flag(FLAG_SIGN, (result & 0x80) != 0);
    set_flag(FLAG_PARITY, parity_even(result));
}

// result is the 16-bit sum including carry in. AC is the carry into bit 4.
void CPU::update_flags_add(uint8_t lhs, uint8_t value, uint16_t result) {
    update_flags_zsp(result & 0xFF);
    set_flag(FLAG_CARRY, (result & 0x100) != 0);
    set_flag(FLAG_AUX_CARRY, ((lhs ^ value ^ result) & 0x10) != 0);
}

// result is the 16-bit difference including borrow in. AC is the inverse of the borrow into bit 4.
void CPU::update_flags_sub(uint8_t lhs, uint8_t value, uint16_t result) {
    update_flags_zsp(result & 0xFF);
    set_flag(FLAG_CARRY, (result & 0x100) != 0);
    set_flag(FLAG_AUX_CARRY, ((lhs ^ value ^ result) & 0x10) == 0);
}

// Clears CY and AC
void CPU::update_flags_logic(uint8_t result) {
    update_flags_zsp(result);
    set_flag(FLAG_CARRY, false);
    set_flag(FLAG_AUX_CARRY, false);
}


/*
ALU helper functions
*/

// A = A + value + carry_in
void CPU::alu_add(uint8_t value, uint8_t carry_in) {
    uint16_t result = (uint16_t)a + value + carry_in;
    update_flags_add(a, value, result);
    a = result & 0xFF;
}

// A = A - value - borrow_in
void CPU::alu_sub(uint8_t value, uint8_t borrow_in, bool store_result) {
    uint16_t result = (uint16_t)a - value - borrow_in;
    update_flags_sub(a, value, result);
    if (store_result)
        a = result & 0xFF;
}

// A = A & value. AC is bit 3 of (A | value).
void CPU::alu_and(uint8_t value) {
    uint8_t result = a & value;
    update_flags_logic(result);
    set_flag(FLAG_AUX_CARRY, ((a | value) & 0x08) != 0);
    a = result;
}

// A = A ^ value
void CPU::alu_xor(uint8_t value) {
    a ^= value;
    update_flags_logic(a);
}

// A = A | value
void CPU::alu_or(uint8_t value) {
    a |= value;
    update_flags_logic(a);
}


/*
Arithmetic and logic instruction handlers
*/

// ADD r instruction handler
uint32_t CPU::op_add(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    alu_add(read_reg_or_memory(src, memory), 0);
    return (src == 6) ? 7 : 4;
}

// ADC r instruction handler
uint32_t CPU::op_adc(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    alu_add(read_reg_or_memory(src, memory), get_flag(FLAG_CARRY) ? 1 : 0);
    return (src == 6) ? 7 : 4;
}

// SUB r instruction handler
uint32_t CPU::op_sub(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    alu_sub(read_reg_or_memory(src, memory), 0, true);
    return (src == 6) ? 7 : 4;
}

// SBB r instruction handler
uint32_t CPU::op_sbb(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    alu_sub(read_reg_or_memory(src, memory), get_flag(FLAG_CARRY) ? 1 : 0, true);
    return (src == 6) ? 7 : 4;
}

// ANA r instruction handler
uint32_t CPU::op_ana(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    alu_and(read_reg_or_memory(src, memory));
    return (src == 6) ? 7 : 4;
}

// XRA r instruction handler
uint32_t CPU::op_xra(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    alu_xor(read_reg_or_memory(src, memory));
    return (src == 6) ? 7 : 4;
}

// ORA r instruction handler
uint32_t CPU::op_ora(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    alu_or(read_reg_or_memory(src, memory));
    return (src == 6) ? 7 : 4;
}

// CMP r instruction handler
uint32_t CPU::op_cmp(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    alu_sub(read_reg_or_memory(src, memory), 0, false);
    return (src == 6) ? 7 : 4;
}

// INR r instruction handler (CY unaffected)
uint32_t CPU::op_inr(Memory& memory) {
    uint8_t dst = (current_opcode >> 3) & 0x07;
    uint8_t value = read_reg_or_memory(dst, memory);
    uint8_t result = value + 1;

    update_flags_zsp(result);
    set_flag(FLAG_AUX_CARRY, (result & 0x0F) == 0x00);

    write_reg_or_memory(dst, result, memory);
    return (dst == 6) ? 10 : 5;
}

// DCR r instruction handler (CY unaffected)
uint32_t CPU::op_dcr(Memory& memory) {
    uint8_t dst = (current_opcode >> 3) & 0x07;
    uint8_t value = read_reg_or_memory(dst, memory);
    uint8_t result = value - 1;

    update_flags_zsp(result);
    set_flag(FLAG_AUX_CARRY, (result & 0x0F) != 0x0F);

    write_reg_or_memory(dst, result, memory);
    return (dst == 6) ? 10 : 5;
}

// DAD rp instruction handler (CY only)
uint32_t CPU::op_dad(Memory&) {
    uint16_t hl = (h << 8) | l;
    uint16_t value = 0;

    switch ((current_opcode >> 4) & 0x03) {
        case 0: value = (b << 8) | c; break;
        case 1: value = (d << 8) | e; break;
        case 2: value = hl; break;
        case 3: value = sp; break;
    }

    uint32_t result = (uint32_t)hl + value;
    set_flag(FLAG_CARRY, (result & 0x10000) != 0);

    h = (result >> 8) & 0xFF;
    l = result & 0xFF;
    return 10;
}

// DAA instruction handler
uint32_t CPU::op_daa(Memory&) {
    uint8_t correction = 0;
    bool carry = get_flag(FLAG_CARRY);
    uint8_t low = a & 0x0F;
    uint8_t high = a >> 4;

    if (get_flag(FLAG_AUX_CARRY) || low > 9)
        correction |= 0x06;

    if (carry || high > 9 || (high >= 9 && low > 9)) {
        correction |= 0x60;
        carry = true;
    }

    alu_add(correction, 0);
    set_flag(FLAG_CARRY, carry);
    return 4;
}

// ADI instruction handler
uint32_t CPU::op_adi(Memory& memory) {
    alu_add(fetch_byte(memory), 0);
    return 7;
}

// ACI instruction handler
uint32_t CPU::op_aci(Memory& memory) {
    alu_add(fetch_byte(memory), get_flag(FLAG_CARRY) ? 1 : 0);
    return 7;
}

// SUI instruction handler
uint32_t CPU::op_sui(Memory& memory) {
    alu_sub(fetch_byte(memory), 0, true);
    return 7;
}

// SBI instruction handler
uint32_t CPU::op_sbi(Memory& memory) {
    alu_sub(fetch_byte(memory), get_flag(FLAG_CARRY) ? 1 : 0, true);
    return 7;
}

// ANI instruction handler
uint32_t CPU::op_ani(Memory& memory) {
    alu_and(fetch_byte(memory));
    return 7;
}

// XRI instruction handler
uint32_t CPU::op_xri(Memory& memory) {
    alu_xor(fetch_byte(memory));
    return 7;
}

// ORI instruction handler
uint32_t CPU::op_ori(Memory& memory) {
    alu_or(fetch_byte(memory));
    return 7;
}

// CPI instruction handler
uint32_t CPU::op_cpi(Memory& memory) {
    alu_sub(fetch_byte(memory), 0, false);
    return 7;
}

// RLC instruction handler
uint32_t CPU::op_rlc(Memory&) {
    bool bit7 = (a & 0x80) != 0;
    a = (a << 1) | (bit7 ? 1 : 0);
    set_flag(FLAG_CARRY, bit7);
    return 4;
}

// RRC instruction handler
uint32_t CPU::op_rrc(Memory&) {
    bool bit0 = (a & 0x01) != 0;
    a = (a >> 1) | (bit0 ? 0x80 : 0);
    set_flag(FLAG_CARRY, bit0);
    return 4;
}

// RAL instruction handler
uint32_t CPU::op_ral(Memory&) {
    bool bit7 = (a & 0x80) != 0;
    a = (a << 1) | (get_flag(FLAG_CARRY) ? 1 : 0);
    set_flag(FLAG_CARRY, bit7);
    return 4;
}

// RAR instruction handler
uint32_t CPU::op_rar(Memory&) {
    bool bit0 = (a & 0x01) != 0;
    a = (a >> 1) | (get_flag(FLAG_CARRY) ? 0x80 : 0);
    set_flag(FLAG_CARRY, bit0);
    return 4;
}

// CMA instruction handler
uint32_t CPU::op_cma(Memory&) {
    a = ~a;
    return 4;
}

// STC instruction handler
uint32_t CPU::op_stc(Memory&) {
    set_flag(FLAG_CARRY, true);
    return 4;
}

// CMC instruction handler
uint32_t CPU::op_cmc(Memory&) {
    set_flag(FLAG_CARRY, !get_flag(FLAG_CARRY));
    return 4;
}
