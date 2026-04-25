#ifndef PS5_SDK_PS5_SDK_NETWORK_HELPERS_H
#define PS5_SDK_PS5_SDK_NETWORK_HELPERS_H

#include "ps5_sdk_types.h"

static const char *ps5_sdk_netctl_state_name(s32 st) {
    if (st == 0) return "DISCONNECTED";
    if (st == 1) return "CONNECTING";
    if (st == 2) return "IPOBTAINING";
    if (st == 3) return "IPOBTAINED";
    return "UNKNOWN";
}

static const char *ps5_sdk_netctl_state_short_name(s32 st) {
    if (st == 0) return "OFFLINE";
    if (st == 1) return "CONNECT";
    if (st == 2) return "IP WAIT";
    if (st == 3) return "ONLINE";
    return "UNKNOWN";
}

static s32 ps5_sdk_netctl_get_state(void *G, void *fn, s32 *state_out) {
    if (!fn || !state_out) return -1;
    *state_out = -1;
    return (s32)NC(G, fn, (u64)state_out, 0, 0, 0, 0, 0);
}

static s32 ps5_sdk_netctl_is_online(s32 state) {
    return state == 3;
}

#endif /* PS5_SDK_PS5_SDK_NETWORK_HELPERS_H */
