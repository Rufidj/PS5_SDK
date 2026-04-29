#include "example_common.h"

/*
 * gnm_dma_fill2 — diagnostico de direcciones GPU.
 *
 * Problematica: gnm_dma_fill solo coloreaba una banda arriba.
 * La GPU ejecuto el DMA (submit=0) pero solo parte del framebuffer
 * aparecio roja — la GPU VA que pasamos probablemente no coincide
 * exactamente con la CPU VA del framebuffer.
 *
 * Este payload hace DOS intentos con un fence PM4 entre medias:
 *
 *   Intento A — direccion CPU VA (igual que v1):
 *     IT_DMA_DATA fill  → RED  (0x00FF0000) al framebuffer via CPU VA
 *     IT_WRITE_DATA     → escribe 0xAAAA0001 a fence RAM
 *     CPU spin-wait     → confirma que la GPU ejecuto ambos packets
 *     flip + hold 1.5s
 *
 *   Intento B — direccion FISICA (phys + F0_OFF):
 *     IT_DMA_DATA fill  → GREEN (0x0000FF00) al framebuffer via phys addr
 *     IT_WRITE_DATA     → escribe 0xAAAA0002 a fence RAM
 *     CPU spin-wait
 *     flip + hold 1.5s
 *
 *   Dialog final: fence_A, fence_B, submit_A, submit_B, flip_A, flip_B
 *
 * IT_WRITE_DATA packet (opcode 0x37, GCN/RDNA2):
 *   header  = (3<<30)|(3<<16)|(0x37<<8) = 0xC0033700   [4 body dwords]
 *   control = 0x00050000  [DST_SEL=5 @ bits[20:16] = TC_L2]
 *   dst_lo, dst_hi
 *   value
 */

enum {
    FW = 1920, FH = 1080,
    FB_SZ  = FW * FH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    CB_OFF = 0,
    CB_SZ  = 0x2000,          /* 8 KB — room for both command buffers    */
    F0_OFF = CB_SZ,
    F1_OFF = CB_SZ + FB_AL,
    DM_TOT = CB_SZ + FB_AL * 2,
    DM_AL  = (DM_TOT + 0x1FFFFF) & ~0x1FFFFF,
    FENCE_OFF = CB_SZ - 16,   /* fence slot inside CB region             */
};

#define PM4_DMA_HDR   0xC0055000u  /* IT_DMA_DATA,   count-1=5           */
#define PM4_WD_HDR    0xC0033700u  /* IT_WRITE_DATA, count-1=3           */
#define PM4_NOP       0xC0001000u
#define DMA_FILL      0xC0000000u  /* ATTR0: CP_SYNC | SRC=DATA | DST=L2 */
#define WD_CTRL       0x00050000u  /* DST_SEL=5 @ [20:16] = TC_L2        */

static s32 takeover_open(void *G, void *vid_open) {
    s32 types[4] = { 0xFF, 0, 1, 2 };
    for (int i = 0; i < 4; i++) {
        s32 h = (s32)NC(G, vid_open, (u64)types[i], 0, 0, 0, 0, 0);
        if (h >= 0) return h;
    }
    return -1;
}

/* Build an 8-dword command buffer: DMA_DATA fill + WRITE_DATA fence + NOP */
static u32 build_cb(u32 *cb,
                    u64 dst_va, u32 fill_color, u32 byte_count,
                    u64 fence_va, u32 fence_val) {
    u32 n = 0;
    /* DMA_DATA fill */
    cb[n++] = PM4_DMA_HDR;
    cb[n++] = DMA_FILL;
    cb[n++] = fill_color;
    cb[n++] = 0u;
    cb[n++] = (u32)(dst_va  & 0xFFFFFFFFu);
    cb[n++] = (u32)(dst_va  >> 32);
    cb[n++] = byte_count;
    /* WRITE_DATA fence (CPU-visible, confirms GPU completion) */
    cb[n++] = PM4_WD_HDR;
    cb[n++] = WD_CTRL;
    cb[n++] = (u32)(fence_va & 0xFFFFFFFFu);
    cb[n++] = (u32)(fence_va >> 32);
    cb[n++] = fence_val;
    cb[n++] = PM4_NOP;
    return n * 4u;  /* byte count */
}

