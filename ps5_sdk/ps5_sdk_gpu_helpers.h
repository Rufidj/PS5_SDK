#ifndef PS5_SDK_PS5_SDK_GPU_HELPERS_H
#define PS5_SDK_PS5_SDK_GPU_HELPERS_H

#include "ps5_sdk_types.h"

static u64 ps5_sdk_gpu_get_core_clock_hz(void *G, void *fn) {
    if (!fn) return 0;
    return (u64)NC(G, fn, 0, 0, 0, 0, 0, 0);
}

static u32 ps5_sdk_gpu_get_core_clock_mhz(void *G, void *fn) {
    return (u32)(ps5_sdk_gpu_get_core_clock_hz(G, fn) / 1000000ULL);
}

static s32 ps5_sdk_gpu_is_user_pa_enabled(void *G, void *fn) {
    if (!fn) return -1;
    return (s32)NC(G, fn, 0, 0, 0, 0, 0, 0);
}

#endif /* PS5_SDK_PS5_SDK_GPU_HELPERS_H */
