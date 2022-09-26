#include <memory.h>
#include <proc.h>

static void* pf = NULL;

void* new_page(size_t nr_page) {
    char* now = (char*)pf;
    size_t size = PGSIZE * nr_page;
    pf = now + size;
    return now;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
    size_t nr_page = (n + PGSIZE - 1) / PGSIZE;
    char* buf = new_page(nr_page);
    memset(buf, 0x0, n);
    return buf;
}
#endif

void free_page(void* p) {
    panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
    // page aligned.
    uintptr_t max_brk = current->max_brk;
    while (max_brk < brk) {
        void* pa = new_page(1);
        map(&current->as, (void*)max_brk, pa, 0);
        max_brk += PGSIZE;
    }
    current->max_brk = max_brk;
    return 0;
}

void init_mm() {
    pf = (void*)ROUNDUP(heap.start, PGSIZE);
    Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
    vme_init(pg_alloc, free_page);
#endif
}
