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

    void *getpid_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_getpid);
    void *getuid_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_getuid);
    void *getgid_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_getgid);
    void *getegid_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_getegid);
    void *cpumode_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelGetCpumode);
    void *ptime_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelGetProcessTime);

    s64 pid = getpid_fn ? (s64)NC(ex.G, getpid_fn, 0, 0, 0, 0, 0, 0) : -1;
    s64 uid = getuid_fn ? (s64)NC(ex.G, getuid_fn, 0, 0, 0, 0, 0, 0) : -1;
    s64 gid = getgid_fn ? (s64)NC(ex.G, getgid_fn, 0, 0, 0, 0, 0, 0) : -1;
    s64 egid = getegid_fn ? (s64)NC(ex.G, getegid_fn, 0, 0, 0, 0, 0, 0) : -1;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    ret = ex_open_text(
        &ex,
        buf,
        "Kernel safe info\npid=%d uid=%d\ngid=%d egid=%d\ncpuFn=%p timeFn=%p",
        (int)pid,
        (int)uid,
        (int)gid,
        (int)egid,
        cpumode_fn,
        ptime_fn
    );
    ext->dbg[0] = (u64)(s64)ret;
    if (ret == 0) {
        ext->status = 0;
        ex_wait_user_close(&ex);
        ex_close_dialog(&ex);
    } else {
        ext->status = -4;
    }

    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
