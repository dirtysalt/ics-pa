#include <utils.h>

def_EHelper(auipc) {
    rtl_li(s, ddest, id_src1->imm + s->pc);
}

// offset[20|10:1|11|19:12]
// offset[19|9:0|10|18:11]
#define JAL_SHUFFLE(imm) \
    (((imm >> 19) & 0x1) << 19) | ((imm & 0xff) << 11) | (((imm >> 8) & 0x1) << 10) | ((imm >> 9) & 0x3ff)

// offset[12|10:5|4:1|11]
// offset[11|9:4|3:0|10]
#define BRANCH_SHUFFLE(imm) \
    (((imm >> 11) & 0x1) << 11) | ((imm & 0x1) << 10) | (((imm >> 5) & 0x3f) << 4) | ((imm >> 1) & 0x0f)

def_EHelper(jal) {
    rtl_li(s, ddest, s->pc + 4);

    // TODO(yan): can we manipulate imm right here?
    // and can we take s->pc + ext as primitive operation?
    word_t imm = (id_src1->imm >> 12);
    word_t val = JAL_SHUFFLE(imm);
    word_t ext = SEXT(val << 1, 21);
    // printf("val = %ld, ext = %ld\n", val << 1, result);

    rtl_j(s, s->pc + ext);

    // Log("jal. pc = " FMT_WORD ", next = " FMT_WORD, s->pc, s->dnpc);

#ifdef CONFIG_FTRACE
    FuncTraceEvent* event = new_ftrace_event();
    event->type = FTRACE_JMP;
    event->now_pc = s->pc;
    event->jmp_pc = s->dnpc;
#endif
}

def_EHelper(jalr) {
    rtl_li(s, s1, s->pc + 4);
    rtl_addi(s, s0, id_src1->preg, id_src2->imm);
    rtl_andi(s, s0, s0, ~(sword_t)1);
    rtl_jr(s, s0);
    rtl_mv(s, ddest, s1);

    // Log("jalr. pc = " FMT_WORD ", next = " FMT_WORD, s->pc, s->dnpc);

#ifdef CONFIG_FTRACE
    FuncTraceEvent* event = new_ftrace_event();
    event->type = FTRACE_JMP;
    event->now_pc = s->pc;
    event->jmp_pc = s->dnpc;
#endif
}

def_EHelper(lui) {
    rtl_li(s, ddest, id_src1->imm);
}

// branchless version. all operations are in RTL.
#if 0
#define BRANCH_LOG(name, op)                                                                            \
    Log("pc = " FMT_WORD ", snpc = " FMT_WORD ", imm = " FMT_WORD ", val = " FMT_WORD ", a = " FMT_WORD \
        ", b = " FMT_WORD ", dnpc = " FMT_WORD,                                                         \
        s->pc, s->snpc, imm, ext, *id_src1->preg, *id_dest->preg, *s0);

#define BRANCH_IMM_TEMPLATE(name, op)                          \
    def_EHelper(name) {                                        \
        word_t imm = (id_src2->imm);                           \
        word_t val = BRANCH_SHUFFLE(imm);                      \
        word_t ext = SEXT(val << 1, 13);                       \
        rtl_setrelop(s, op, s0, id_src1->preg, id_dest->preg); \
        rtl_sub(s, s0, rz, s0); /* 0xfffff is 1 */             \
        rtl_andi(s, s1, s0, s->pc + ext);                      \
        rtl_not(s, s0, s0); /* 0x0 is 0 */                     \
        rtl_andi(s, s2, s0, s->snpc);                          \
        rtl_or(s, s0, s1, s2);                                 \
        BRANCH_LOG(name, op);                                  \
        rtl_jr(s, s0);                                         \
    }
#else

#define BRANCH_IMM_TEMPLATE(name, op)                          \
    def_EHelper(name) {                                        \
        word_t imm = (id_src2->imm);                           \
        word_t val = BRANCH_SHUFFLE(imm);                      \
        word_t ext = SEXT(val << 1, 13);                       \
        rtl_setrelop(s, op, s0, id_src1->preg, id_dest->preg); \
        if (*s0 == 1) rtl_j(s, s->pc + ext);                   \
    }
#endif

BRANCH_IMM_TEMPLATE(beq, RELOP_EQ)
BRANCH_IMM_TEMPLATE(bne, RELOP_NE)
BRANCH_IMM_TEMPLATE(bge, RELOP_GE)
BRANCH_IMM_TEMPLATE(bgeu, RELOP_GEU)
BRANCH_IMM_TEMPLATE(bltu, RELOP_LTU)
BRANCH_IMM_TEMPLATE(blt, RELOP_LT)

