def_EHelper(auipc) {
    rtl_li(s, ddest, id_src1->imm + s->pc);
}

def_EHelper(addi) {
    rtl_addi(s, ddest, id_src1->preg, id_src2->imm);
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
}

def_EHelper(jalr) {
    rtl_li(s, s1, s->pc + 4);
    rtl_addi(s, s0, id_src1->preg, SEXT(id_src2->imm, 12));
    rtl_andi(s, s0, s0, ~(sword_t)1);
    rtl_jr(s, s0);
    rtl_mv(s, ddest, s1);
}

def_EHelper(add) {
    rtl_add(s, ddest, id_src1->preg, id_src2->preg);
}

def_EHelper(sub) {
    rtl_sub(s, ddest, id_src1->preg, id_src2->preg);
}

def_EHelper(sltiu) {
    word_t val = SEXT(id_src2->imm, 12);
    rtl_setrelopi(s, RELOP_LTU, s0, id_src1->preg, val);
    Log("pc = " FMT_WORD ", a = " FMT_WORD ", b = " FMT_WORD ", result = " FMT_WORD, s->pc, *id_src1->preg, val, *s0);
    rtl_mv(s, ddest, s0);
}

#define BRANCH_LOG(name, op)                                                                            \
    Log("pc = " FMT_WORD ", snpc = " FMT_WORD ", imm = " FMT_WORD ", val = " FMT_WORD ", a = " FMT_WORD \
        ", b = " FMT_WORD ", dnpc = " FMT_WORD,                                                         \
        s->pc, s->snpc, imm, ext, *id_src1->preg, *id_dest->preg, *s0);

#define BRANCH_TEMPLATE(name, op)                              \
    def_EHelper(name) {                                        \
        word_t imm = (id_src2->imm);                           \
        word_t val = BRANCH_SHUFFLE(imm);                      \
        word_t ext = SEXT(val << 1, 13);                       \
        rtl_setrelop(s, op, s0, id_src1->preg, id_dest->preg); \
        rtl_sub(s, s0, rz, s0); /* 0xfffff is 1*/              \
        rtl_andi(s, s1, s0, s->pc + ext);                      \
        rtl_not(s, s0, s0); /* 0x0 is 0 */                     \
        rtl_andi(s, s2, s0, s->snpc);                          \
        rtl_or(s, s0, s1, s2);                                 \
        BRANCH_LOG(name, op);                                  \
        rtl_jr(s, s0);                                         \
    }

BRANCH_TEMPLATE(beq, RELOP_EQ)
BRANCH_TEMPLATE(bne, RELOP_NE)
