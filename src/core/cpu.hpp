#ifndef CPU_HPP
#define CPU_HPP

#include <array>
#include <cstdint>

class Memory;

class CPU {
    public:
        CPU();
        uint32_t step(Memory& memory);

    private:
        using OpFunc = uint32_t (CPU::*)(Memory& memory);
        
        // Opcode table
        std::array<OpFunc, 256> table;

        // CPU state
        uint8_t a, b, c, d, e, h, l;
        uint16_t pc, sp;
        uint8_t flags;
        uint8_t current_opcode;
        bool halted;

        // Fetch helpers
        uint8_t fetch_byte(Memory& memory);
        uint16_t fetch_word(Memory& memory);

        // Register/memory helpers
        uint8_t read_reg_or_memory(uint8_t index, Memory& memory);
        void write_reg_or_memory(uint8_t index, uint8_t value, Memory& memory);

        // Flag helpers
        void update_flags_add(uint8_t a, uint8_t value, uint16_t result);
        void update_flags_sub(uint8_t a, uint8_t value, uint16_t result);
        void update_flags_logic(uint8_t result);

        // Flag masks
        static constexpr uint8_t FLAG_CARRY = 0x01;
        static constexpr uint8_t FLAG_PARITY = 0x04;
        static constexpr uint8_t FLAG_AUX_CARRY = 0x10;
        static constexpr uint8_t FLAG_ZERO = 0x40;
        static constexpr uint8_t FLAG_SIGN = 0x80;

        // Opcode handlers
        uint32_t op_nop(Memory& memory);

        // Data movement
        uint32_t op_mov(Memory& memory);
        uint32_t op_mvi(Memory& memory);
        uint32_t op_lxi(Memory& memory);
        uint32_t op_inx(Memory& memory);
        uint32_t op_lda(Memory& memory);
        uint32_t op_sta(Memory& memory);
        uint32_t op_ldax(Memory& memory);
        uint32_t op_stax(Memory& memory);
        uint32_t op_lhld(Memory& memory);
        uint32_t op_shld(Memory& memory);
        uint32_t op_xchg(Memory& memory);
        uint32_t op_dcx(Memory& memory);
        uint32_t op_hlt(Memory& memory);

        // Arithmetic
        uint32_t op_add(Memory& memory);
        uint32_t op_adc(Memory& memory);
        uint32_t op_sub(Memory& memory);
        uint32_t op_sbb(Memory& memory);
        uint32_t op_ana(Memory& memory);
        uint32_t op_xra(Memory& memory);
        uint32_t op_ora(Memory& memory);
        uint32_t op_cmp(Memory& memory);

        // Control flow / Branching
        uint32_t op_jmp(Memory& memory);
        uint32_t op_call(Memory& memory);
        uint32_t op_ret(Memory& memory);
        uint32_t op_pchl(Memory& memory);
        uint32_t op_jcond(Memory& memory);
        uint32_t op_ccond(Memory& memory);
        uint32_t op_rcond(Memory& memory);
    };

#endif