#include "example_common.h"

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) {
        ext->status = ret;
        return;
    }

    s32 load_ret = ex_module_handle(&ex, "libSceVideoOut.sprx");
    void *vid_open = ex_sym_module(&ex, "libSceVideoOut.sprx", "sceVideoOutOpen");
    void *vid_close = ex_sym_module(&ex, "libSceVideoOut.sprx", "sceVideoOutClose");
    void *vid_reg = ex_sym_module(&ex, "libSceVideoOut.sprx", "sceVideoOutRegisterBuffers");
    void *vid_flip = ex_sym_module(&ex, "libSceVideoOut.sprx", "sceVideoOutSubmitFlip");
    void *vid_rate = ex_sym_module(&ex, "libSceVideoOut.sprx", "sceVideoOutSetFlipRate");
    void *vid_evt = ex_sym_module(&ex, "libSceVideoOut.sprx", "sceVideoOutAddFlipEvent");

    u64 host_raw = *(u64 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);
    u64 q0 = 0, q1 = 0, q2 = 0, q3 = 0;

    if (host_raw >= 0x0000000800000000ULL && host_raw < 0x0000000900000000ULL) {
        u64 *p = (u64 *)host_raw;
        q0 = p[0];
        q1 = p[1];
        q2 = p[2];
        q3 = p[3];
    }

    u32 resolved = 0;
    if (vid_open) resolved++;
    if (vid_close) resolved++;
    if (vid_reg) resolved++;
    if (vid_flip) resolved++;
    if (vid_rate) resolved++;
    if (vid_evt) resolved++;

    ext->dbg[0] = host_raw;
    ext->dbg[1] = gs_thread;
    ext->dbg[2] = q0;
    ext->dbg[3] = q1;
    ext->dbg[4] = q2;
    ext->dbg[5] = q3;
    ext->dbg[6] = (u64)resolved;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "VO host state\nload=%x resolved=%d/6\nraw=%lx gs=%lx\nq0=%lx\nq1=%lx",
        (u32)load_ret,
        (int)resolved,
        host_raw,
        gs_thread,
        q0,
        q1
    );
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
