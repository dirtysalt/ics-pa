#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T* kbd) {
    uint32_t code = inl(KBD_ADDR);
    if (code == 0) {
        kbd->keydown = 0;
        kbd->keycode = AM_KEY_NONE;
    } else {
        kdb->keydown = 1;
        kdb->keycode = code;
    }
}
