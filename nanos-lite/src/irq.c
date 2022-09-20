#include <common.h>

void halt(int code);

enum {
    SYS_exit,
    SYS_yield,
    SYS_open,
    SYS_read,
    SYS_write,
    SYS_kill,
    SYS_getpid,
    SYS_close,
    SYS_lseek,
    SYS_brk,
    SYS_fstat,
    SYS_time,
    SYS_signal,
    SYS_execve,
    SYS_fork,
    SYS_link,
    SYS_unlink,
    SYS_wait,
    SYS_times,
    SYS_gettimeofday
};

static void handle_syscall(Event* e, Context* c) {
    if (e->cause == SYS_exit) {
        halt(c->GPR2);
    }
}

static Context* do_event(Event e, Context* c) {
    switch (e.event) {
    case EVENT_YIELD: {
        Log("event yield");
        break;
    }
    case EVENT_SYSCALL: {
        Log("event syscall. number = %p", e.cause);
        handle_syscall(&e, c);
        break;
    }
    default:
        panic("Unhandled event ID = %d", e.event);
    }
    return c;
}

void init_irq(void) {
    Log("Initializing interrupt/exception handler...");
    cte_init(do_event);
}
