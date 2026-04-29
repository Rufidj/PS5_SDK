#include "example_common.h"

/*
 * gnm_flush_test — como gnm_onion_probe pero con sceGnmFlushGarlic
 * DESPUES de que el fence confirma que el DMA termino.
 *
 * gnm_onion_probe confirmo:
 *   - fence=BEEF0002: DMA_DATA se ejecuto completamente (GPU escribio).
 *   - Pantalla negra: los datos siguen en la GPU L2 cache (no en DRAM).
 *   - sceGnmFlushGarlic DESPUES del fence debe forzar el flush a DRAM.
 *   - El display engine lee de DRAM → deberia verse ROJO.
 *
 * Si sigue negro con flush → no es cache, es otro problema.
 * Si ROJO → cache issue resuelto, tenemos GPU DMA funcional completo.
 *
 * Adicionalmente probamos FB en ONION (type=1) por si el problema
 * es el tipo de memoria del framebuffer (GARLIC vs ONION).
 *
 * Secuencia:
 *   A) CB=ONION + FB=GARLIC + flush despues del fence → flip RED
 *   B) CB=ONION + FB=ONION  + DMA via CPU VA         → flip GREEN
 *      (ONION es coherente, garantiza que DMA y display ven mismo dato)
 */

enum {
    FW = 1920, FH = 1080,
    FB_SZ  = FW * FH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    CB_SZ  = 0x1000,
    FENCE_OFF = CB_SZ - 16,
};

#define PM4_NOP     0xC0001000u
#define PM4_WD_HDR  0xC0033700u
#define WD_TC_L2    0x00020000u
#define PM4_DMA_HDR 0xC0055000u
#define DMA_FILL    0xC0000000u

static s32 takeover_open(void *G, void *vid_open) {
    s32 types[4] = { 0xFF, 0, 1, 2 };
    for (int i = 0; i < 4; i++) {
        s32 h = (s32)NC(G, vid_open, (u64)types[i], 0, 0, 0, 0, 0);
        if (h >= 0) return h;
    }
    return -1;
}

/* Build PM4: DMA_DATA fill + WRITE_DATA fence in ONION CB */
static u32 build_cb(u32 *pm4, u64 dst_va, u32 color,
                    u64 fence_va, u32 fence_val) {
    u32 n = 0;
    pm4[n++] = PM4_DMA_HDR;
    pm4[n++] = DMA_FILL;
    pm4[n++] = color;
    pm4[n++] = 0u;
    pm4[n++] = (u32)(dst_va & 0xFFFFFFFFu);
    pm4[n++] = (u32)(dst_va >> 32);
    pm4[n++] = FW * FH * 4u;
    pm4[n++] = PM4_WD_HDR;
    pm4[n++] = WD_TC_L2;
    pm4[n++] = (u32)(fence_va & 0xFFFFFFFFu);
    pm4[n++] = (u32)(fence_va >> 32);
    pm4[n++] = fence_val;
    pm4[n++] = PM4_NOP;
    return n * 4u;
}

