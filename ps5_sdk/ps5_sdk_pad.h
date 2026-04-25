#ifndef PS5_SDK_PS5_SDK_PAD_H
#define PS5_SDK_PS5_SDK_PAD_H

#include "ps5_sdk_types.h"

#define PS5SDK_PAD_BTN_OPTIONS  0x00000008u
#define PS5SDK_PAD_BTN_UP       0x00000010u
#define PS5SDK_PAD_BTN_RIGHT    0x00000020u
#define PS5SDK_PAD_BTN_DOWN     0x00000040u
#define PS5SDK_PAD_BTN_LEFT     0x00000080u
#define PS5SDK_PAD_BTN_L1       0x00000400u
#define PS5SDK_PAD_BTN_R1       0x00000800u
#define PS5SDK_PAD_BTN_TRIANGLE 0x00001000u
#define PS5SDK_PAD_BTN_CIRCLE   0x00002000u
#define PS5SDK_PAD_BTN_CROSS    0x00004000u
#define PS5SDK_PAD_BTN_SQUARE   0x00008000u

static u32 ps5_sdk_pad_buttons_from_raw(u32 raw) {
    return raw & 0x001FFFFFu;
}

static u32 ps5_sdk_pad_first_button(u32 b) {
    if (b & PS5SDK_PAD_BTN_CROSS) return PS5SDK_PAD_BTN_CROSS;
    if (b & PS5SDK_PAD_BTN_SQUARE) return PS5SDK_PAD_BTN_SQUARE;
    if (b & PS5SDK_PAD_BTN_CIRCLE) return PS5SDK_PAD_BTN_CIRCLE;
    if (b & PS5SDK_PAD_BTN_TRIANGLE) return PS5SDK_PAD_BTN_TRIANGLE;
    if (b & PS5SDK_PAD_BTN_L1) return PS5SDK_PAD_BTN_L1;
    if (b & PS5SDK_PAD_BTN_R1) return PS5SDK_PAD_BTN_R1;
    if (b & PS5SDK_PAD_BTN_UP) return PS5SDK_PAD_BTN_UP;
    if (b & PS5SDK_PAD_BTN_DOWN) return PS5SDK_PAD_BTN_DOWN;
    if (b & PS5SDK_PAD_BTN_LEFT) return PS5SDK_PAD_BTN_LEFT;
    if (b & PS5SDK_PAD_BTN_RIGHT) return PS5SDK_PAD_BTN_RIGHT;
    if (b & PS5SDK_PAD_BTN_OPTIONS) return PS5SDK_PAD_BTN_OPTIONS;
    return 0;
}

static const char *ps5_sdk_pad_button_name(u32 b) {
    if (b == PS5SDK_PAD_BTN_CROSS) return "CROSS";
    if (b == PS5SDK_PAD_BTN_SQUARE) return "SQUARE";
    if (b == PS5SDK_PAD_BTN_CIRCLE) return "CIRCLE";
    if (b == PS5SDK_PAD_BTN_TRIANGLE) return "TRIANGLE";
    if (b == PS5SDK_PAD_BTN_L1) return "L1";
    if (b == PS5SDK_PAD_BTN_R1) return "R1";
    if (b == PS5SDK_PAD_BTN_UP) return "UP";
    if (b == PS5SDK_PAD_BTN_DOWN) return "DOWN";
    if (b == PS5SDK_PAD_BTN_LEFT) return "LEFT";
    if (b == PS5SDK_PAD_BTN_RIGHT) return "RIGHT";
    if (b == PS5SDK_PAD_BTN_OPTIONS) return "OPTIONS";
    return "none";
}

static void ps5_sdk_pad_append_text(char *dst, u32 dst_size, const char *src) {
    u32 pos = 0;
    while (pos + 1 < dst_size && dst[pos]) pos++;
    while (pos + 1 < dst_size && *src) dst[pos++] = *src++;
    dst[pos] = 0;
}

static void ps5_sdk_pad_append_button_name(char *dst, u32 dst_size, u32 *first, const char *name) {
    if (!*first) ps5_sdk_pad_append_text(dst, dst_size, " ");
    ps5_sdk_pad_append_text(dst, dst_size, name);
    *first = 0;
}

static void ps5_sdk_pad_names_from_buttons(u32 b, char *dst, u32 dst_size) {
    u32 first = 1;
    if (!dst || dst_size == 0) return;
    dst[0] = 0;

    if (b & PS5SDK_PAD_BTN_CROSS) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "CROSS");
    if (b & PS5SDK_PAD_BTN_SQUARE) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "SQUARE");
    if (b & PS5SDK_PAD_BTN_CIRCLE) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "CIRCLE");
    if (b & PS5SDK_PAD_BTN_TRIANGLE) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "TRIANGLE");
    if (b & PS5SDK_PAD_BTN_L1) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "L1");
    if (b & PS5SDK_PAD_BTN_R1) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "R1");
    if (b & PS5SDK_PAD_BTN_UP) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "UP");
    if (b & PS5SDK_PAD_BTN_DOWN) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "DOWN");
    if (b & PS5SDK_PAD_BTN_LEFT) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "LEFT");
    if (b & PS5SDK_PAD_BTN_RIGHT) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "RIGHT");
    if (b & PS5SDK_PAD_BTN_OPTIONS) ps5_sdk_pad_append_button_name(dst, dst_size, &first, "OPTIONS");
    if (first) ps5_sdk_pad_append_text(dst, dst_size, "none");
}

static s32 ps5_sdk_pad_read_buttons(void *G, void *pad_read, s32 pad_h, u8 *pad_buf, u32 *raw_out, u32 *buttons_out) {
    if (!pad_buf || !raw_out || !buttons_out || !pad_read || pad_h < 0) return -1;
    for (int i = 0; i < 128; i++) pad_buf[i] = 0;
    s32 ret = (s32)NC(G, pad_read, (u64)pad_h, (u64)pad_buf, 1, 0, 0, 0);
    *raw_out = *(u32 *)pad_buf;
    *buttons_out = ps5_sdk_pad_buttons_from_raw(*raw_out);
    return ret;
}

static void ps5_sdk_pad_fill_lightbar_rgb(u8 *dst, u8 r, u8 g, u8 b) {
    if (!dst) return;
    dst[0] = r;
    dst[1] = g;
    dst[2] = b;
    dst[3] = 0;
}

static s32 ps5_sdk_pad_set_lightbar(void *G, void *pad_set_lightbar, s32 pad_h, u8 *work, u8 r, u8 g, u8 b) {
    if (!G || !pad_set_lightbar || !work || pad_h < 0) return -1;
    for (int i = 0; i < 0x1000; i++) work[i] = 0;
    ps5_sdk_pad_fill_lightbar_rgb(work, r, g, b);
    return (s32)NC(G, pad_set_lightbar, (u64)pad_h, (u64)work, 0, 0, 0, 0);
}

static s32 ps5_sdk_pad_reset_lightbar(void *G, void *pad_reset_lightbar, s32 pad_h) {
    if (!G || !pad_reset_lightbar || pad_h < 0) return -1;
    return (s32)NC(G, pad_reset_lightbar, (u64)pad_h, 0, 0, 0, 0, 0);
}

#endif /* PS5_SDK_PS5_SDK_PAD_H */
