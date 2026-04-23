#include "example_common.h"

#define SDK_AF_INET     2
#define SDK_SOCK_DGRAM  2

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

    s32 net_mod = (s32)NC(ex.G, ex.load_mod, (u64)"libSceNet.sprx", 0, 0, 0, 0, 0);
    void *socket_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "socket");
    void *close_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "close");
    void *sce_socket_fn = SYM(ex.G, ex.D, NET_HANDLE, "sceNetSocket");
    void *sce_close_fn = SYM(ex.G, ex.D, NET_HANDLE, "sceNetSocketClose");

    s32 fd = socket_fn
        ? (s32)NC(ex.G, socket_fn, SDK_AF_INET, SDK_SOCK_DGRAM, 0, 0, 0, 0)
        : -1;

    s32 close_ret = 0x7fffffff;
    if (close_fn && fd >= 0) {
        close_ret = (s32)NC(ex.G, close_fn, (u64)fd, 0, 0, 0, 0, 0);
    }

    s32 sce_fd = -1;
    s32 sce_close_ret = 0x7fffffff;
    if (sce_socket_fn) {
        sce_fd = (s32)NC(ex.G, sce_socket_fn, (u64)"sdk_udp_probe", SDK_AF_INET, SDK_SOCK_DGRAM, 0, 0, 0);
        if (sce_close_fn && sce_fd >= 0) {
            sce_close_ret = (s32)NC(ex.G, sce_close_fn, (u64)sce_fd, 0, 0, 0, 0, 0);
        }
    }

    ext->dbg[0] = (u64)(s64)fd;
    ext->dbg[1] = (u64)(s64)close_ret;
    ext->dbg[2] = (u64)(s64)sce_fd;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if (fd >= 0 || sce_fd >= 0) {
        ret = ex_open_text(
            &ex,
            buf,
            "Socket open probe\nnet=%x\nposix fd=%d close=%x\nsce fd=%d close=%x\nOpen/Close OK",
            (u32)net_mod,
            (int)fd,
            (u32)close_ret,
            (int)sce_fd,
            (u32)sce_close_ret
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "Socket open probe\nnet=%x\nposix=%x sce=%x\nsceNetSocket=%p\nNo socket opened",
            (u32)net_mod,
            (u32)fd,
            (u32)sce_fd,
            sce_socket_fn
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
