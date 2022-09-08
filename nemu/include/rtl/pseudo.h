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
    rtl_li(s, dest, ~(*src1));
}

static inline def_rtl(neg, rtlreg_t* dest, const rtlreg_t* src1) {
    rtl_li(s, dest, -(*src1));
}

static inline def_rtl(sext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
    // dest <- signext(src1[(width * 8 - 1) .. 0])
    if ((width * 8) == sizeof(*src1)) {
        rtl_li(s, dest, *src1);
    } else {
        rtl_li(s, dest, SIGNEX(*src1, width * 8));
    }
}

static inline def_rtl(zext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
    // dest <- zeroext(src1[(width * 8 - 1) .. 0])
    if ((width * 8) == sizeof(*src1)) {
        rtl_li(s, dest, *src1);
    } else {
        word_t mask = ((uint64_t)1 << (8 * width)) - 1;
        rtl_li(s, dest, *src1 & mask);
    }
}

static inline def_rtl(msb, rtlreg_t* dest, const rtlreg_t* src1, int width) {
    // dest <- src1[width * 8 - 1]
    rtl_zext(s, dest, src1, width);
}
#endif
