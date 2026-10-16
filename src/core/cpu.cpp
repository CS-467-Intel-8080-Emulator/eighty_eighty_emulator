#include "cpu.hpp"
#include "memory.hpp"

/*
Constructor for the CPU class
 */
CPU::CPU()
    : a(0), b(0), c(0), d(0), e(0), h(0), l(0),
      pc(0), sp(0), flags(0) 
    
    {
        // --- Explicit opcodes ---
        table[0x00] = &CPU::op_nop;
        table[0x01] = &CPU::op_lxi_b;
        table[0x02] = &CPU::op_stax_b;
        table[0x03] = &CPU::op_inx_b;
        table[0x3A] = &CPU::op_lda;
        table[0x32] = &CPU::op_sta;

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

        // --- Branching group (0xC0 - 0xFF) ---
        for (uint8_t opcode = 0xC0; opcode <= 0xFF; opcode++)
            table[opcode] = &CPU::op_branching;
    }

 uint32_t CPU::step(Memory& memory) {
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

// LXI B instruction handler
uint32_t CPU::op_lxi_b(Memory& memory) {
    uint16_t value = fetch_word(memory);
    c = value & 0xFF;
    b = value >> 8;
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

// LHLD instruction handler

// SHLD instruction handler

// LDAX instruction handler

// STAX D instruction handler

// XCHG instruction handler
    

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