static u32 wait_fence(void *G, void *usleep_fn,
                      volatile u32 *fence, u32 val, u32 ms) {
    for (u32 i = 0; i < ms; i++) {
        if (*fence == val) return 1u;
        if (usleep_fn) NC(G, usleep_fn, 1000, 0, 0, 0, 0, 0);
    }
    return 0u;
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceGnmDriver.sprx");
    void *fn_submit = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmSubmitCommandBuffers");
    void *fn_flush  = SYM(ex.G, ex.D, GNMDRIVER_HANDLE, "sceGnmFlushGarlic");

    void *cancel_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_scePthreadCancel);
    void *alloc_dm  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *vid_rate  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSetFlipRate");

    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
    if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);

    s32 video_h = vid_open ? takeover_open(ex.G, vid_open) : -1;

    u32 fence_a = 0, fence_b = 0;
    s32 sub_a = 0x7F, sub_b = 0x7F;
    s32 flip_a = 0x7F, flip_b = 0x7F;
    u64 cb_phys = 0, fga_phys = 0, fgb_phys = 0;
    void *cb_mem = 0, *fga_mem = 0, *fgb_mem = 0;

    if (!fn_submit || video_h < 0 || !alloc_dm || !map_dm || !vid_reg || !vid_flip)
        goto show;

    /* CB — ONION (type=1) */
    if ((s32)NC(ex.G, alloc_dm, 0, 0x100000000ULL, CB_SZ, 0x1000, 1, (u64)&cb_phys))
        goto show;
    NC(ex.G, map_dm, (u64)&cb_mem, CB_SZ, 0x33, 0, cb_phys, 0x1000);
    if (!cb_mem) goto show;

    /* FB_A — GARLIC (type=3): mismo que siempre */
    if ((s32)NC(ex.G, alloc_dm, 0, 0x300000000ULL, FB_AL * 2, 0x200000, 3, (u64)&fga_phys))
        goto show;
    NC(ex.G, map_dm, (u64)&fga_mem, FB_AL * 2, 0x33, 0, fga_phys, 0x200000);
    if (!fga_mem) goto show;

    /* FB_B — ONION (type=1): alternativa coherente */
    if ((s32)NC(ex.G, alloc_dm, 0, 0x100000000ULL, FB_AL * 2, 0x200000, 1, (u64)&fgb_phys))
        goto show;
    NC(ex.G, map_dm, (u64)&fgb_mem, FB_AL * 2, 0x33, 0, fgb_phys, 0x200000);
    if (!fgb_mem) goto show;

    {
        volatile u32 *fence = (volatile u32 *)((u8 *)cb_mem + FENCE_OFF);
        u64 fence_va = (u64)(fence);
        u32 *pm4 = (u32 *)cb_mem;

        /* ── A: GARLIC FB, CPU VA, + flush Garlic tras fence ─────────── */
        void *fbs_a[2] = { fga_mem, (u8 *)fga_mem + FB_AL };
        {
            u8 attr[64]; ex_zero(attr, sizeof(attr));
            *(u32 *)(attr +  0) = 0x80000000;
            *(u32 *)(attr +  4) = 1;
            *(u32 *)(attr + 12) = FW;
            *(u32 *)(attr + 16) = FH;
            *(u32 *)(attr + 20) = FW;
            NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs_a, 2, (u64)attr, 0);
        }
        if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

        *fence = 0u;
        u32 sz_a = build_cb(pm4, (u64)fbs_a[0], 0x00FF0000u /* RED */,
                            fence_va, 0xF00D0001u);
        void *cb_ptr = cb_mem;
        sub_a = (s32)NC(ex.G, fn_submit, 1, (u64)&cb_ptr, (u64)&sz_a, 0, 0, 0);
        wait_fence(ex.G, ex.usleep_fn, fence, 0xF00D0001u, 2000u);
        fence_a = *fence;

        /* FLUSH Garlic DESPUES del fence → fuerza GPU L2 a DRAM */
        if (fn_flush) NC(ex.G, fn_flush, 0, 0, 0, 0, 0, 0);

        flip_a = (s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 2000000, 0, 0, 0, 0, 0);

        /* ── B: ONION FB, CPU VA, sin flush necesario ─────────────────  */
        void *fbs_b[2] = { fgb_mem, (u8 *)fgb_mem + FB_AL };
        {
            u8 attr[64]; ex_zero(attr, sizeof(attr));
            *(u32 *)(attr +  0) = 0x80000000;
            *(u32 *)(attr +  4) = 1;
            *(u32 *)(attr + 12) = FW;
            *(u32 *)(attr + 16) = FH;
            *(u32 *)(attr + 20) = FW;
            NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs_b, 2, (u64)attr, 0);
        }
        *fence = 0u;
        u32 sz_b = build_cb(pm4, (u64)fbs_b[0], 0x0000FF00u /* GREEN */,
                            fence_va, 0xF00D0002u);
        sub_b = (s32)NC(ex.G, fn_submit, 1, (u64)&cb_ptr, (u64)&sz_b, 0, 0, 0);
        wait_fence(ex.G, ex.usleep_fn, fence, 0xF00D0002u, 2000u);
        fence_b = *fence;

        /* ONION es coherente, no necesita flush */
        flip_b = (s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 2000000, 0, 0, 0, 0, 0);
    }

show:
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "gnm_flush_test\n"
        "A(GARLIC+flush): sub=%x flip=%x\n"
        "  fence_A=%x\n"
        "B(ONION):        sub=%x flip=%x\n"
        "  fence_B=%x\n"
        "F00D0001=A ok  F00D0002=B ok",
        (u32)sub_a, (u32)flip_a, fence_a,
        (u32)sub_b, (u32)flip_b, fence_b);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
