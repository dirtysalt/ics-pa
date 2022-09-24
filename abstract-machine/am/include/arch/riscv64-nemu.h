#ifndef ARCH_H__
#define ARCH_H__

struct Context {
    // TODO: fix the order of these members to match trap.S
    uintptr_t gpr[32], mcause, mstatus, mepc;
    // TODO(yan): I don't know what's that for? what's dir?
    // page directory?
    void* pdir;
};

#define GPR1 gpr[17] // a7
// a0 a1 a2
#define GPR2 gpr[10]
#define GPR3 gpr[11]
#define GPR4 gpr[12]
// a0
#define GPRx gpr[10]

#endif
