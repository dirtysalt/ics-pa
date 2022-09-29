#ifndef __ISA_RISCV64_H__
#define __ISA_RISCV64_H__

#include <common.h>

// https://ibex-core.readthedocs.io/en/latest/03_reference/cs_registers.html#
#define CSR_START_ADDR 0x180
#define CSR_SATP 0x180
#define CSR_MSTATUS 0x300
#define CSR_MTVEC 0x305
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342
#define CSR_END_ADDR 0x346
#define MIE 3
#define MPIE 7
// MIE is 0 when initialized.
#define CSR_MSTATUS_INIT 0xa00001800

#define IRQ_TIMER 0x8000000000000007 // for riscv64

typedef struct {
    union {
        uint64_t _64;
    } gpr[32];
    vaddr_t pc;
    union {
        uint64_t _64;
    } csr[CSR_END_ADDR - CSR_START_ADDR];
    bool INTR;
} riscv64_CPU_state;

extern riscv64_CPU_state cpu;
#define csr(idx) (cpu.csr[idx - CSR_START_ADDR]._64)

static inline void enable_intr() {
    word_t st = csr(CSR_MSTATUS);
    // move MPIE to to MIE
    word_t b = ((st >> MPIE) & 0x1) << MIE;
    // turn off MIE
    // turn on MPIE
    word_t off = (st & (~(1 << MIE))) | (1 << MPIE);
    csr(CSR_MSTATUS) = b | off;
}

static inline void disable_intr() {
    word_t st = csr(CSR_MSTATUS);
    // move MIE to MPIE
    word_t b = ((st >> MIE) & 0x1) << MPIE;
    // turn off MPIE & MIE
    word_t off = st & (~((1 << MPIE) | (1 << MIE)));
    csr(CSR_MSTATUS) = b | off;
}
static inline bool is_intr_enabled() {
    word_t st = csr(CSR_MSTATUS);
    return (st >> MIE) & 0x1;
}

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

// #define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif
