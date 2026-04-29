#include "example_common.h"

/*
 * gnm_phys_fill — GPU DMA usando la direccion FISICA del framebuffer.
 *
 * gnm_onion_probe confirmo:
 *   - CB en ONION (type=1): GPU CP lo lee correctamente.
 *   - IT_WRITE_DATA DST_SEL=2: funciona, ambas fences dispararon.
 *   - IT_DMA_DATA se ejecuto (fence BEEF0002 disparo tras el DMA).
 *   - Pantalla negra: CPU VA del GARLIC FB NO es el GPU VA.
 *
 * Hipotesis: para memoria GARLIC (WC) el GPU VA == direccion fisica.
 * Este payload comprueba eso usando fb_phys como destino del DMA.
 *
 * Layout:
 *   CB: ONION type=1, 4 KB
 *   FB: GARLIC type=3, 2xFB_AL, F0 empieza en offset 0
 *
 * PM4 en CB:
 *   IT_DMA_DATA fill RED → GPU VA = fb_phys + 0
 *   IT_WRITE_DATA fence=0xC0DEC0DE
 *   IT_NOP
 */

enum {
    FW = 1920, FH = 1080,
    FB_SZ  = FW * FH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    CB_SZ  = 0x1000,
    FENCE_OFF = CB_SZ - 16,
};

#define PM4_NOP      0xC0001000u
#define PM4_WD_HDR   0xC0033700u
#define WD_TC_L2     0x00020000u
#define PM4_DMA_HDR  0xC0055000u
#define DMA_FILL     0xC0000000u

static s32 takeover_open(void *G, void *vid_open) {
    s32 types[4] = { 0xFF, 0, 1, 2 };
    for (int i = 0; i < 4; i++) {
        s32 h = (s32)NC(G, vid_open, (u64)types[i], 0, 0, 0, 0, 0);
        if (h >= 0) return h;
    }
    return -1;
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

    s32  sub_ret  = 0x7F, flip_ret = 0x7F;
    u32  fence_val = 0;
    u64  cb_phys = 0, fb_phys = 0;
    void *cb_mem = 0, *fb_mem = 0;

    if (!fn_submit || video_h < 0 || !alloc_dm || !map_dm || !vid_reg || !vid_flip)
        goto show;

    /* CB: ONION (type=1, WB coherent — GPU CP puede leerlo) */
    if ((s32)NC(ex.G, alloc_dm, 0, 0x100000000ULL, CB_SZ, 0x1000, 1, (u64)&cb_phys))
        goto show;
    NC(ex.G, map_dm, (u64)&cb_mem, CB_SZ, 0x33, 0, cb_phys, 0x1000);
    if (!cb_mem) goto show;

    /* FB: GARLIC (type=3, WC GPU-fast) */
    if ((s32)NC(ex.G, alloc_dm, 0, 0x300000000ULL, FB_AL * 2, 0x200000, 3, (u64)&fb_phys))
        goto show;
    NC(ex.G, map_dm, (u64)&fb_mem, FB_AL * 2, 0x33, 0, fb_phys, 0x200000);
    if (!fb_mem) goto show;

    {
        void *fbs[2] = { fb_mem, (u8 *)fb_mem + FB_AL };
        u8 attr[64]; ex_zero(attr, sizeof(attr));
        *(u32 *)(attr +  0) = 0x80000000;
        *(u32 *)(attr +  4) = 1;
        *(u32 *)(attr + 12) = FW;
        *(u32 *)(attr + 16) = FH;
        *(u32 *)(attr + 20) = FW;
        if ((s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0))
            goto show;
        if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

        volatile u32 *fence = (volatile u32 *)((u8 *)cb_mem + FENCE_OFF);
        *fence = 0u;

        u64 fence_va = (u64)(fence);
        /* GPU VA del framebuffer = direccion FISICA (para memoria GARLIC) */
        u64 fb0_gpu_va = fb_phys;
        u32 fbsz = FW * FH * 4u;

        u32 *pm4 = (u32 *)cb_mem;
        u32  n   = 0;

        /* DMA_DATA: fill RED usando direccion FISICA como GPU VA */
        pm4[n++] = PM4_DMA_HDR;
        pm4[n++] = DMA_FILL;
        pm4[n++] = 0x00FF0000u;              /* RED (0x00RRGGBB) */
        pm4[n++] = 0u;
        pm4[n++] = (u32)(fb0_gpu_va & 0xFFFFFFFFu);
        pm4[n++] = (u32)(fb0_gpu_va >> 32);
        pm4[n++] = fbsz;

        /* WRITE_DATA fence para confirmar que el DMA termino */
        pm4[n++] = PM4_WD_HDR;
        pm4[n++] = WD_TC_L2;
        pm4[n++] = (u32)(fence_va & 0xFFFFFFFFu);
        pm4[n++] = (u32)(fence_va >> 32);
        pm4[n++] = 0xC0DEC0DEu;

        pm4[n++] = PM4_NOP;

        u32  cb_bytes = n * 4u;
        void *cb_ptr  = (void *)cb_mem;

        sub_ret = (s32)NC(ex.G, fn_submit, 1,
                           (u64)&cb_ptr, (u64)&cb_bytes, 0, 0, 0);

        /* Spin-wait fence hasta 2s */
        for (u32 i = 0; i < 2000u && *fence != 0xC0DEC0DEu; i++) {
            if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1000, 0, 0, 0, 0, 0);
        }
        fence_val = *fence;

        flip_ret = (s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 3000000, 0, 0, 0, 0, 0);
    }

show:
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "gnm_phys_fill\n"
        "sub=%x  flip=%x\n"
        "fence=%x\n"
        "C0DEC0DE=DMA ok\n"
        "fb_phys=%lx\n"
        "cb_phys=%lx",
        (u32)sub_ret, (u32)flip_ret,
        fence_val, fb_phys, cb_phys);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
