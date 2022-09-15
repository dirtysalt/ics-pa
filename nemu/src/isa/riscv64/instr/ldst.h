def_EHelper(ld) {
    rtl_lm(s, ddest, dsrc1, id_src2->imm, 8);
}

def_EHelper(lw) {
    rtl_lms(s, ddest, dsrc1, id_src2->imm, 4);
}

def_EHelper(lh) {
    rtl_lms(s, ddest, dsrc1, id_src2->imm, 2);
}

def_EHelper(lb) {
    rtl_lms(s, ddest, dsrc1, id_src2->imm, 1);
}



def_EHelper(sd) {
    rtl_sm(s, ddest, dsrc1, id_src2->imm, 8);
}

def_EHelper(sh) {
    rtl_sm(s, ddest, dsrc1, id_src2->imm, 2);
}

def_EHelper(sb) {
    rtl_sm(s, ddest, dsrc1, id_src2->imm, 1);
}

def_EHelper(sw) {
    rtl_sm(s, ddest, dsrc1, id_src2->imm, 4);
}


def_EHelper(lbu) {
    rtl_lms(s, s0, dsrc1, id_src2->imm, 1);
    rtl_andi(s, ddest, s0, 0x0ff);
}

def_EHelper(lhu) {
    rtl_lms(s, s0, dsrc1, id_src2->imm, 2);
    rtl_andi(s, ddest, s0, 0x0ffff);
}
