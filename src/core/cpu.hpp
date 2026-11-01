#ifndef CPU_HPP
#define CPU_HPP

#include <array>
#include <cstdint>

class Memory;
class IOBus;

class CPU {
public:
    CPU();

    uint32_t step(Memory& memory);
    uint32_t interrupt(Memory& memory, uint8_t rst_num);

    // Attach machine I/O handlers
    void set_io(IOBus* bus);

private:

    // ============================================================
    // CPU STATE
    // ============================================================

    uint8_t a, b, c, d, e, h, l;
    uint16_t pc, sp;
    uint8_t flags;
    uint8_t current_opcode;

    // Interrupt / execution state
    bool interrupts_enabled;
    bool ei_pending;
    bool halted;

    // External I/O bus
    IOBus* io;

    // ============================================================
    // FLAG MASKS
    // ============================================================

    static constexpr uint8_t FLAG_CARRY     = 0x01;
    static constexpr uint8_t FLAG_PARITY    = 0x04;
    static constexpr uint8_t FLAG_AUX_CARRY = 0x10;
    static constexpr uint8_t FLAG_ZERO      = 0x40;
    static constexpr uint8_t FLAG_SIGN      = 0x80;

    // ============================================================
    // OPCODE DISPATCH
    // ============================================================

    using OpFunc = uint32_t (CPU::*)(Memory& memory);

    std::array<OpFunc, 256> table;

    // ============================================================
    // FETCH / STACK HELPERS
    // ============================================================

    uint8_t fetch_byte(Memory& memory);
    uint16_t fetch_word(Memory& memory);

    void push_word(Memory& memory, uint16_t value);
    uint16_t pop_word(Memory& memory);

    // ============================================================
    // REGISTER HELPERS
    // ============================================================

    uint8_t read_reg_or_memory(uint8_t index, Memory& memory);
    void write_reg_or_memory(
        uint8_t index,
        uint8_t value,
        Memory& memory
    );

    // ============================================================
    // CONDITION / FLAG HELPERS
    // ============================================================

    bool check_condition(uint8_t cc) const;

    void update_flags_add(
        uint8_t a,
        uint8_t value,
        uint16_t result
    );

    void update_flags_sub(
        uint8_t a,
        uint8_t value,
        uint16_t result
    );

    void update_flags_logic(uint8_t result);
    void update_flags_zsp(uint8_t result);

    void set_flag(uint8_t mask, bool value);
    bool get_flag(uint8_t mask) const;

    static bool parity_even(uint8_t value);

    // ============================================================
    // ALU HELPERS
    // ============================================================

    void alu_add(uint8_t value, uint8_t carry_in);
    void alu_sub(
        uint8_t value,
        uint8_t borrow_in,
        bool store_result
    );

    void alu_and(uint8_t value);
    void alu_xor(uint8_t value);
    void alu_or(uint8_t value);

    // ============================================================
    // OPCODE HANDLERS
    // ============================================================

    // Catch-all
    uint32_t op_unimplemented(Memory& memory);

    // ------------------------------------------------------------
    // Base Instructions
    // ------------------------------------------------------------

    uint32_t op_nop(Memory& memory);

    // ------------------------------------------------------------
    // Machine Control
    // ------------------------------------------------------------

    uint32_t op_hlt(Memory& memory);
    uint32_t op_ei(Memory& memory);
    uint32_t op_di(Memory& memory);

    // ------------------------------------------------------------
    // I/O
    // ------------------------------------------------------------

    uint32_t op_in(Memory& memory);
    uint32_t op_out(Memory& memory);

    // ------------------------------------------------------------
    // Data Movement
    // ------------------------------------------------------------

    uint32_t op_mov(Memory& memory);
    uint32_t op_mvi(Memory& memory);

    uint32_t op_lxi(Memory& memory);

    uint32_t op_lda(Memory& memory);
    uint32_t op_sta(Memory& memory);

    uint32_t op_ldax(Memory& memory);
    uint32_t op_stax(Memory& memory);

    uint32_t op_lhld(Memory& memory);
    uint32_t op_shld(Memory& memory);

    uint32_t op_xchg(Memory& memory);

    // ------------------------------------------------------------
    // Register Pair Operations
    // ------------------------------------------------------------

    uint32_t op_inx(Memory& memory);
    uint32_t op_dcx(Memory& memory);
    uint32_t op_dad(Memory& memory);

    // ------------------------------------------------------------
    // Stack Operations
    // ------------------------------------------------------------

    uint32_t op_push(Memory& memory);
    uint32_t op_pop(Memory& memory);

    uint32_t op_xthl(Memory& memory);
    uint32_t op_sphl(Memory& memory);

    // ------------------------------------------------------------
    // Arithmetic
    // ------------------------------------------------------------

    uint32_t op_add(Memory& memory);
    uint32_t op_adc(Memory& memory);
    uint32_t op_sub(Memory& memory);
    uint32_t op_sbb(Memory& memory);

    uint32_t op_inr(Memory& memory);
    uint32_t op_dcr(Memory& memory);

    uint32_t op_daa(Memory& memory);

    // ------------------------------------------------------------
    // Logical
    // ------------------------------------------------------------

    uint32_t op_ana(Memory& memory);
    uint32_t op_xra(Memory& memory);
    uint32_t op_ora(Memory& memory);
    uint32_t op_cmp(Memory& memory);

    uint32_t op_cma(Memory& memory);
    uint32_t op_stc(Memory& memory);
    uint32_t op_cmc(Memory& memory);

    // ------------------------------------------------------------
    // Immediate Arithmetic / Logical
    // ------------------------------------------------------------

    uint32_t op_adi(Memory& memory);
    uint32_t op_aci(Memory& memory);
    uint32_t op_sui(Memory& memory);
    uint32_t op_sbi(Memory& memory);

    uint32_t op_ani(Memory& memory);
    uint32_t op_xri(Memory& memory);
    uint32_t op_ori(Memory& memory);

    uint32_t op_cpi(Memory& memory);

    // ------------------------------------------------------------
    // Rotates
    // ------------------------------------------------------------

    uint32_t op_rlc(Memory& memory);
    uint32_t op_rrc(Memory& memory);
    uint32_t op_ral(Memory& memory);
    uint32_t op_rar(Memory& memory);

    // ------------------------------------------------------------
    // Jumps
    // ------------------------------------------------------------

    uint32_t op_jmp(Memory& memory);
    uint32_t op_jcc(Memory& memory);
    uint32_t op_pchl(Memory& memory);

    // ------------------------------------------------------------
    // Calls / Returns / Restarts
    // ------------------------------------------------------------

    uint32_t op_call(Memory& memory);
    uint32_t op_ccc(Memory& memory);

    uint32_t op_ret(Memory& memory);
    uint32_t op_rcc(Memory& memory);

    uint32_t op_rst(Memory& memory);

    // ============================================================
    // TEST ACCESS
    // ============================================================

    friend struct CPUTestAccess;
};

#endif