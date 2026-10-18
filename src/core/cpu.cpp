#include "cpu.hpp"
#include "memory.hpp"

/*
Constructor for the CPU class
 */
CPU::CPU()
    : a(0), b(0), c(0), d(0), e(0), h(0), l(0),
      pc(0), sp(0), flags(0), 
      halted(false)
    {
        // --- Explicit opcodes (individually handled opcodes) ---
        table[0x00] = &CPU::op_nop;
        table[0x01] = &CPU::op_lxi;
        table[0x11] = &CPU::op_lxi;
        table[0x21] = &CPU::op_lxi;
        table[0x31] = &CPU::op_lxi;
        table[0x03] = &CPU::op_inx;
        table[0x13] = &CPU::op_inx;
        table[0x23] = &CPU::op_inx;
        table[0x33] = &CPU::op_inx;
        table[0x0B] = &CPU::op_dcx;
        table[0x1B] = &CPU::op_dcx;
        table[0x2B] = &CPU::op_dcx;
        table[0x3B] = &CPU::op_dcx;
        table[0x3A] = &CPU::op_lda;
        table[0x32] = &CPU::op_sta;
        table[0x76] = &CPU::op_hlt; // HLT instruction for MOV block
        table[0x2A] = &CPU::op_lhld;
        table[0x22] = &CPU::op_shld;
        table[0x0A] = &CPU::op_ldax;
        table[0x1A] = &CPU::op_ldax;
        table[0x02] = &CPU::op_stax; // STAX
        table[0x12] = &CPU::op_stax; // STAX
        table[0xEB] = &CPU::op_xchg; // XCHG
        table[0xC3] = &CPU::op_jmp; // JMP
        table[0xE9] = &CPU::op_pchl; // PCHL
        table[0xC2] = &CPU::op_jcond; // JNZ
        table[0xCA] = &CPU::op_jcond; // JZ
        table[0xD2] = &CPU::op_jcond; // JNC
        table[0xDA] = &CPU::op_jcond; // JC
        table[0xE2] = &CPU::op_jcond; // JPO
        table[0xEA] = &CPU::op_jcond; // JPE
        table[0xF2] = &CPU::op_jcond; // JP
        table[0xFA] = &CPU::op_jcond; // JM
        table[0xCD] = &CPU::op_call; // CALL
        table[0xC9] = &CPU::op_ret; // RET
        table[0xC4] = &CPU::op_ccond; // CNZ
        table[0xCC] = &CPU::op_ccond; // CZ
        table[0xD4] = &CPU::op_ccond; // CNC
        table[0xDC] = &CPU::op_ccond; // CC
        table[0xE4] = &CPU::op_ccond; // CPO
        table[0xEC] = &CPU::op_ccond; // CPE
        table[0xF4] = &CPU::op_ccond; // CP
        table[0xFC] = &CPU::op_ccond; // CM
        table[0xC0] = &CPU::op_rcond; // RNZ
        table[0xC8] = &CPU::op_rcond; // RZ
        table[0xD0] = &CPU::op_rcond; // RNC
        table[0xD8] = &CPU::op_rcond; // RC
        table[0xE0] = &CPU::op_rcond; // RPO
        table[0xE8] = &CPU::op_rcond; // RPE
        table[0xF0] = &CPU::op_rcond; // RP
        table[0xF8] = &CPU::op_rcond; // RM

        // --- MVI group (0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x36, 0x3E) ---
        for (uint8_t opcode = 0x06; opcode <= 0x3E; opcode += 0x08) 
            table[opcode] = &CPU::op_mvi;
            
        // --- MOV group (0x40 - 0x7F) ---
        for (uint8_t opcode = 0x40; opcode <= 0x7F; opcode++) 
            table[opcode] = &CPU::op_mov;
        
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

    }

 uint32_t CPU::step(Memory& memory) {
    if (halted) {
        return 7; // HLT consumes 7 cycles
    }
    uint8_t opcode = memory.read(pc);
    pc++;
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
    uint8_t lo = memory.read(pc++);
    uint8_t hi = memory.read(pc++);
    return (hi << 8) | lo;
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

// MOV r1, r2 instruction handler
uint32_t CPU::op_mov(Memory& memory) {
    uint8_t src = current_opcode & 0x07; 
    uint8_t dst = (current_opcode >> 3) & 0x07;

    uint8_t value = read_reg_or_memory(src, memory);
    write_reg_or_memory(dst, value, memory);

    return 7;
}

// ADD r instruction handler
uint32_t CPU::op_add(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    uint8_t value = read_reg_or_memory(src, memory);

    uint16_t result = (uint16_t)a + value;
    update_flags_add(a, value, result);

    a = result & 0xFF;
    return 4;
}

// ADC r instruction handler
uint32_t CPU::op_adc(Memory& memory) {
    uint8_t src = current_opcode & 0x07;
    uint8_t value = read_reg_or_memory(src, memory);
    uint8_t carry = (flags & 0x01) ? 1 : 0;

    uint16_t result = (uint16_t)a + value + carry;
    update_flags_add(a, value + carry, result);

    a = result & 0xFF;
    return 4;
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

// LHLD instruction handler
uint32_t CPU::op_lhld(Memory& memory){
    uint16_t addr = fetch_word(memory);
    // Load L then H from memory
    l = memory.read(addr);
    h = memory.read(addr + 1);
    return 16;
}

// SHLD instruction handler
uint32_t CPU::op_shld(Memory& memory){
    uint16_t addr = fetch_word(memory);
    // Store L then H to memory
    memory.write(addr, l);
    memory.write(addr + 1, h);
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


// STAX instruction handler
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

// HLT instruction handler
uint32_t CPU::op_hlt(Memory& memory) {
    halted = true;
    return 0;
}

// JMP instruction handler
uint32_t CPU::op_jmp(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    pc = addr;
    return 10;
}

// PCHL instruction handler
uint32_t CPU::op_pchl(Memory& memory) {
    pc = (h << 8) | l;
    return 5;
}

// Branching instruction handler
uint32_t CPU::op_jcond(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    bool should_jump = false;
    switch (current_opcode) {
        case 0xC2: should_jump = !(flags & FLAG_ZERO); break; // JNZ
        case 0xCA: should_jump = (flags & FLAG_ZERO); break; // JZ
        case 0xD2: should_jump = !(flags & FLAG_CARRY); break; // JNC
        case 0xDA: should_jump = (flags & FLAG_CARRY); break; // JC
        case 0xE2: should_jump = !(flags & FLAG_PARITY); break; // JPO
        case 0xEA: should_jump = (flags & FLAG_PARITY); break; // JPE
        case 0xF2: should_jump = !(flags & FLAG_SIGN); break; // JP
        case 0xFA: should_jump = (flags & FLAG_SIGN); break; // JM
    }
    if (should_jump) {
        pc = addr;
        return 10; // Jump taken
    }
    return 10; // Jump not taken
}

// CALL instruction handler
uint32_t CPU::op_call(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    // Push current PC onto stack
    uint16_t return_addr = pc;
    sp -= 2;
    memory.write(sp, return_addr & 0xFF); // Low byte
    memory.write(sp + 1, return_addr >> 8); // High byte
    pc = addr;
    return 17; // CALL takes 17 cycles
}

// RET instruction handler
uint32_t CPU::op_ret(Memory& memory) {
    uint8_t lo = memory.read(sp);
    uint8_t hi = memory.read(sp + 1);
    sp += 2;
    pc = (hi << 8) | lo;
    return 10; // RET takes 10 cycles
}

// CCOND instruction handler
uint32_t CPU::op_ccond(Memory& memory) {
    uint16_t addr = fetch_word(memory);
    bool should_call = false;
    switch (current_opcode) {
        case 0xC4: should_call = !(flags & FLAG_ZERO); break; // CNZ
        case 0xCC: should_call = (flags & FLAG_ZERO); break; // CZ
        case 0xD4: should_call = !(flags & FLAG_CARRY); break; // CNC
        case 0xDC: should_call = (flags & FLAG_CARRY); break; // CC
        case 0xE4: should_call = !(flags & FLAG_PARITY); break; // CPO
        case 0xEC: should_call = (flags & FLAG_PARITY); break; // CPE
        case 0xF4: should_call = !(flags & FLAG_SIGN); break; // CP 
        case 0xFC: should_call = (flags & FLAG_SIGN); break; // CM
    }
    if (should_call) {
        // Push current PC onto stack
        uint16_t return_addr = pc;
        sp -= 2;
        memory.write(sp, return_addr & 0xFF); // Low byte
        memory.write(sp + 1, return_addr >> 8); // High byte
        pc = addr;
        return 17; // CALL takes 17 cycles
    }
    return 11; // CALL not taken
}

// RCOND instruction handler
uint32_t CPU::op_rcond(Memory& memory) {
    bool should_return = false;
    switch (current_opcode) {
        case 0xC0: should_return = !(flags & FLAG_ZERO); break; // RNZ
        case 0xC8: should_return = (flags & FLAG_ZERO); break; // RZ
        case 0xD0: should_return = !(flags & FLAG_CARRY); break; // RNC
        case 0xD8: should_return = (flags & FLAG_CARRY); break; // RC
        case 0xE0: should_return = !(flags & FLAG_PARITY); break; // RPO
        case 0xE8: should_return = (flags & FLAG_PARITY); break; // RPE
        case 0xF0: should_return = !(flags & FLAG_SIGN); break; // RP
        case 0xF8: should_return = (flags & FLAG_SIGN); break; // RM
    }
    if (should_return) {
        uint8_t lo = memory.read(sp);
        uint8_t hi = memory.read(sp + 1);
        sp += 2;
        pc = (hi << 8) | lo;
        return 10; // RET takes 10 cycles when taken
    }
    return 5; // RET not taken
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
