#include <cpu/decode.h>

#include "../local-include/rtl.h"

#define INSTR_LIST(f) f(auipc) f(ld) f(sd) f(inv) f(nemu_trap) f(addi) f(jal) f(jalr) \
f(add) f(sub) f(sltiu) f(beq) f(bne) f(addiw) f(slli) f(lw) f(addw) f(sh) f(srai) \
f(lbu) f(sllw) f(andi) f(and) f(or) f(sltu) f(xori) f(sb) f(sraiw) f(sw) \
f(mul) f(div) f(mulw) f(divw) f(divuw) f(remw) f(remuw) f(srli) f(bge) f(bgeu) f(blt) f(bltu) \
f(slti) f(slt) f(lh) f(lhu) f(subw) f(srlw) f(sraw) f(slliw) f(srliw) f(lui)

def_all_EXEC_ID();