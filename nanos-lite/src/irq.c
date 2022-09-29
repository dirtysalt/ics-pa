#include <am.h>
#include <common.h>
#include <fs.h>
#include <klib-macros.h>
#include <klib.h>
#include <proc.h>
#include <sys/time.h>

void halt(int code);
void switch_boot_pcb();
Context* schedule(Context* c);

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

// in linker.ld, program break.
extern char _brk_begin;
extern char _brk_end;
char* program_break = &_brk_begin;
uintptr_t program_end = (uintptr_t)&_brk_end;
#define PGB (program_break)

void context_uload(PCB* pcb, const char* filename, char* const argv[], char* const envp[], bool reuse_stack);
extern int mm_brk(uintptr_t brk);

static Context* handle_syscall(Event* e, Context* c) {
    if (e->cause == SYS_exit) {
        Log("syscall exit. code = %d", c->GPR2);
        halt(c->GPR2);
    } else if (e->cause == SYS_write) {
        int fd = c->GPR2;
        char* buf = (char*)c->GPR3;
        size_t count = c->GPR4;
        // Log("syscall write. fd = %d, buf = %p, count = %p", fd, buf, count);
        size_t ret = fs_write(fd, buf, count);
        c->GPRx = ret;

    } else if (e->cause == SYS_close) {
        int fd = c->GPR2;
        // Log("syscall close. fd = %d", fd);
        c->GPRx = fs_close(fd);

    } else if (e->cause == SYS_brk) {
        size_t inc = c->GPR2;

        // // TODO(yan): brk from program break to heap start.
        // uintptr_t ret = (uintptr_t)PGB;
        // Log("syscall sbrk. inc = %p, ret = %p", inc, ret);
        // if ((ret + inc) > program_end) {
        //     Log("syscall sbrk failed. about to enter heap zone");
        //     c->GPRx = -1;
        // } else {
        //     PGB += inc;
        //     c->GPRx = ret;
        // }

        // TODO(yan): now brk can grow beyond program break.
        uintptr_t brk = current->max_brk;
        if (mm_brk(brk + inc) == 0) {
            c->GPRx = brk;
        } else {
            c->GPRx = -1;
        }

    } else if (e->cause == SYS_open) {
        const char* path = (const char*)c->GPR2;
        Log("syscall open. path = %s", path);
        int flags = c->GPR3;
        int mode = c->GPR4;
        int fd = fs_open(path, flags, mode);
        c->GPRx = fd;

    } else if (e->cause == SYS_read) {
        int fd = c->GPR2;
        char* buf = (char*)c->GPR3;
        size_t count = c->GPR4;
        // Log("syscall read. fd = %d, buf = %p, count = %p", fd, buf, count);
        size_t ret = fs_read(fd, buf, count);
        c->GPRx = ret;

    } else if (e->cause == SYS_lseek) {
        int fd = c->GPR2;
        size_t offset = c->GPR3;
        int whence = c->GPR4;
        // Log("syscall seek. fd = %d, offset = %p, whence = %p", fd, offset, whence);
        size_t ret = fs_lseek(fd, offset, whence);
        c->GPRx = ret;

    } else if (e->cause == SYS_gettimeofday) {
        // Log("syscall gettimeofday");
        // ideally it should call AM ioe_read
        struct timeval* tv = (struct timeval*)(c->GPR2);
        // struct timezone* tz = (struct timezone*)(c->GPR3);
        uint64_t us = io_read(AM_TIMER_UPTIME).us;
        tv->tv_sec = us / 1000000;
        tv->tv_usec = us % 1000000;
        c->GPRx = 0;

    } else if (e->cause == SYS_execve) {
        const char* filename = (const char*)c->GPR2;
        char* const* argv = (char* const*)c->GPR3;
        char* const* envp = (char* const*)c->GPR4;
        // overwrite current pcb.
        // PCB* now = current;
        // because we already use filename_as_arg0, so we ignore first argument.
        context_uload(current, filename, argv + 1, envp, false);
        switch_boot_pcb();
        c = schedule(c);
    }
    return c;
}

static Context* do_event(Event e, Context* c) {
    switch (e.event) {
    case EVENT_YIELD: {
        // Log("event yield");
        c = schedule(c);
        break;
    }
    case EVENT_SYSCALL: {
        // Log("event syscall. number = %p", e.cause);
        c = handle_syscall(&e, c);
        break;
    }
    case EVENT_IRQ_TIMER: {
        Log("event irq timer. ready to schedule");
        c = schedule(c);
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
