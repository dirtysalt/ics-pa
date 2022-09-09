#include <cpu/decode.h>

#include "../local-include/rtl.h"

#define INSTR_LIST(f) f(auipc) f(ld) f(sd) f(inv) f(nemu_trap) f(addi) f(jal) f(jalr) \
f(add) f(sub) f(sltiu) f(beq) f(bne) f(addiw) f(slli) f(lw) f(addw)

def_all_EXEC_ID();