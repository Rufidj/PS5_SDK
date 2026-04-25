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

    void *dlg_init;
    void *dlg_open;
    void *dlg_term;
    void *dlg_set_val;
    void *dlg_set_msg;
    void *dlg_update;
    void *dlg_close;
};

static void ex_zero(u8 *p, u32 size) {
    for (u32 i = 0; i < size; i++) p[i] = 0;
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
    if (!ex->load_mod || !ex->mmap_fn) return -1;

    NC(ex->G, ex->load_mod, (u64)"libSceMsgDialog.sprx", 0, 0, 0, 0, 0);

    ex->dlg_init = SYM(ex->G, ex->D, MSGDIALOG_HANDLE, MSGDIALOG_sceMsgDialogInitialize);
    ex->dlg_open = SYM(ex->G, ex->D, MSGDIALOG_HANDLE, MSGDIALOG_sceMsgDialogOpen);
    ex->dlg_term = SYM(ex->G, ex->D, MSGDIALOG_HANDLE, MSGDIALOG_sceMsgDialogTerminate);
    ex->dlg_set_val = SYM(ex->G, ex->D, MSGDIALOG_HANDLE, MSGDIALOG_sceMsgDialogProgressBarSetValue);
    ex->dlg_set_msg = SYM(ex->G, ex->D, MSGDIALOG_HANDLE, MSGDIALOG_sceMsgDialogProgressBarSetMsg);
    ex->dlg_update = SYM(ex->G, ex->D, MSGDIALOG_HANDLE, MSGDIALOG_sceMsgDialogUpdateStatus);
    ex->dlg_close = SYM(ex->G, ex->D, MSGDIALOG_HANDLE, MSGDIALOG_sceMsgDialogClose);
    if (!ex->dlg_init || !ex->dlg_open || !ex->dlg_term) return -2;

    return 0;
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

#endif /* PS5_SDK_EXAMPLE_COMMON_H */
