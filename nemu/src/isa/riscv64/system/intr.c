#include <isa.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
    /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
    csr(CSR_MEPC) = epc;
    csr(CSR_MCAUSE) = NO;
    return csr(CSR_MTVEC);
}

word_t isa_query_intr() {
    return INTR_EMPTY;
}
