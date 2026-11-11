#ifndef CPU_HPP
#define CPU_HPP

#include <array>
#include <cstdint>

class MemoryBus;
class IOBus;
class StateWriter;
class StateReader;

class CPU {
public:
    CPU();

    uint32_t step(MemoryBus& memory);
    uint32_t interrupt(MemoryBus& memory, uint8_t rst_num);
    bool is_halted() const { return halted; }

    // Attach machine I/O handlers
    void set_io(IOBus* bus);

    // Read-only register access
    uint8_t get_a() const { return a; }
    uint8_t get_b() const { return b; }
    uint8_t get_c() const { return c; }
    uint8_t get_d() const { return d; }
    uint8_t get_e() const { return e; }
    uint8_t get_h() const { return h; }
    uint8_t get_l() const { return l; }
    uint8_t get_flags() const { return flags; }
    uint16_t get_pc() const { return pc; }
    uint16_t get_sp() const { return sp; }

    // Setters for loading and running programs
    void set_pc(uint16_t value) { pc = value; }
    void set_sp(uint16_t value) { sp = value; }

    // Save and load state
    void save_state(StateWriter& writer) const;
    void load_state(StateReader& reader);

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

    using OpFunc = uint32_t (CPU::*)(MemoryBus& memory);

    std::array<OpFunc, 256> table;

    // ============================================================
    // FETCH / STACK HELPERS
    // ============================================================

    uint8_t fetch_byte(MemoryBus& memory);
    uint16_t fetch_word(MemoryBus& memory);

    void push_word(MemoryBus& memory, uint16_t value);
    uint16_t pop_word(MemoryBus& memory);

    // ============================================================
    // REGISTER HELPERS
    // ============================================================

    uint8_t read_reg_or_memory(uint8_t index, MemoryBus& memory);
    void write_reg_or_memory(
        uint8_t index,
        uint8_t value,
        MemoryBus& memory
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
    uint32_t op_unimplemented(MemoryBus& memory);

    // ------------------------------------------------------------
    // Base Instructions
    // ------------------------------------------------------------

    uint32_t op_nop(MemoryBus& memory);

    // ------------------------------------------------------------
    // Machine Control
    // ------------------------------------------------------------

    uint32_t op_hlt(MemoryBus& memory);
    uint32_t op_ei(MemoryBus& memory);
    uint32_t op_di(MemoryBus& memory);

    // ------------------------------------------------------------
    // I/O
    // ------------------------------------------------------------

    uint32_t op_in(MemoryBus& memory);
    uint32_t op_out(MemoryBus& memory);

    // ------------------------------------------------------------
    // Data Movement
    // ------------------------------------------------------------

    uint32_t op_mov(MemoryBus& memory);
    uint32_t op_mvi(MemoryBus& memory);

    uint32_t op_lxi(MemoryBus& memory);

    uint32_t op_lda(MemoryBus& memory);
    uint32_t op_sta(MemoryBus& memory);

    uint32_t op_ldax(MemoryBus& memory);
    uint32_t op_stax(MemoryBus& memory);

    uint32_t op_lhld(MemoryBus& memory);
    uint32_t op_shld(MemoryBus& memory);

    uint32_t op_xchg(MemoryBus& memory);

    // ------------------------------------------------------------
    // Register Pair Operations
    // ------------------------------------------------------------

    uint32_t op_inx(MemoryBus& memory);
    uint32_t op_dcx(MemoryBus& memory);
    uint32_t op_dad(MemoryBus& memory);

    // ------------------------------------------------------------
    // Stack Operations
    // ------------------------------------------------------------

    uint32_t op_push(MemoryBus& memory);
    uint32_t op_pop(MemoryBus& memory);

    uint32_t op_xthl(MemoryBus& memory);
    uint32_t op_sphl(MemoryBus& memory);

    // ------------------------------------------------------------
    // Arithmetic
    // ------------------------------------------------------------

    uint32_t op_add(MemoryBus& memory);
    uint32_t op_adc(MemoryBus& memory);
    uint32_t op_sub(MemoryBus& memory);
    uint32_t op_sbb(MemoryBus& memory);

    uint32_t op_inr(MemoryBus& memory);
    uint32_t op_dcr(MemoryBus& memory);

    uint32_t op_daa(MemoryBus& memory);

    // ------------------------------------------------------------
    // Logical
    // ------------------------------------------------------------

    uint32_t op_ana(MemoryBus& memory);
    uint32_t op_xra(MemoryBus& memory);
    uint32_t op_ora(MemoryBus& memory);
    uint32_t op_cmp(MemoryBus& memory);

    uint32_t op_cma(MemoryBus& memory);
    uint32_t op_stc(MemoryBus& memory);
    uint32_t op_cmc(MemoryBus& memory);

    // ------------------------------------------------------------
    // Immediate Arithmetic / Logical
    // ------------------------------------------------------------

    uint32_t op_adi(MemoryBus& memory);
    uint32_t op_aci(MemoryBus& memory);
    uint32_t op_sui(MemoryBus& memory);
    uint32_t op_sbi(MemoryBus& memory);

    uint32_t op_ani(MemoryBus& memory);
    uint32_t op_xri(MemoryBus& memory);
    uint32_t op_ori(MemoryBus& memory);

    uint32_t op_cpi(MemoryBus& memory);

    // ------------------------------------------------------------
    // Rotates
    // ------------------------------------------------------------

    uint32_t op_rlc(MemoryBus& memory);
    uint32_t op_rrc(MemoryBus& memory);
    uint32_t op_ral(MemoryBus& memory);
    uint32_t op_rar(MemoryBus& memory);

    // ------------------------------------------------------------
    // Jumps
    // ------------------------------------------------------------

    uint32_t op_jmp(MemoryBus& memory);
    uint32_t op_jcc(MemoryBus& memory);
    uint32_t op_pchl(MemoryBus& memory);

    // ------------------------------------------------------------
    // Calls / Returns / Restarts
    // ------------------------------------------------------------

    uint32_t op_call(MemoryBus& memory);
    uint32_t op_ccc(MemoryBus& memory);

    uint32_t op_ret(MemoryBus& memory);
    uint32_t op_rcc(MemoryBus& memory);

    uint32_t op_rst(MemoryBus& memory);

    // ============================================================
    // TEST ACCESS
    // ============================================================

    friend struct CPUTestAccess;
};

#endif