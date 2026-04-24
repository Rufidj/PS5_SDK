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

    s32 load_ret = (s32)NC(ex.G, ex.load_mod, (u64)"libScePad.sprx", 0, 0, 0, 0, 0);
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_info = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetControllerInformation");

    s32 init_ret = pad_init ? (s32)NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0) : -1;
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    u8 *info = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)info == -1) info = 0;
    if (info) ex_zero(info, 0x1000);

    s32 info_ret = -1;
    if (pad_info && pad_h >= 0 && info) {
        info_ret = (s32)NC(ex.G, pad_info, (u64)pad_h, (u64)info, 0, 0, 0, 0);
    }

    ext->dbg[0] = (u64)(s64)pad_h;
    ext->dbg[1] = (u64)(s64)info_ret;
    ext->dbg[2] = info ? *(u32 *)(info + 0x00) : 0;
    ext->dbg[3] = info ? *(u32 *)(info + 0x04) : 0;
    ext->dbg[4] = info ? *(u32 *)(info + 0x08) : 0;
    ext->dbg[5] = info ? *(u32 *)(info + 0x0C) : 0;
    ext->dbg[6] = info ? *(u32 *)(info + 0x10) : 0;
    ext->dbg[7] = info ? *(u32 *)(info + 0x14) : 0;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (buf) {
        ex_dialog_begin(&ex);
        ret = ex_open_text(
            &ex,
            buf,
            "Pad info probe\nload=%x init=%x h=%x info=%x\nd0=%08x d1=%08x d2=%08x d3=%08x\nd4=%08x d5=%08x",
            (u32)load_ret,
            (u32)init_ret,
            (u32)pad_h,
            (u32)info_ret,
            info ? *(u32 *)(info + 0x00) : 0,
            info ? *(u32 *)(info + 0x04) : 0,
            info ? *(u32 *)(info + 0x08) : 0,
            info ? *(u32 *)(info + 0x0C) : 0,
            info ? *(u32 *)(info + 0x10) : 0,
            info ? *(u32 *)(info + 0x14) : 0
        );
        if (ret == 0) ex_wait_user_close(&ex);
        ex_close_dialog(&ex);
        ex_free_dialog(&ex, buf);
    }

    if (info && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)info, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
