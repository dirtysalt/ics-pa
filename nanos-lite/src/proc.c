#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB* current = NULL;

void naive_uload(PCB* pcb, const char* filename);
void context_uload(PCB* pcb, const char* filename, char* const argv[], char* const envp[], bool reuse_stack);

Context* kcontext(Area kstack, void (*entry)(void*), void* arg);

void context_kload(PCB* pcb, void (*entry)(void*), void* arg) {
    Area stack = {.start = pcb->stack, .end = pcb->stack + sizeof(pcb->stack)};
    Context* ctx = kcontext(stack, entry, arg);
    pcb->cp = ctx;
}

void switch_boot_pcb() {
    current = &pcb_boot;
}

void hello_fun(void* arg) {
    int j = 1;
    while (1) {
        Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
        j++;
        yield();
    }
}

static int arg0, arg1;
char* const argv_hello[] = {"[FUCK]", NULL};

void init_proc() {
    Log("arg0 = %p, arg1 = %p", &arg0, &arg1);
    context_kload(&pcb[0], hello_fun, &arg0);
    // context_kload(&pcb[1], hello_fun, &arg1);
    // context_uload(&pcb[1], "/bin/hello", argv_hello, NULL, false);
    // context_uload(&pcb[1], "/bin/exec-test", NULL, NULL, false);

    context_kload(&pcb[1], hello_fun, &arg1);
    context_uload(&pcb[2], "/bin/hello", argv_hello, NULL, false);
    context_uload(&pcb[3], "/bin/exec-test", NULL, NULL, false);

    switch_boot_pcb();

    Log("Initializing processes...");

    // load program here
    // naive_uload(NULL, NULL);
    // naive_uload(NULL, "/bin/dummy");
    // naive_uload(NULL, "/bin/hello");
    // naive_uload(NULL, "/bin/file-test");
    // naive_uload(NULL, "/bin/timer-test");
    // naive_uload(NULL, "/bin/event-test");
    // naive_uload(NULL, "/bin/bmp-test");
}

Context* schedule(Context* prev) {
    current->cp = prev;
    // always switch to pcb[0]
    if (current == &pcb_boot) {
        current = &pcb[0];
    } else {
        for (int i = 0; i < MAX_NR_PROC; i++) {
            if (current == &pcb[i]) {
                current = &pcb[(i + 1) % MAX_NR_PROC];
                break;
            }
        }
    }
    // current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
    return current->cp;
}
