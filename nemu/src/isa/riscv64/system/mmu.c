#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

#define PREAD(x) paddr_read((paddr_t)(uintptr_t)(x), 8)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
    {
        uintptr_t x = (uintptr_t)vaddr;
        int a = x / PAGE_SIZE;
        int b = (x + len - 1) / PAGE_SIZE;
        if (a != b) {
            panic("mmu translate cross page");
        }
    }
    uint64_t satp = csr(CSR_SATP);

    // sv39
    //  9   9   9   12
    // L1  L2  L3
    uint64_t* pte = (uint64_t*)((satp & 0x0fffffffffff) << 12);
    // if ((uintptr_t)pte != 0x800e1000) {
    //     panic("pte check failed");
    // }
    uint64_t p = (uint64_t)(vaddr);
    int a0 = (p >> 30) & 0x1ff;
    int a1 = (p >> 21) & 0x1ff;
    int a2 = (p >> 12) & 0x1ff;

    bool trace = false;
    // if ((a0 == 2) && (a1 == 24) && (a2 == 6)) {
    //     trace = true;
    // }
    // L1.
    {
        uint64_t x = PREAD(pte + a0);
        if (trace) Log("L1 PTE = " FMT_WORD ", x = " FMT_WORD, (uintptr_t)pte, x);
        if ((x & 0x0fff) != 0x337) {
            Log("L1 page table entry is invalid. vaddr = " FMT_WORD, vaddr);
            panic("L1 page table entry is invalid");
        }
        pte = (uint64_t*)((x >> 12) << 12);
    }
    // L2.
    {
        uint64_t x = PREAD(pte + a1);
        if (trace) Log("L1 PTE = " FMT_WORD ", x = " FMT_WORD, (uintptr_t)pte, x);
        if ((x & 0x0fff) != 0x337) {
            Log("L2 page table entry is invalid. vaddr = " FMT_WORD, vaddr);
            panic("L2 page table entry is invalid");
        }
        pte = (uint64_t*)((x >> 12) << 12);
    }
    // L3.
    // paddr_t pa = vaddr & 0x0fff;
    paddr_t pa = 0;
    {
        uint64_t x = PREAD(pte + a2);
        if (trace) Log("L1 PTE = " FMT_WORD ", x = " FMT_WORD, (uintptr_t)pte, x);
        if ((x & 0x0fff) != 0x337) {
            Log("L3 page table entry is invalid. vaddr = " FMT_WORD, vaddr);
            panic("L3. page table entry is invalid");
        }
        pa = ((x >> 12) << 12);
        pa |= vaddr & 0x0fff;
    }


    // TODO(yan): this assumption holds when there is kernel space.    
    // if (pa != vaddr) {
    //     Log("something wrong with page walk. pa = " FMT_PADDR ", vaddr = " FMT_WORD ". a0 = %d, a1 = %d, a2 = %d", pa,
    //         vaddr, a0, a1, a2);
    //     panic("something wrong with page walk");
    //     // } else {
    //     //   Log("Great");
    // }
    return pa;
}

int isa_mmu_check(vaddr_t vaddr, int len, int type) {
    uint64_t satp = csr(CSR_SATP);
    int t = (satp >> 63) & 0x1;
    if (t) {
        return MMU_TRANSLATE;
    }
    return MMU_DIRECT;
}
