#ifndef __RTL_PSEUDO_H__
#define __RTL_PSEUDO_H__

#ifndef __RTL_RTL_H__
#error "Should be only included by <rtl/rtl.h>"
#endif

/* RTL pseudo instructions */

static inline def_rtl(li, rtlreg_t* dest, const rtlreg_t imm) {
    rtl_addi(s, dest, rz, imm);
}

static inline def_rtl(mv, rtlreg_t* dest, const rtlreg_t* src1) {
    rtl_addi(s, dest, src1, 0);
}

static inline def_rtl(not, rtlreg_t* dest, const rtlreg_t* src1) {
    rtl_xori(s, ddest, src1, ~(word_t)1);
}

static inline def_rtl(neg, rtlreg_t* dest, const rtlreg_t* src1) {
    rtl_sub(s, ddest, rz, src1);
}

static inline def_rtl(sext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
    // dest <- signext(src1[(width * 8 - 1) .. 0])
    rtl_zext(s, ddest, src1, width);
    // make it sign extended.
    rtl_srai(s, ddest, ddest, 0);
}

static inline def_rtl(zext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
    // dest <- zeroext(src1[(width * 8 - 1) .. 0])
    // make mask (width << 3) - 1
    rtl_li(s, ddest, imm);
    rtl_slli(s, ddest, ddest, 3);
    rtl_subi(s, ddest, ddest, 1);
    // and.
    rtl_and(s, ddest, src1, ddest);
}

static inline def_rtl(msb, rtlreg_t* dest, const rtlreg_t* src1, int width) {
    // dest <- src1[width * 8 - 1]
    rtl_zext(s, dest, src1, width);
}
#endif
