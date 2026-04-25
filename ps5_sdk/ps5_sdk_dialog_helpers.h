#ifndef PS5_SDK_PS5_SDK_DIALOG_HELPERS_H
#define PS5_SDK_PS5_SDK_DIALOG_HELPERS_H

#include "ps5_sdk_types.h"

#define PS5SDK_DLG_BUF_SIZE 0x400u
#define PS5SDK_DLG_DP_OFF   0x000u
#define PS5SDK_DLG_PP_OFF   0x088u
#define PS5SDK_DLG_MSG_OFF  0x0E0u

static void ps5_sdk_dialog_zero(u8 *p, u32 size) {
    if (!p) return;
    for (u32 i = 0; i < size; i++) p[i] = 0;
}

static u8 *ps5_sdk_dialog_dp(u8 *buf) {
    return buf ? buf + PS5SDK_DLG_DP_OFF : 0;
}

static u8 *ps5_sdk_dialog_pp(u8 *buf) {
    return buf ? buf + PS5SDK_DLG_PP_OFF : 0;
}

static char *ps5_sdk_dialog_msg(u8 *buf) {
    return buf ? (char *)(buf + PS5SDK_DLG_MSG_OFF) : 0;
}

static void ps5_sdk_dialog_setup_user_msg(u8 *dp, u8 *pp, char *msg, s32 user_id) {
    u32 magic = (u32)((u64)dp + 0xC0D1A109);
    *(u64 *)(dp + 0x00) = 0x30;
    *(u32 *)(dp + 0x2C) = magic;
    *(u64 *)(dp + 0x30) = 0x88;
    *(u32 *)(dp + 0x38) = 1;
    *(u64 *)(dp + 0x40) = (u64)pp;
    *(u32 *)(dp + 0x58) = (u32)user_id;
    *(u32 *)(pp + 0x00) = 0;
    *(u64 *)(pp + 0x08) = (u64)msg;
}

static void ps5_sdk_dialog_setup_progress(u8 *dp, u8 *pp, char *msg, s32 user_id) {
    u32 magic = (u32)((u64)dp + 0xC0D1A109);
    *(u64 *)(dp + 0x00) = 0x30;
    *(u32 *)(dp + 0x2C) = magic;
    *(u64 *)(dp + 0x30) = 0x88;
    *(u32 *)(dp + 0x38) = 2;
    *(u64 *)(dp + 0x48) = (u64)pp;
    *(u32 *)(dp + 0x58) = (u32)user_id;
    *(u32 *)(pp + 0x00) = 0;
    *(u64 *)(pp + 0x08) = (u64)msg;
}

static void ps5_sdk_dialog_prepare_text(u8 *buf, s32 user_id, const char *fmt, __builtin_va_list ap) {
    u8 *dp = ps5_sdk_dialog_dp(buf);
    u8 *pp = ps5_sdk_dialog_pp(buf);
    char *msg = ps5_sdk_dialog_msg(buf);
    ps5_sdk_dialog_zero(buf, PS5SDK_DLG_BUF_SIZE);
    ps5_sdk_vsnprintf(msg, 160, fmt, ap);
    ps5_sdk_dialog_setup_user_msg(dp, pp, msg, user_id);
}

static void ps5_sdk_dialog_prepare_progress(u8 *buf, s32 user_id, const char *fmt, __builtin_va_list ap) {
    u8 *dp = ps5_sdk_dialog_dp(buf);
    u8 *pp = ps5_sdk_dialog_pp(buf);
    char *msg = ps5_sdk_dialog_msg(buf);
    ps5_sdk_dialog_zero(buf, PS5SDK_DLG_BUF_SIZE);
    ps5_sdk_vsnprintf(msg, 160, fmt, ap);
    ps5_sdk_dialog_setup_progress(dp, pp, msg, user_id);
}

#endif /* PS5_SDK_PS5_SDK_DIALOG_HELPERS_H */
