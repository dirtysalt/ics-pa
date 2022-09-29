#include <am.h>
#include <klib.h>
#include <nemu.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = { // Kernel memory mappings
        NEMU_PADDR_SPACE};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void* pdir) {
    uintptr_t mode = 1ul << (__riscv_xlen - 1);
    asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
    uintptr_t satp;
    asm volatile("csrr %0, satp" : "=r"(satp));
    return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
    pgalloc_usr = pgalloc_f;
    pgfree_usr = pgfree_f;

    kas.ptr = pgalloc_f(PGSIZE);

    // work on physical pages.
    int i;
    for (i = 0; i < LENGTH(segments); i++) {
        void* va = segments[i].start;
        for (; va < segments[i].end; va += PGSIZE) {
            map(&kas, va, va, 0);
        }
    }
    // printf("vme_init. pte = %p\n", kas.ptr);
    set_satp(kas.ptr);
    vme_enable = 1;

    return true;
}

void protect(AddrSpace* as) {
    PTE* updir = (PTE*)(pgalloc_usr(PGSIZE));
    as->ptr = updir;
    as->area = USER_SPACE;
    as->pgsize = PGSIZE;
    // TODO(yan): 这样就可以访问kernel space了。
    // map kernel space
    memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace* as) {}

void __am_get_cur_as(Context* c) {
    c->pdir = (vme_enable ? (void*)get_satp() : NULL);
}

void __am_switch(Context* c) {
    if (vme_enable && c->pdir != NULL) {
        set_satp(c->pdir);
    }
}

void map(AddrSpace* as, void* va, void* pa, int prot) {
    // allocate page
    // sv39
    // 9   9    9    12
    // L1  L2   L3
    uint64_t p = (uint64_t)(va);
    if (p % PGSIZE != 0) {
        panic("virtual address not page aligned");
    }
    int a0 = (p >> 30) & 0x1ff;
    int a1 = (p >> 21) & 0x1ff;
    int a2 = (p >> 12) & 0x1ff;
    // printf("map va %p -> pa %p a0 = %d, a1 = %d, a2 = %d\n", va, pa, a0, a1, a2);
    bool trace = false;
    // if ((a0 == 2) && (a1 == 24) && (a2 == 6)) {
    //     trace = true;
    // }
    uint64_t* pte = (uint64_t*)as->ptr;
    if (trace) printf("L1 PTE = %p\n", (uintptr_t)pte);
    // L1.
    {
        uint64_t* x = pte + a0;
        if (!(*x & 0x1)) {
            // allocate new page.
            uintptr_t pp = (uintptr_t)pgalloc_usr(PGSIZE);
            *x = ((pp >> 12) << 12) | 0x337;
        }
        pte = (uint64_t*)(((*x) >> 12) << 12);
    }
    if (trace) printf("L2 PTE = %p\n", (uintptr_t)pte);
    // L2.
    {
        uint64_t* x = pte + a1;
        if (!(*x & 0x1)) {
            // allocate new page.
            uintptr_t pp = (uintptr_t)pgalloc_usr(PGSIZE);
            *x = ((pp >> 12) << 12) | 0x337;
        }
        pte = (uint64_t*)(((*x) >> 12) << 12);
    }
    if (trace) printf("L3 PTE = %p\n", (uintptr_t)pte);
    // L3.
    {
        uint64_t* x = pte + a2;
        uintptr_t v = (uintptr_t)pa;
        *x = ((v >> 12) << 12) | 0x337;
    }
}

#define MPIE 7
Context* ucontext(AddrSpace* as, Area kstack, void* entry) {
    char* buf = kstack.end - sizeof(Context);
    Context* ret = (Context*)buf;
    // return from __am_irq_handle
    ret->mepc = (uintptr_t)entry;
    // enable mpie, so when mret, intr will be enabled.
    ret->mstatus = (1 << MPIE);
    if (as != NULL) {
        ret->pdir = as->ptr;
    }
    return ret;
}
