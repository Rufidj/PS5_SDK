#ifndef PS5_SDK_PS5_SDK_SYSMODULE_HELPERS_H
#define PS5_SDK_PS5_SDK_SYSMODULE_HELPERS_H

#include "ps5_sdk_types.h"

static s32 ps5_sdk_load_sprx(void *G, void *load_mod, const char *path) {
    if (!G || !load_mod || !path) return -1;
    return (s32)NC(G, load_mod, (u64)path, 0, 0, 0, 0, 0);
}

static s32 ps5_sdk_sysmodule_load(void *G, void *fn, u32 id) {
    if (!G || !fn) return -1;
    return (s32)NC(G, fn, (u64)id, 0, 0, 0, 0, 0);
}

static s32 ps5_sdk_sysmodule_unload(void *G, void *fn, u32 id) {
    if (!G || !fn) return -1;
    return (s32)NC(G, fn, (u64)id, 0, 0, 0, 0, 0);
}

static s32 ps5_sdk_sysmodule_is_loaded(void *G, void *fn, u32 id) {
    if (!G || !fn) return -1;
    return (s32)NC(G, fn, (u64)id, 0, 0, 0, 0, 0);
}

#endif /* PS5_SDK_PS5_SDK_SYSMODULE_HELPERS_H */
