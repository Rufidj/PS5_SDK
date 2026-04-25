/* ps5_sdk_runtime.h - tiny runtime helpers for PS5 payloads.
 * No libc required.  Keep this state-free: flat payloads should not depend on
 * writable global runtime storage being mapped/zeroed like a normal ELF.
 */
#ifndef PS5_SDK_PS5_SDK_RUNTIME_H
#define PS5_SDK_PS5_SDK_RUNTIME_H

#include "ps5_sdk_types.h"
#include "ps5_sdk_core.h"

static void ps5_sdk_putc(char *out, int *pos, int cap, char c) {
    if (*pos < cap - 1) out[*pos] = c;
    (*pos)++;
}

static void ps5_sdk_puts(char *out, int *pos, int cap, const char *s) {
    if (!s) s = "(null)";
    while (*s) ps5_sdk_putc(out, pos, cap, *s++);
}

static int ps5_sdk_u64_len(u64 v, int base) {
    int n = 1;
    while (v >= (u64)base) {
        v /= (u64)base;
        n++;
    }
    return n;
}

static void ps5_sdk_put_pad(char *out, int *pos, int cap, char ch, int count) {
    while (count-- > 0) ps5_sdk_putc(out, pos, cap, ch);
}

static void ps5_sdk_put_u64(char *out, int *pos, int cap, u64 v, int base, int upper) {
    const char *lo = "0123456789abcdef";
    const char *hi = "0123456789ABCDEF";
    const char *digits = upper ? hi : lo;
    char tmp[32];
    int n = 0;
    if (v == 0) {
        ps5_sdk_putc(out, pos, cap, '0');
        return;
    }
    while (v && n < (int)sizeof(tmp)) {
        tmp[n++] = digits[v % (u64)base];
        v /= (u64)base;
    }
    while (n > 0) ps5_sdk_putc(out, pos, cap, tmp[--n]);
}

static int ps5_sdk_vsnprintf(char *out, int cap, const char *fmt, __builtin_va_list ap) {
    int pos = 0;
    if (!out || cap <= 0) return 0;
    if (!fmt) fmt = "";

    while (*fmt) {
        if (*fmt != '%') {
            ps5_sdk_putc(out, &pos, cap, *fmt++);
            continue;
        }
        fmt++;
        if (*fmt == '%') {
            ps5_sdk_putc(out, &pos, cap, *fmt++);
            continue;
        }

        int zero_pad = 0;
        int width = 0;
        if (*fmt == '0') {
            zero_pad = 1;
            fmt++;
        }
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        int long_flag = 0;
        if (*fmt == 'l') {
            long_flag = 1;
            fmt++;
            if (*fmt == 'l') fmt++;
        }

        char spec = *fmt ? *fmt++ : 0;
        if (spec == 's') {
            ps5_sdk_puts(out, &pos, cap, __builtin_va_arg(ap, const char *));
        } else if (spec == 'c') {
            ps5_sdk_putc(out, &pos, cap, (char)__builtin_va_arg(ap, int));
        } else if (spec == 'd' || spec == 'i') {
            s64 v = long_flag ? __builtin_va_arg(ap, s64) : (s64)__builtin_va_arg(ap, int);
            int neg = (v < 0);
            if (v < 0) {
                v = -v;
            }
            if (zero_pad && width > 0) {
                int len = ps5_sdk_u64_len((u64)v, 10) + (neg ? 1 : 0);
                if (neg) ps5_sdk_putc(out, &pos, cap, '-');
                ps5_sdk_put_pad(out, &pos, cap, '0', width - len);
            } else if (neg) {
                ps5_sdk_putc(out, &pos, cap, '-');
            }
            ps5_sdk_put_u64(out, &pos, cap, (u64)v, 10, 0);
        } else if (spec == 'u') {
            u64 v = long_flag ? __builtin_va_arg(ap, u64) : (u64)__builtin_va_arg(ap, unsigned int);
            if (zero_pad && width > 0) {
                ps5_sdk_put_pad(out, &pos, cap, '0', width - ps5_sdk_u64_len(v, 10));
            }
            ps5_sdk_put_u64(out, &pos, cap, v, 10, 0);
        } else if (spec == 'x' || spec == 'X') {
            u64 v = long_flag ? __builtin_va_arg(ap, u64) : (u64)__builtin_va_arg(ap, unsigned int);
            if (zero_pad && width > 0) {
                ps5_sdk_put_pad(out, &pos, cap, '0', width - ps5_sdk_u64_len(v, 16));
            }
            ps5_sdk_put_u64(out, &pos, cap, v, 16, spec == 'X');
        } else if (spec == 'p') {
            u64 v = (u64)__builtin_va_arg(ap, void *);
            ps5_sdk_puts(out, &pos, cap, "0x");
            ps5_sdk_put_u64(out, &pos, cap, v, 16, 0);
        } else {
            ps5_sdk_putc(out, &pos, cap, '%');
            if (spec) ps5_sdk_putc(out, &pos, cap, spec);
        }
    }

    if (pos >= cap) out[cap - 1] = 0;
    else out[pos] = 0;
    return pos;
}

static int ps5_sdk_snprintf(char *out, int cap, const char *fmt, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    int n = ps5_sdk_vsnprintf(out, cap, fmt, ap);
    __builtin_va_end(ap);
    return n;
}

static int ps5_sdk_dialog_printf(void *G, void *dlg_set_msg, char *out, int cap, const char *fmt, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    int n = ps5_sdk_vsnprintf(out, cap, fmt, ap);
    __builtin_va_end(ap);
    if (G && dlg_set_msg && out) {
        NC(G, dlg_set_msg, 0, (u64)out, 0, 0, 0, 0);
    }
    return n;
}

static void ps5_sdk_runtime_init(void *G, void *D) {
    (void)G;
    (void)D;
}

static void ps5_sdk_runtime_set_udp_log(s32 fd, const u8 addr[16]) {
    (void)fd;
    (void)addr;
}

static void ps5_sdk_runtime_enable_udp_log(void) {
}

static void ps5_sdk_runtime_enable_debug_out(void) {
}

static int ps5_sdk_printf(const char *fmt, ...) {
    (void)fmt;
    return 0;
}

#endif /* PS5_SDK_PS5_SDK_RUNTIME_H */
