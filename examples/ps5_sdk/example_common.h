#ifndef PS5_SDK_EXAMPLE_COMMON_H
#define PS5_SDK_EXAMPLE_COMMON_H

#include "ps5_sdk.h"

struct ext_args {
    s64 status;
    s64 step;
    u32 frame_count;
    u32 _pad;
    s32 log_fd;
    s32 pad_fd;
    u8  log_addr[16];
    u64 dbg[8];
};

struct ps5_example {
    void *G;
    void *D;
    s32 user_id;

    void *load_mod;
    void *usleep_fn;
    void *mmap_fn;
    void *munmap_fn;
    void *exit_fn;
    void *c_exit_fn;

    void *dlg_init;
    void *dlg_open;
    void *dlg_term;
    void *dlg_set_val;
    void *dlg_set_msg;
    void *dlg_update;
    void *dlg_close;

    /* Dynamic module handles (resolved at runtime). */
    s32 h_msgdialog;
    s32 h_pad;
    s32 h_audioout;
    s32 h_userservice;
    s32 h_rtc;
    s32 h_netctl;
    s32 h_systemservice;
    s32 h_gnm;
    s32 h_videoout;
};

static void ex_zero(u8 *p, u32 size) {
    for (u32 i = 0; i < size; i++) p[i] = 0;
}

static int ex_streq(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return (*a == 0 && *b == 0);
}

static int ex_init(struct ps5_example *ex, u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    ex_zero((u8 *)ex, sizeof(*ex));
    ex->G = (void *)(eboot_base + GADGET_OFFSET);
    ex->D = (void *)dlsym_addr;
    ex->user_id = ext ? (s32)ext->dbg[3] : 0;

    ex->load_mod = SYM(ex->G, ex->D, LIBKERNEL_HANDLE, KERNEL_sceKernelLoadStartModule);
    ex->usleep_fn = SYM(ex->G, ex->D, LIBKERNEL_HANDLE, KERNEL_sceKernelUsleep);
    ex->mmap_fn = SYM(ex->G, ex->D, LIBKERNEL_HANDLE, KERNEL_mmap);
    ex->munmap_fn = SYM(ex->G, ex->D, LIBKERNEL_HANDLE, KERNEL_munmap);
    ex->exit_fn = SYM(ex->G, ex->D, LIBKERNEL_HANDLE, KERNEL__exit);
    ex->c_exit_fn = SYM(ex->G, ex->D, C_HANDLE, "exit");
    if (!ex->load_mod || !ex->mmap_fn) return -1;

    ex->h_msgdialog = (s32)NC(ex->G, ex->load_mod, (u64)"libSceMsgDialog.sprx", 0, 0, 0, 0, 0);
    if (ex->h_msgdialog < 0) return -2;

    ex->dlg_init = SYM(ex->G, ex->D, ex->h_msgdialog, MSGDIALOG_sceMsgDialogInitialize);
    ex->dlg_open = SYM(ex->G, ex->D, ex->h_msgdialog, MSGDIALOG_sceMsgDialogOpen);
    ex->dlg_term = SYM(ex->G, ex->D, ex->h_msgdialog, MSGDIALOG_sceMsgDialogTerminate);
    ex->dlg_set_val = SYM(ex->G, ex->D, ex->h_msgdialog, MSGDIALOG_sceMsgDialogProgressBarSetValue);
    ex->dlg_set_msg = SYM(ex->G, ex->D, ex->h_msgdialog, MSGDIALOG_sceMsgDialogProgressBarSetMsg);
    ex->dlg_update = SYM(ex->G, ex->D, ex->h_msgdialog, MSGDIALOG_sceMsgDialogUpdateStatus);
    ex->dlg_close = SYM(ex->G, ex->D, ex->h_msgdialog, MSGDIALOG_sceMsgDialogClose);
    if (!ex->dlg_init || !ex->dlg_open || !ex->dlg_term) return -2;

    return 0;
}

