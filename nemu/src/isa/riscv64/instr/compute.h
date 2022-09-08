def_EHelper(auipc) {
    rtl_li(s, ddest, id_src1->imm + s->pc);
}

def_EHelper(addi) {
    rtl_addi(s, ddest, id_src1->preg, id_src2->imm);
}

def_EHelper(jal) {
    rtl_li(s, ddest, s->pc + 4);
    word_t imm = (id_src1->imm >> 12);
    word_t val = (((imm >> 19) & 0x1) << 19) | ((imm & 0xff) << 11) | (((imm >> 8) & 0x1) << 10) | ((imm >> 9) & 0x3ff);
    word_t ext = SEXT(val << 1, 21);
    // printf("val = %ld, ext = %ld\n", val << 1, result);
    rtl_j(s, s->pc + ext);
}

def_EHelper(jalr) {
    rtl_li(s, t0, s->pc + 4);
    // TODO(yan):
    rtl_addi(s, s0, id_src1->preg, SEXT(id_src2->imm, 12));
    rtl_jr(s, t0);
}