/* Wait up to timeout_ms for *fence to equal expected. Returns 1 on success. */
static u32 wait_fence(void *G, void *usleep_fn,
                      volatile u32 *fence, u32 expected, u32 timeout_ms) {
    for (u32 i = 0; i < timeout_ms; i++) {
        if (*fence == expected) return 1u;
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

    u32 sub_a = 0x7Fu, sub_b = 0x7Fu;
    u32 flip_a = 0x7Fu, flip_b = 0x7Fu;
    u32 fence_a = 0u,   fence_b = 0u;
    u64 phys = 0;
    void *mem = 0;

    if (!fn_submit || video_h < 0 || !alloc_dm || !map_dm || !vid_reg || !vid_flip)
        goto show_result;

    {
        s32 ar = (s32)NC(ex.G, alloc_dm,
                          0, 0x300000000ULL, DM_AL, 0x200000, 3, (u64)&phys);
        if (ar != 0) goto show_result;
        NC(ex.G, map_dm, (u64)&mem, DM_AL, 0x33, 0, phys, 0x200000);
        if (!mem) goto show_result;
    }

    {
        u8 *base = (u8 *)mem;

        void *fbs[2] = { base + F0_OFF, base + F1_OFF };
        u8 attr[64];
        ex_zero(attr, sizeof(attr));
        *(u32 *)(attr +  0) = 0x80000000;
        *(u32 *)(attr +  4) = 1;
        *(u32 *)(attr + 12) = FW;
        *(u32 *)(attr + 16) = FH;
        *(u32 *)(attr + 20) = FW;

        if ((s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0) != 0)
            goto show_result;
        if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

        u32 fb_bytes = FW * FH * 4u;
        volatile u32 *fence_ptr = (volatile u32 *)(base + FENCE_OFF);
        u64 fence_va = (u64)(base + FENCE_OFF);  /* CPU VA for fence     */

        /* ── Intento A: direccion CPU VA ─────────────────────────────── */
        *fence_ptr = 0u;
        u64 fb0_cpu_va = (u64)fbs[0];
        u32 *cb = (u32 *)base;
        u32 cb_sz = build_cb(cb, fb0_cpu_va, 0x00FF0000u /* RED */,
                             fb_bytes, fence_va, 0xAAAA0001u);
        void *cb_ptr = (void *)cb;
        sub_a = (u32)(s32)NC(ex.G, fn_submit, 1,
                              (u64)&cb_ptr, (u64)&cb_sz, 0, 0, 0);
        if (fn_flush) NC(ex.G, fn_flush, 0, 0, 0, 0, 0, 0);
        wait_fence(ex.G, ex.usleep_fn, fence_ptr, 0xAAAA0001u, 1000u);
        fence_a = *fence_ptr;
        flip_a = (u32)(s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1500000, 0, 0, 0, 0, 0);

        /* ── Intento B: direccion FISICA ─────────────────────────────── */
        *fence_ptr = 0u;
        u64 fb0_phys_va = phys + (u64)F0_OFF;   /* physical address      */
        cb_sz = build_cb(cb, fb0_phys_va, 0x0000FF00u /* GREEN */,
                         fb_bytes, fence_va, 0xAAAA0002u);
        sub_b = (u32)(s32)NC(ex.G, fn_submit, 1,
                              (u64)&cb_ptr, (u64)&cb_sz, 0, 0, 0);
        if (fn_flush) NC(ex.G, fn_flush, 0, 0, 0, 0, 0, 0);
        wait_fence(ex.G, ex.usleep_fn, fence_ptr, 0xAAAA0002u, 1000u);
        fence_b = *fence_ptr;
        flip_b = (u32)(s32)NC(ex.G, vid_flip, (u64)video_h, 0, 1, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 1500000, 0, 0, 0, 0, 0);
    }

show_result:
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);

    ext->dbg[0] = ((u64)sub_a << 32)  | sub_b;
    ext->dbg[1] = ((u64)flip_a << 32) | flip_b;
    ext->dbg[2] = fence_a;
    ext->dbg[3] = fence_b;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "gnm_dma_fill2\n"
        "sub_A(cpuva)=%x  sub_B(phys)=%x\n"
        "flip_A=%x  flip_B=%x\n"
        "fence_A=%x\n"
        "fence_B=%x\n"
        "fence=AAAAx001 -> GPU ok\n"
        "full RED=cpuva ok\n"
        "full GREEN=phys ok",
        sub_a, sub_b, flip_a, flip_b,
        fence_a, fence_b);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
