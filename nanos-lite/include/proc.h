#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

#define STACK_SIZE (8 * PGSIZE)

typedef union {
    uint8_t stack[STACK_SIZE] PG_ALIGN;
    struct {
        Context* cp;
        // TODO(yan): to record user space stack end.
        // so we can reuse user space stack.
        void* user_stack_end;
        AddrSpace as;
        // we do not free memory, so use `max_brk' to determine when to call _map()
        uintptr_t max_brk;
    };
} PCB;

extern PCB* current;

#endif
