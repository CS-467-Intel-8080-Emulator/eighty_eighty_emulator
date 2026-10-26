#ifndef CPU_HPP
#define CPU_HPP

#include <array>
#include <cstdint>

class MemoryBus;
class IOBus;

class CPU {
    public:
        CPU();
        uint32_t step(MemoryBus& memory);
        uint32_t interrupt(MemoryBus& memory, uint8_t rst_num);

        // Attach the machine's port handlers for IN/OUT
        void set_io(IOBus* bus);

    private:
        using OpFunc = uint32_t (CPU::*)(MemoryBus& memory);
        
        // Opcode table
        std::array<OpFunc, 256> table;

        // unimplmented code handler
        uint32_t op_unimplemented(MemoryBus& memory);


        // CPU state
        uint8_t a, b, c, d, e, h, l;
        uint16_t pc, sp;
        uint8_t flags;
        uint8_t current_opcode;

        // Interrupt/halt state
        bool interrupts_enabled;
        bool ei_pending;
        bool halted;

        // Port I/O (IN/OUT)
        IOBus* io;

        // Fetch helpers
        uint8_t fetch_byte(MemoryBus& memory);
        uint16_t fetch_word(MemoryBus& memory);

        // Condition helper (Jcc, Ccc, Rcc)
        bool check_condition(uint8_t cc) const;

        // Stack helpers (CALL, Ccc, RST, RET, Rcc, interrupts)
        void push_word(MemoryBus& memory, uint16_t value);
        uint16_t pop_word(MemoryBus& memory);

        // Register/memory helpers
        uint8_t read_reg_or_memory(uint8_t index, MemoryBus& memory);
        void write_reg_or_memory(uint8_t index, uint8_t value, MemoryBus& memory);

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
        uint32_t op_nop(MemoryBus& memory);


        // Machine control
        uint32_t op_hlt(MemoryBus& memory);
        uint32_t op_ei(MemoryBus& memory);
        uint32_t op_di(MemoryBus& memory);

        // I/O
        uint32_t op_in(MemoryBus& memory);
        uint32_t op_out(MemoryBus& memory);

        // Data movement
        uint32_t op_mov(MemoryBus& memory);
        uint32_t op_mvi(MemoryBus& memory);
        uint32_t op_lxi(MemoryBus& memory);
        uint32_t op_inx(MemoryBus& memory);
        uint32_t op_dcx(MemoryBus& memory);
        uint32_t op_lda(MemoryBus& memory);
        uint32_t op_sta(MemoryBus& memory);
        uint32_t op_ldax(MemoryBus& memory);
        uint32_t op_stax(MemoryBus& memory);
        uint32_t op_lhld(MemoryBus& memory);
        uint32_t op_shld(MemoryBus& memory);
        uint32_t op_xchg(MemoryBus& memory);

        // Arithmetic
        uint32_t op_add(MemoryBus& memory);
        uint32_t op_adc(MemoryBus& memory);
        uint32_t op_sub(MemoryBus& memory);
        uint32_t op_sbb(MemoryBus& memory);
        uint32_t op_ana(MemoryBus& memory);
        uint32_t op_xra(MemoryBus& memory);
        uint32_t op_ora(MemoryBus& memory);
        uint32_t op_cmp(MemoryBus& memory);
        uint32_t op_inr(MemoryBus& memory);
        uint32_t op_dcr(MemoryBus& memory);
        uint32_t op_dad(MemoryBus& memory);
        uint32_t op_daa(MemoryBus& memory);

        // Immediates
        uint32_t op_adi(MemoryBus& memory);
        uint32_t op_aci(MemoryBus& memory);
        uint32_t op_sui(MemoryBus& memory);
        uint32_t op_sbi(MemoryBus& memory);
        uint32_t op_ani(MemoryBus& memory);
        uint32_t op_xri(MemoryBus& memory);
        uint32_t op_ori(MemoryBus& memory);
        uint32_t op_cpi(MemoryBus& memory);

        // Rotates and carry
        uint32_t op_rlc(MemoryBus& memory);
        uint32_t op_rrc(MemoryBus& memory);
        uint32_t op_ral(MemoryBus& memory);
        uint32_t op_rar(MemoryBus& memory);
        uint32_t op_cma(MemoryBus& memory);
        uint32_t op_stc(MemoryBus& memory);
        uint32_t op_cmc(MemoryBus& memory);

        // Control flow / Branching
        uint32_t op_branching(MemoryBus& memory);
        uint32_t op_jmp(MemoryBus& memory);
        uint32_t op_jcc(MemoryBus& memory);
        uint32_t op_call(MemoryBus& memory);
        uint32_t op_ccc(MemoryBus& memory);
        uint32_t op_ret(MemoryBus& memory);
        uint32_t op_rcc(MemoryBus& memory);
        uint32_t op_rst(MemoryBus& memory);
        uint32_t op_pchl(MemoryBus& memory);

        // Unit test access
        friend struct CPUTestAccess;

    };

#endif