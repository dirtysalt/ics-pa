#include "local-include/reg.h"

#include <isa.h>

const char* regs[] = {"$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                      "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

void isa_reg_display() {
    int n = sizeof(regs) / sizeof(regs[0]);

    word_t value = cpu.pc;
    printf("%s  " FMT_WORD "\n", "pc", value);

    for (int i = 0; i < n; i++) {
        word_t value = gpr(i);
        printf("%s  " FMT_WORD "\n", regs[i], value);
    }
}

word_t isa_reg_str2val(const char* s, bool* success) {
    if (strcmp(s, "pc") == 0) {
        *success = true;
        return cpu.pc;
    }

    int n = sizeof(regs) / sizeof(regs[0]);
    *success = false;
    for (int i = 0; i < n; i++) {
        if (strcmp(regs[i], s) == 0) {
            *success = true;
            return gpr(i);
        }
    }
    return 0;
}
