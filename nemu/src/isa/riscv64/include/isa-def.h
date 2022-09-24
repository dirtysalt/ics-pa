#ifndef __ISA_RISCV64_H__
#define __ISA_RISCV64_H__

#include <common.h>

// https://ibex-core.readthedocs.io/en/latest/03_reference/cs_registers.html#
#define CSR_START_ADDR 0x300
#define CSR_END_ADDR 0x346

#define CSR_MSTATUS 0x300
#define CSR_MTVEC 0x305
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342
#define CSR_MSTATUS_INIT 0xa00001800

typedef struct {
    union {
        uint64_t _64;
    } gpr[32];
    vaddr_t pc;
    union {
        uint64_t _64;
    } csr[CSR_END_ADDR - CSR_START_ADDR];
//    uint64_t satp;
} riscv64_CPU_state;

#define csr(idx) (cpu.csr[idx - CSR_START_ADDR]._64)

// decode
typedef struct {
    union {
        struct {
            uint32_t opcode1_0 : 2;
            uint32_t opcode6_2 : 5;
            uint32_t rd : 5;
            uint32_t funct3 : 3;
            uint32_t rs1 : 5;
            int32_t simm11_0 : 12;
        } i;
        struct {
            uint32_t opcode1_0 : 2;
            uint32_t opcode6_2 : 5;
            uint32_t imm4_0 : 5;
            uint32_t funct3 : 3;
            uint32_t rs1 : 5;
            uint32_t rs2 : 5;
            int32_t simm11_5 : 7;
        } s;
        struct {
            uint32_t opcode1_0 : 2;
            uint32_t opcode6_2 : 5;
            uint32_t rd : 5;
            int32_t simm31_12 : 20;
        } u;
        uint32_t val;
    } instr;
} riscv64_ISADecodeInfo;

#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif
