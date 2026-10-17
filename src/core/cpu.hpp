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

        // Fetch helpers
        uint8_t fetch_byte(Memory& memory);
        uint16_t fetch_word(Memory& memory);

        // Condition helper (Jcc, Ccc, Rcc)
        bool check_condition(uint8_t cc) const;

        // Register/memory helpers
        uint8_t read_reg_or_memory(uint8_t index, Memory& memory);
        void write_reg_or_memory(uint8_t index, uint8_t value, Memory& memory);

        // Flag helpers
        void update_flags_add(uint8_t a, uint8_t value, uint16_t result);
        void update_flags_sub(uint8_t a, uint8_t value, uint16_t result);
        void update_flags_logic(uint8_t result);
        void update_flags_zsp(uint8_t result);
        void set_flag(uint8_t mask, bool value);
        bool get_flag(uint8_t mask) const;
        static bool parity_even(uint8_t value);

        // ALU helpers
        void alu_add(uint8_t value, uint8_t carry_in);
        void alu_sub(uint8_t value, uint8_t borrow_in, bool store_result);
        void alu_and(uint8_t value);
        void alu_xor(uint8_t value);
        void alu_or(uint8_t value);

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
        uint32_t op_lxi_b(Memory& memory);
        uint32_t op_stax_b(Memory& memory);
        uint32_t op_inx_b(Memory& memory);
        uint32_t op_lda(Memory& memory);
        uint32_t op_sta(Memory& memory);
        uint32_t op_ldax(Memory& memory);
        uint32_t op_stax(Memory& memory);
        uint32_t op_lhld(Memory& memory);
        uint32_t op_shld(Memory& memory);
        uint32_t op_xchg(Memory& memory);

        // Arithmetic
        uint32_t op_add(Memory& memory);
        uint32_t op_adc(Memory& memory);
        uint32_t op_sub(Memory& memory);
        uint32_t op_sbb(Memory& memory);
        uint32_t op_ana(Memory& memory);
        uint32_t op_xra(Memory& memory);
        uint32_t op_ora(Memory& memory);
        uint32_t op_cmp(Memory& memory);
        uint32_t op_inr(Memory& memory);
        uint32_t op_dcr(Memory& memory);
        uint32_t op_dad(Memory& memory);
        uint32_t op_daa(Memory& memory);

        // Immediates
        uint32_t op_adi(Memory& memory);
        uint32_t op_aci(Memory& memory);
        uint32_t op_sui(Memory& memory);
        uint32_t op_sbi(Memory& memory);
        uint32_t op_ani(Memory& memory);
        uint32_t op_xri(Memory& memory);
        uint32_t op_ori(Memory& memory);
        uint32_t op_cpi(Memory& memory);

        // Rotates and carry
        uint32_t op_rlc(Memory& memory);
        uint32_t op_rrc(Memory& memory);
        uint32_t op_ral(Memory& memory);
        uint32_t op_rar(Memory& memory);
        uint32_t op_cma(Memory& memory);
        uint32_t op_stc(Memory& memory);
        uint32_t op_cmc(Memory& memory);

        // Control flow / Branching
        uint32_t op_branching(Memory& memory);
        uint32_t op_jmp(Memory& memory);
        uint32_t op_jcc(Memory& memory);
        uint32_t op_call(Memory& memory);
        uint32_t op_ret(Memory& memory);

        // Unit test access
        friend struct CPUTestAccess;

    };

#endif