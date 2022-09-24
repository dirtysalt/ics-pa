#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) [AM_KEY_##key] = #key,

static const char* keyname[256] __attribute__((used)) = {[AM_KEY_NONE] = "NONE", AM_KEYS(NAME)};

void yield();
size_t serial_write(const void* buf, size_t offset, size_t len) {
    MULTIPROGRAM_YIELD();
    const char* b = (const char*)buf;
    for (size_t i = 0; i < len; i++) {
        putch(b[i]);
    }
    return len;
}

size_t events_read(void* buf, size_t offset, size_t len) {
    MULTIPROGRAM_YIELD();
    // TODO(yan): looks like can not read keystroke.
    AM_INPUT_KEYBRD_T t = io_read(AM_INPUT_KEYBRD);
    if (t.keycode == AM_KEY_NONE) return 0;
    int ret = 0;
    if (t.keydown) {
        ret = sprintf(buf, "kd %s\n", keyname[t.keycode]);
    } else {
        ret = sprintf(buf, "ku %s\n", keyname[t.keycode]);
    }
    return ret;
}

size_t dispinfo_read(void* buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T t = io_read(AM_GPU_CONFIG);
    int width = t.width;
    int height = t.height;
    char* b = (char*)buf;
    memcpy(b, &width, sizeof(width));
    memcpy(b + 4, &height, sizeof(height));
    return 8;
}

struct DrawRequest {
    uint32_t* px;
    int x;
    int y;
    int w;
    int h;
};

size_t fb_write(const void* buf, size_t offset, size_t len) {
    MULTIPROGRAM_YIELD();
    struct DrawRequest* req = (struct DrawRequest*)buf;
    assert(len == sizeof(*req));
    AM_GPU_FBDRAW_T t;
    t.pixels = req->px;
    t.x = req->x;
    t.y = req->y;
    t.w = req->w;
    t.h = req->h;
    t.sync = true;
    Log("fb write. pixels = %p, x = %d, y = %d, w = %d, h = %d", t.pixels, t.x, t.y, t.w, t.h);
    ioe_write(AM_GPU_FBDRAW, &t);
    return 0;
}

void init_device() {
    Log("Initializing devices...");
    ioe_init();
}
