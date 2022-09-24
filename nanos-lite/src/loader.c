#include <am.h>
#include <elf.h>
#include <fs.h>
#include <proc.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#define ELF_Addr Elf64_Addr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#define ELF_Addr Elf32_Addr
#endif

size_t ramdisk_read(void* buf, size_t offset, size_t len);
static void load_segment(int fd, Elf_Phdr* header);
static void load_segment_null(Elf_Phdr* header);

static uintptr_t loader_null(PCB* pcb, const char* filename) {
    // TODO();
    Elf_Ehdr elf_header;
    ramdisk_read(&elf_header, 0, sizeof(elf_header));

    //assert(*(uint32_t*)(elf_header.e_ident) == 0x7f454c46);
    assert(*(uint32_t*)(elf_header.e_ident) == 0x464c457f);

    size_t offset = elf_header.e_phoff;
    for (int i = 0; i < elf_header.e_phnum; i++) {
        Elf_Phdr ph;
        assert(sizeof(ph) == elf_header.e_phentsize);
        ramdisk_read(&ph, offset, sizeof(ph));
        // todo
        if (ph.p_type == PT_LOAD) {
            load_segment_null(&ph);
        }
        offset += elf_header.e_phentsize;
    }

    return elf_header.e_entry;
}

static uintptr_t loader(PCB* pcb, const char* filename) {
    Log("loader with filename: %s", filename);
    if (filename == NULL) {
        return loader_null(pcb, filename);
    }

    int fd = fs_open(filename, 0, 0);
    Elf_Ehdr elf_header;
    fs_read(fd, &elf_header, sizeof(elf_header));

    //assert(*(uint32_t*)(elf_header.e_ident) == 0x7f454c46);
    assert(*(uint32_t*)(elf_header.e_ident) == 0x464c457f);

    size_t offset = elf_header.e_phoff;
    for (int i = 0; i < elf_header.e_phnum; i++) {
        Elf_Phdr ph;
        assert(sizeof(ph) == elf_header.e_phentsize);
        fs_lseek(fd, offset, SEEK_SET);
        fs_read(fd, &ph, sizeof(ph));
        // todo
        if (ph.p_type == PT_LOAD) {
            load_segment(fd, &ph);
        }
        offset += elf_header.e_phentsize;
    }

    return elf_header.e_entry;
}

static void load_segment_null(Elf_Phdr* header) {
    size_t offset = header->p_offset;
    char* vaddr = (char*)header->p_vaddr;
    uint64_t file_size = header->p_filesz;
    uint64_t mem_size = header->p_memsz;
    Log("load segment. offset = %p, vaddr = %p, file_size = %p, mem_size = %p", offset, vaddr, file_size, mem_size);
    memset(vaddr, 0, mem_size);
    ramdisk_read(vaddr, offset, file_size);
}

static void load_segment(int fd, Elf_Phdr* header) {
    size_t offset = header->p_offset;
    char* vaddr = (char*)header->p_vaddr;
    uint64_t file_size = header->p_filesz;
    uint64_t mem_size = header->p_memsz;
    Log("load segment. offset = %p, vaddr = %p, file_size = %p, mem_size = %p", offset, vaddr, file_size, mem_size);
    memset(vaddr, 0, mem_size);
    fs_lseek(fd, offset, SEEK_SET);
    fs_read(fd, vaddr, file_size);
}

void naive_uload(PCB* pcb, const char* filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void (*)())entry)();
}

Context* ucontext(AddrSpace* as, Area kstack, void* entry);

// void context_uload(PCB* pcb, const char* filename) {
//     Area kstack = {.start = pcb->stack, .end = pcb->stack + sizeof(pcb->stack)};
//     uintptr_t entry = loader(pcb, filename);
//     Context* ctx = ucontext(NULL, kstack, (void*)entry);
//     // a0. looks at riscv64.S, move sp, a0 to initialize stack pointer.
//     ctx->GPRx = (uintptr_t)heap.end;
//     pcb->cp = ctx;
// }

void context_uload(PCB* pcb, const char* filename, char* const argv[], char* const envp[], bool reuse_stack) {
    Area kstack = {.start = pcb->stack, .end = pcb->stack + sizeof(pcb->stack)};
    uintptr_t entry = loader(pcb, filename);
    Context* ctx = ucontext(NULL, kstack, (void*)entry);

    // place them onto stack from stack top.
    uintptr_t argc = 0;
    int string_size = 0;

    bool filename_as_arg0 = true;

    if (filename_as_arg0) {
        // filename as the first argument.
        string_size += strlen(filename) + 1;
    }

    if (argv) {
        while (argv[argc]) {
            string_size += strlen(argv[argc]) + 1;
            argc++;
        }
    }

    int envc = 0;
    if (envp) {
        while (envp[envc]) {
            string_size += strlen(envp[envc]) + 1;
            envc++;
        }
    }

    // argc(int) (argc+1)(pointer)  (envc+1)(pointer) strings.

    if (filename_as_arg0) {
        argc += 1; // filename.
    }

    int arg_size = (1 + argc + 1 + envc + 1) * sizeof(uintptr_t);
    // char* stack = (char*)(heap.end - arg_size - string_size);
    char* stack_end = NULL;
    if (!reuse_stack) {
        char* stack_begin = (char*)new_page(4);
        stack_end = stack_begin + 4 * PGSIZE;
        pcb->user_stack_end = stack_end;
    } else {
        stack_end = pcb->user_stack_end;
    }
    char* stack = stack_end - arg_size - string_size;

    Log("uload. argc = %d, arg_size = %d, string_size = %d, stack = %p", (int)argc, arg_size, string_size, stack);

    // copy argv
    char* p = stack;
    char* p2 = p + arg_size;

    // make it uintptr_t size.
    memcpy(p, &argc, sizeof(uintptr_t));
    p += sizeof(uintptr_t);

    if (filename_as_arg0) {
        *(char**)p = p2;
        strcpy(p2, filename);
        p2 += strlen(filename) + 1;
        p += sizeof(char*);
    }

    if (argv) {
        for (int i = 0; argv[i]; i++) {
            *(char**)p = p2;
            strcpy(p2, argv[i]);
            p2 += strlen(argv[i]) + 1;
            p += sizeof(char*);
        }
    }
    *(char**)p = 0;
    p += sizeof(char*);

    // copy envp
    if (envp) {
        for (int i = 0; envp[i]; i++) {
            *(char**)p = p2;
            strcpy(p2, envp[i]);
            p2 += strlen(envp[i]) + 1;
            p += sizeof(char*);
        }
    }
    *(char**)p = 0;
    p += sizeof(char*);

    // a0. looks at riscv64.S, move sp, a0 to initialize stack pointer.
    ctx->GPRx = (uintptr_t)stack;

    {
        uintptr_t* d = (uintptr_t*)stack;
        for (int i = 0; i < (argc + 2 + envc + 1); i++) {
            Log("stack[%d] p = %p, data  = %p", i, d + i, d[i]);
        }
    }
    pcb->cp = ctx;
}
