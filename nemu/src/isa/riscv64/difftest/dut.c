#include <cpu/difftest.h>
#include <isa.h>

#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state* ref_r, vaddr_t pc) {
    for (int i = 0; i < 32; i++) {
        word_t ref = ref_r->gpr[i]._64;
        word_t dut = cpu.gpr[i]._64;
        if (dut != ref) {
            Log("Reg %s not equal: pc = " FMT_WORD ", ref = " FMT_WORD ", dut = " FMT_WORD, reg_name(i, 8), pc, dut,
                ref);
            return false;
        }
    }
    return true;
}

void isa_difftest_attach() {}
