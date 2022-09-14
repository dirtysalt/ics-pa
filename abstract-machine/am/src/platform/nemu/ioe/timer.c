#include <am.h>
#include <nemu.h>

void __am_timer_init() {}

void __am_timer_uptime(AM_TIMER_UPTIME_T* uptime) {
    // timer.c in nemu,
    // I have to read 4 bytes after to load value
    uint32_t hi = inl(RTC_ADDR + 4);
    uint32_t lo = inl(RTC_ADDR);
    uint64_t us = ((uint64_t)hi << 32) | (uint64_t)lo;
    uptime->us = us;
}

void __am_timer_rtc(AM_TIMER_RTC_T* rtc) {
    rtc->second = 0;
    rtc->minute = 0;
    rtc->hour = 0;
    rtc->day = 0;
    rtc->month = 0;
    rtc->year = 1900;
}
