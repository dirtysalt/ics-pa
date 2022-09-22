#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int event_fd = -1;

uint32_t NDL_GetTicks() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint32_t t = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return t;
}

int NDL_PollEvent(char* buf, int len) {
    return read(event_fd, buf, len);
}

void NDL_OpenCanvas(int* w, int* h) {
    if (getenv("NWM_APP")) {
        int fbctl = 4;
        fbdev = 5;
        screen_w = *w;
        screen_h = *h;
        char buf[64];
        int len = sprintf(buf, "%d %d", screen_w, screen_h);
        // let NWM resize the window and create the frame buffer
        write(fbctl, buf, len);
        while (1) {
            // 3 = evtdev
            int nread = read(3, buf, sizeof(buf) - 1);
            if (nread <= 0) continue;
            buf[nread] = '\0';
            if (strcmp(buf, "mmap ok") == 0) break;
        }
        close(fbctl);
    }

    int fd = open("/proc/dispinfo", 0, 0);
    char buf[8];
    read(fd, buf, sizeof(buf));
    int width, height;
    memcpy(&width, buf, sizeof(width));
    memcpy(&height, buf + 4, sizeof(height));
    *w = width;
    *h = height;
    screen_w = width;
    screen_h = height;
}

struct DrawRequest {
    uint32_t* px;
    int x;
    int y;
    int w;
    int h;
};

void NDL_DrawRect(uint32_t* pixels, int x, int y, int w, int h) {
    // normally we should write bytes, but sice we are in a single app
    // we can pass pointer.
    struct DrawRequest req = {.px = pixels, .x = x, .y = y, .w = w, .h = h};
    write(fbdev, &req, sizeof(req));
}

void NDL_OpenAudio(int freq, int channels, int samples) {}

void NDL_CloseAudio() {}

int NDL_PlayAudio(void* buf, int len) {
    return 0;
}

int NDL_QueryAudio() {
    return 0;
}

int NDL_Init(uint32_t flags) {
    if (getenv("NWM_APP")) {
        evtdev = 3;
    }
    event_fd = open("/dev/events", 0, 0);
    fbdev = open("/dev/fb", 0, 0);
    return 0;
}

void NDL_Quit() {
    close(event_fd);
    close(fbdev);
}