static s32 ex_load_module(struct ps5_example *ex, const char *path) {
    if (!ex || !ex->G || !ex->load_mod || !path) return -1;
    return (s32)NC(ex->G, ex->load_mod, (u64)path, 0, 0, 0, 0, 0);
}

static s32 ex_module_handle(struct ps5_example *ex, const char *path) {
    if (!ex || !path) return -1;

    if (ex_streq(path, "libScePad.sprx")) {
        if (ex->h_pad <= 0) ex->h_pad = ex_load_module(ex, path);
        return ex->h_pad;
    }
    if (ex_streq(path, "libSceAudioOut.sprx")) {
        if (ex->h_audioout <= 0) ex->h_audioout = ex_load_module(ex, path);
        return ex->h_audioout;
    }
    if (ex_streq(path, "libSceUserService.sprx")) {
        if (ex->h_userservice <= 0) ex->h_userservice = ex_load_module(ex, path);
        return ex->h_userservice;
    }
    if (ex_streq(path, "libSceRtc.sprx")) {
        if (ex->h_rtc <= 0) ex->h_rtc = ex_load_module(ex, path);
        return ex->h_rtc;
    }
    if (ex_streq(path, "libSceNetCtl.sprx")) {
        if (ex->h_netctl <= 0) ex->h_netctl = ex_load_module(ex, path);
        return ex->h_netctl;
    }
    if (ex_streq(path, "libSceSystemService.sprx")) {
        if (ex->h_systemservice <= 0) ex->h_systemservice = ex_load_module(ex, path);
        return ex->h_systemservice;
    }
    if (ex_streq(path, "libSceGnmDriver.sprx")) {
        if (ex->h_gnm <= 0) ex->h_gnm = ex_load_module(ex, path);
        return ex->h_gnm;
    }
    if (ex_streq(path, "libSceVideoOut.sprx")) {
        if (ex->h_videoout <= 0) ex->h_videoout = ex_load_module(ex, path);
        return ex->h_videoout;
    }
    if (ex_streq(path, "libSceMsgDialog.sprx")) {
        if (ex->h_msgdialog <= 0) ex->h_msgdialog = ex_load_module(ex, path);
        return ex->h_msgdialog;
    }
    return ex_load_module(ex, path);
}

static void *ex_sym_module(struct ps5_example *ex, const char *path, const char *name) {
    s32 h = ex_module_handle(ex, path);
    if (!ex || !ex->G || !ex->D || !name || h <= 0) return 0;
    return SYM(ex->G, ex->D, h, name);
}

static u8 *ex_alloc_dialog(struct ps5_example *ex) {
    u8 *buf = (u8 *)NC(ex->G, ex->mmap_fn, 0, PS5SDK_DLG_BUF_SIZE, 3, 0x1002, (u64)-1, 0);
    if ((s64)buf == -1) return 0;
    ps5_sdk_dialog_zero(buf, PS5SDK_DLG_BUF_SIZE);
    return buf;
}

static void ex_free_dialog(struct ps5_example *ex, u8 *buf) {
    if (buf && ex->munmap_fn) {
        NC(ex->G, ex->munmap_fn, (u64)buf, PS5SDK_DLG_BUF_SIZE, 0, 0, 0, 0);
    }
}

static void ex_dialog_begin(struct ps5_example *ex) {
    NC(ex->G, ex->dlg_term, 0, 0, 0, 0, 0, 0);
    NC(ex->G, ex->dlg_init, 0, 0, 0, 0, 0, 0);
}

static s32 ex_open_text(struct ps5_example *ex, u8 *buf, const char *fmt, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    ps5_sdk_dialog_prepare_text(buf, ex->user_id, fmt, ap);
    __builtin_va_end(ap);
    return (s32)NC(ex->G, ex->dlg_open, (u64)ps5_sdk_dialog_dp(buf), 0, 0, 0, 0, 0);
}

