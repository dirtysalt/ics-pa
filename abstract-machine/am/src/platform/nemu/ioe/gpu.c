#include <am.h>
#include <nemu.h>
#include <stdio.h>
#include <string.h>

static uint32_t width = 0;
static uint32_t height = 0;

#define SYNC_ADDR (VGACTL_ADDR + 4)
void __am_gpu_init() {
    uint32_t val = inl(VGACTL_ADDR);
    width = (val >> 16) & 0xffff;
    height = val & 0xffff;

    uint32_t* fb = (uint32_t*)FB_ADDR;
    for (int i = 0; i < width * height; i++) fb[i] = i;
    outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T* cfg) {
    // uint32_t val = inl(VGACTL_ADDR);
    // uint32_t width = (val >> 16) & 0xffff;
    // uint32_t height = val & 0xffff;
    *cfg = (AM_GPU_CONFIG_T){.present = true,
                             .has_accel = false,
                             .width = width,
                             .height = height,
                             .vmemsz = width * height * sizeof(uint32_t)};
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T* ctl) {
    // uint32_t val = inl(VGACTL_ADDR);
    // uint32_t width = (val >> 16) & 0xffff;
    // uint32_t height = val & 0xffff;

    uint32_t* fb = (uint32_t*)FB_ADDR;
    uint32_t* px = (uint32_t*)ctl->pixels;

    //    printf("draw. x = %d, y = %d, w = %d, h = %d\n", ctl->x, ctl->y, ctl->h, ctl->w);
    for (int i = 0; i < ctl->h; i++) {
        int base = (ctl->y + i) * width + ctl->x;
        int base2 = i * ctl->w;
        memcpy((fb + base), px + base2, sizeof(uint32_t) * ctl->w);
        // for (int j = 0; j < ctl->w; j++) {
        //     uintptr_t p = (uintptr_t)(fb + base + j);
        //     outl(p, px[base2 + j]);
        // }
    }

    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T* status) {
    status->ready = true;
}
