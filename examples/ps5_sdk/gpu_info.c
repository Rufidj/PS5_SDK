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

    NC(ex.G, ex.load_mod, (u64)"libSceGnmDriver.sprx", 0, 0, 0, 0, 0);
    void *get_clock = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmGetGpuCoreClockFrequency");
    void *is_pa_enabled = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmIsUserPaEnabled");

    u64 clock_hz = ps5_sdk_gpu_get_core_clock_hz(ex.G, get_clock);
    s32 pa_enabled = ps5_sdk_gpu_is_user_pa_enabled(ex.G, is_pa_enabled);

    ext->dbg[0] = clock_hz;
    ext->dbg[1] = (u64)(s64)pa_enabled;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    if (get_clock && is_pa_enabled) {
        ret = ex_open_text(
            &ex,
            buf,
            "GPU info\ncoreClock=%u Hz\nuserPaEnabled=%d\nGnmDriver OK",
            (u32)clock_hz,
            (int)pa_enabled
        );
    } else {
        ret = ex_open_text(
            &ex,
            buf,
            "GPU info\nresolve clock=%p\nresolve userPa=%p",
            get_clock,
            is_pa_enabled
        );
    }
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