static s32 ex_open_progress(struct ps5_example *ex, u8 *buf, const char *fmt, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    ps5_sdk_dialog_prepare_progress(buf, ex->user_id, fmt, ap);
    __builtin_va_end(ap);
    return (s32)NC(ex->G, ex->dlg_open, (u64)ps5_sdk_dialog_dp(buf), 0, 0, 0, 0, 0);
}

static void ex_wait_user_close(struct ps5_example *ex) {
    for (;;) {
        s32 st = 1;
        if (ex->dlg_update) st = (s32)NC(ex->G, ex->dlg_update, 0, 0, 0, 0, 0, 0);
        if (st == 3) break; /* SCE_COMMON_DIALOG_STATUS_FINISHED */
        if (ex->usleep_fn) NC(ex->G, ex->usleep_fn, 50000, 0, 0, 0, 0, 0);
    }
}

static void ex_close_dialog(struct ps5_example *ex) {
    if (ex->dlg_close) NC(ex->G, ex->dlg_close, 0, 0, 0, 0, 0, 0);
    if (ex->dlg_update) {
        for (int i = 0; i < 20; i++) {
            if ((s32)NC(ex->G, ex->dlg_update, 0, 0, 0, 0, 0, 0) == 0) break;
            if (ex->usleep_fn) NC(ex->G, ex->usleep_fn, 50000, 0, 0, 0, 0, 0);
        }
    }
    if (ex->dlg_term) NC(ex->G, ex->dlg_term, 0, 0, 0, 0, 0, 0);
}

static void ex_exit_clean(struct ps5_example *ex, s32 code) {
    if (!ex || !ex->G || !ex->D) return;

    /* Try official UI return path first (best-effort): SystemService KillApp(currentAppId). */
    s32 kill_sent = 0;
    {
        s32 h_sys = ex_module_handle(ex, "libSceSystemService.sprx");
        if (h_sys > 0) {
            void *get_app_status = SYM(ex->G, ex->D, h_sys, SYSTEMSERVICE_sceSystemServiceGetAppStatus);
            void *kill_app       = SYM(ex->G, ex->D, h_sys, SYSTEMSERVICE_sceSystemServiceKillApp);
            if (get_app_status && kill_app) {
                u8 st[0x100];
                ex_zero(st, sizeof(st));
                s32 gr = (s32)NC(ex->G, get_app_status, (u64)st, 0, 0, 0, 0, 0);
                if (gr == 0) {
                    /* Conservative app-id pick only from known slots. */
                    u32 id0 = *(u32 *)(st + 0x08);
                    u32 id1 = *(u32 *)(st + 0x00);
                    u32 app_id = 0;
                    if (id0 >= 0x10u && id0 <= 0x7FFFFFFFu && id0 != 0xFFFFFFFFu) app_id = id0;
                    else if (id1 >= 0x10u && id1 <= 0x7FFFFFFFu && id1 != 0xFFFFFFFFu) app_id = id1;
                    if (app_id) {
                        s32 kr = (s32)NC(ex->G, kill_app, (u64)app_id, 0, 0, 0, 0, 0);
                        if (kr == 0) kill_sent = 1;
                    }
                    if (kill_sent && ex->usleep_fn) NC(ex->G, ex->usleep_fn, 500000, 0, 0, 0, 0, 0);
                }
            }
        }
    }

    /* Pure C-driven exit path: when KillApp is accepted, wait for UI takeover. */
    if (kill_sent && ex->usleep_fn) {
        for (;;) NC(ex->G, ex->usleep_fn, 250000, 0, 0, 0, 0, 0);
    }

    /* Fallback: avoid hard _exit here, return to caller. */
    (void)code;
    if (ex->usleep_fn) NC(ex->G, ex->usleep_fn, 120000, 0, 0, 0, 0, 0);
}

static void ex_finish(struct ps5_example *ex, struct ext_args *ext, s32 status, s32 code) {
    if (ext) ext->status = status;
    ex_exit_clean(ex, code);
}

#endif /* PS5_SDK_EXAMPLE_COMMON_H */