def_EHelper(sltiu) {
    rtl_setrelopi(s, RELOP_LTU, ddest, id_src1->preg, id_src2->imm);
}
def_EHelper(slti) {
    rtl_setrelopi(s, RELOP_LT, ddest, id_src1->preg, id_src2->imm);
}
def_EHelper(sltu) {
    rtl_setrelop(s, RELOP_LTU, ddest, id_src1->preg, id_src2->preg);
}
def_EHelper(slt) {
    rtl_setrelop(s, RELOP_LT, ddest, id_src1->preg, id_src2->preg);
}

#define DIRECT(name, ...) \
    def_EHelper(name) { rtl_##name(s, __VA_ARGS__); }

DIRECT(and, ddest, id_src1->preg, id_src2->preg)
DIRECT(or, ddest, id_src1->preg, id_src2->preg)
DIRECT(xor, ddest, id_src1->preg, id_src2->preg)
DIRECT(add, ddest, id_src1->preg, id_src2->preg)
DIRECT(addw, ddest, id_src1->preg, id_src2->preg)
DIRECT(subw, ddest, id_src1->preg, id_src2->preg)
DIRECT(sllw, ddest, id_src1->preg, id_src2->preg)
DIRECT(srlw, ddest, id_src1->preg, id_src2->preg)
DIRECT(sraw, ddest, id_src1->preg, id_src2->preg)
DIRECT(sub, ddest, id_src1->preg, id_src2->preg)
DIRECT(addi, ddest, id_src1->preg, id_src2->imm)
DIRECT(slli, ddest, id_src1->preg, id_src2->imm & 0x3f)
DIRECT(srai, ddest, id_src1->preg, id_src2->imm & 0x3f)
DIRECT(srli, ddest, id_src1->preg, id_src2->imm & 0x3f)
DIRECT(xori, ddest, id_src1->preg, id_src2->imm)
DIRECT(andi, ddest, id_src1->preg, id_src2->imm)
DIRECT(ori, ddest, id_src1->preg, id_src2->imm)
DIRECT(addiw, ddest, id_src1->preg, id_src2->imm)
DIRECT(sraiw, ddest, dsrc1, id_src2->imm)
DIRECT(slliw, ddest, dsrc1, id_src2->imm)
DIRECT(srliw, ddest, dsrc1, id_src2->imm)
DIRECT(mulw, ddest, dsrc1, dsrc2)
DIRECT(divw, ddest, dsrc1, dsrc2)
DIRECT(divuw, ddest, dsrc1, dsrc2)
DIRECT(remw, ddest, dsrc1, dsrc2)
DIRECT(remuw, ddest, dsrc1, dsrc2)
DIRECT(sll, ddest, dsrc1, dsrc2)

def_EHelper(mul) {
    sword_t a = (sword_t)(*dsrc1);
    sword_t b = (sword_t)(*dsrc2);
    *ddest = (word_t)(a * b);
}

def_EHelper(rem) {
    sword_t a = (sword_t)(*dsrc1);
    sword_t b = (sword_t)(*dsrc2);
    *ddest = (word_t)(a % b);
}

def_EHelper(remu) {
    word_t a = (word_t)(*dsrc1);
    word_t b = (word_t)(*dsrc2);
    *ddest = (word_t)(a % b);
}

def_EHelper(div) {
    rtl_divs_q(s, ddest, dsrc1, dsrc2);
}
def_EHelper(divu) {
    rtl_divu_q(s, ddest, dsrc1, dsrc2);
}

// =========== csr instructions ==========

def_EHelper(csrrs) {
    //    Log("csrrs index = 0x%lx", id_src2->imm);
    word_t t = csr(id_src2->imm);
    csr(id_src2->imm) = t | (*dsrc1);
    *ddest = t;
}

def_EHelper(csrrw) {
    //    Log("csrrw index = 0x%lx, value = " FMT_WORD, id_src2->imm, *dsrc1);
    word_t t = csr(id_src2->imm);
    csr(id_src2->imm) = (*dsrc1);
    *ddest = t;
}

def_EHelper(ecall) {
    word_t no = gpr(17);
    word_t epc = s->snpc;
    //    Log("ecall pc = " FMT_WORD ", no = " FMT_WORD ", epc = " FMT_WORD, s->pc, no, epc);
    // when syscall, intr will be disabled.
    word_t dnpc = isa_raise_intr(no, epc);
    s->dnpc = dnpc;
}

def_EHelper(mret) {
    word_t epc = csr(CSR_MEPC);
    //    Log("mret pc = " FMT_WORD ", epc = " FMT_WORD, s->pc, epc);
    s->dnpc = epc;
    // when ret, mpie has been set, so intr will be enabled.
    enable_intr();
}