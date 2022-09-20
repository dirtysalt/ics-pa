#include <am.h>
#include <klib.h>
#include <riscv/riscv.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context* c) {
    printf("__am_irq_handle. mcause = %p, mstatus = %p, mepc = %p, a7 = %p\n", c->mcause, c->mstatus, c->mepc, c->GPR1);
    if (user_handler) {
        Event ev = {0};
        // [0, 100) are syscall number.
        if (c->mcause >= 0 && c->mcause < 100) {
            ev.event = EVENT_SYSCALL;
            ev.cause = c->mcause;
        } else if (c->mcause == -1) {
            ev.event = EVENT_YIELD;
        } else {
            ev.event = EVENT_ERROR;
        }
        c = user_handler(ev, c);
        assert(c != NULL);
    }
    return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context* (*handler)(Event, Context*)) {
    // initialize exception entry
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

    // register event handler
    user_handler = handler;

    return true;
}

Context* kcontext(Area kstack, void (*entry)(void*), void* arg) {
    return NULL;
}

void yield() {
    asm volatile("li a7, -1; ecall");
}

bool ienabled() {
    return false;
}

void iset(bool enable) {}
