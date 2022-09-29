#include <am.h>
#include <klib.h>
#include <riscv/riscv.h>

static Context* (*user_handler)(Event, Context*) = NULL;
void __am_switch(Context* c);
void __am_get_cur_as(Context* c);
#define IRQ_TIMER 0x8000000000000007 // for riscv64

Context* __am_irq_handle(Context* c) {
    // c->pdir maybe not intiialized.
    // but we have set initialize satp at `vme_init`
    // so right here we intialized it, and satp is assured to have kernel

    // for ctx has been initialized
    // since ctx->pdir == satp
    // so there is no side effect here.
    __am_get_cur_as(c);

    //  printf("__am_irq_handle. mcause = %p, mstatus = %p, mepc = %p, a7 = %p\n", c->mcause, c->mstatus, c->mepc, c->GPR1);
    if (user_handler) {
        Event ev = {0};
        // [0, 100) are syscall number.
        if (c->mcause >= 0 && c->mcause < 100) {
            ev.event = EVENT_SYSCALL;
            ev.cause = c->mcause;
        } else if (c->mcause == -1) {
            ev.event = EVENT_YIELD;
        } else if (c->mcause == IRQ_TIMER) {
            ev.event = EVENT_IRQ_TIMER;
        } else {
            ev.event = EVENT_ERROR;
        }
        c = user_handler(ev, c);
        assert(c != NULL);
    }
    __am_switch(c);
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

#define MPIE 7

Context* kcontext(Area kstack, void (*entry)(void*), void* arg) {
    char* buf = kstack.end - sizeof(Context);
    Context* ret = (Context*)buf;
    // return from __am_irq_handle
    ret->mepc = (uintptr_t)entry;
    // a0 as first argument.
    ret->GPR2 = (uintptr_t)arg;
    // TODO(yan): no need to allocate stack for kernel context
    // above this context, that's stack.
    // when mret, intr will be enabled.
    ret->mstatus = (1 << MPIE);
    return ret;
}

void yield() {
    asm volatile("li a7, -1; ecall");
}

bool ienabled() {
    return false;
}

void iset(bool enable) {}
