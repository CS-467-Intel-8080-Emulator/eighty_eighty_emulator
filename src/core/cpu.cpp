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

        // --- Branching group (0xC0 - 0xFF) ---
        for (uint16_t opcode = 0xC0; opcode <= 0xFF; opcode++)
            table[opcode] = &CPU::op_branching;

        // --- Immediates (must follow the branching loop) ---
        table[0xC6] = &CPU::op_adi;
        table[0xCE] = &CPU::op_aci;
        table[0xD6] = &CPU::op_sui;
        table[0xDE] = &CPU::op_sbi;
        table[0xE6] = &CPU::op_ani;
        table[0xEE] = &CPU::op_xri;
        table[0xF6] = &CPU::op_ori;
        table[0xFE] = &CPU::op_cpi;
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
