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


        // Opcode handlers
        uint32_t op_nop(Memory& memory);
        uint32_t op_lxi_b(Memory& memory);
        uint32_t op_stax_b(Memory& memory);
        uint32_t op_inx_b(Memory& memory);

        uint32_t op_mov(Memory& memory);
        uint32_t op_add(Memory& memory);
        uint32_t op_adc(Memory& memory);
        uint32_t op_sub(Memory& memory);
        uint32_t op_sbb(Memory& memory);
        uint32_t op_ana(Memory& memory);
        uint32_t op_xra(Memory& memory);
        uint32_t op_ora(Memory& memory);
        uint32_t op_cmp(Memory& memory);

        uint32_t op_branching(Memory& memory);

        // Flag helpers

        
        // Register and memory helpers
        uint8_t read_reg_or_memory(uint8_t index, Memory& memory);

        void write_reg_or_memory(uint8_t index, uint8_t value, Memory& memory);
    
        void update_flags_add(uint8_t a, uint8_t value, uint16_t result);
    };

#endif