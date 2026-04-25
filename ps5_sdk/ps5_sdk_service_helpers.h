#ifndef PS5_SDK_PS5_SDK_SERVICE_HELPERS_H
#define PS5_SDK_PS5_SDK_SERVICE_HELPERS_H

#include "ps5_sdk_types.h"

struct ps5_sdk_rtc_clock {
    u16 year;
    u16 month;
    u16 day;
    u16 hour;
    u16 minute;
    u16 second;
    u32 microsecond;
};

static s64 ps5_sdk_system_get_int(void *G, void *fn, s32 id, s32 *out) {
    if (!fn || !out) return -1;
    *out = 0x7fffffff;
    return (s64)NC(G, fn, (u64)id, (u64)out, 0, 0, 0, 0);
}

static void ps5_sdk_ascii_copy(char *dst, u32 max, const char *src) {
    u32 i = 0;
    if (!dst || max == 0) return;
    if (src) {
        while (i + 1 < max && src[i]) {
            char c = src[i];
            dst[i] = (c >= 32 && c <= 126) ? c : '?';
            i++;
        }
    }
    dst[i] = 0;
}

static s64 ps5_sdk_user_get_login_ids(void *G, void *fn, s32 *ids, u32 max_ids, u8 *work, u32 work_size) {
    if (!fn || !ids || !work || work_size < (max_ids * sizeof(s32))) return -1;
    for (u32 i = 0; i < work_size; i++) work[i] = 0;
    for (u32 i = 0; i < max_ids; i++) ((s32 *)work)[i] = -1;
    s64 ret = (s64)NC(G, fn, (u64)work, 0, 0, 0, 0, 0);
    if (ret == 0) {
        for (u32 i = 0; i < max_ids; i++) ids[i] = ((s32 *)work)[i];
    }
    return ret;
}

static s64 ps5_sdk_user_get_name(void *G, void *fn, s32 user_id, char *dst, u32 dst_size, u8 *work, u32 work_size) {
    if (!fn || !dst || dst_size == 0 || !work || work_size == 0) return -1;
    for (u32 i = 0; i < work_size; i++) work[i] = 0;
    s64 ret = (s64)NC(G, fn, (u64)user_id, (u64)work, (u64)work_size, 0, 0, 0);
    if (ret == 0) ps5_sdk_ascii_copy(dst, dst_size, (const char *)work);
    else dst[0] = 0;
    return ret;
}

static s32 ps5_sdk_rtc_get_tick(void *G, void *fn, u64 *tick_out) {
    if (!fn || !tick_out) return -1;
    *tick_out = 0;
    return (s32)NC(G, fn, (u64)tick_out, 0, 0, 0, 0, 0);
}

static s32 ps5_sdk_rtc_get_local_clock(void *G, void *fn, struct ps5_sdk_rtc_clock *clk) {
    if (!fn || !clk) return -1;
    clk->year = clk->month = clk->day = 0;
    clk->hour = clk->minute = clk->second = 0;
    clk->microsecond = 0;
    return (s32)NC(G, fn, (u64)clk, 0, 0, 0, 0, 0);
}

#endif /* PS5_SDK_PS5_SDK_SERVICE_HELPERS_H */
