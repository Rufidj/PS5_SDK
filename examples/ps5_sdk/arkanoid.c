#include "example_common.h"

/* ── Layout ─────────────────────────────────────────────────────────────── */
enum {
    GW = 1920, GH = 1080,
    FB_SZ  = GW * GH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    FB_TOT = FB_AL * 2,
    HUD_H  = 80,    /* top HUD bar height (px) */
    FSC    = 3,     /* default font scale */

    /* Bricks: 10 cols × 6 rows, 150×34 px, 4 px gap */
    BC = 10, BR = 6,
    BW = 150, BH = 34, BG = 4,
    BX0 = 192,   /* (1920 - 10*150 - 9*4) / 2 */
    BY0 = 110,

    /* Paddle */
    PW = 200, PH = 20,
    PY = 970, PSPD = 14,

    /* Ball */
    BSIZE = 16,

    /* Audio grain */
    GRAIN = 256,
};

static const u32 row_color[BR] = {
    0x00ff3030, 0x00ff8830, 0x00ffee30,
    0x0030ee30, 0x003090ff, 0x00bb30ff,
};

/* ── Game state ──────────────────────────────────────────────────────────── */
enum { ST_WAIT = 0, ST_PLAY = 1, ST_WIN = 2, ST_LOSE = 3 };

struct game {
    u8  br[BC * BR];
    s32 px, bx, by, bvx, bvy;
    s32 score, lives, left;
    u32 state;
};

static void game_reset(struct game *g, u32 full) {
    if (full) { g->score = 0; g->lives = 3; g->left = BC * BR;
                for (int i = 0; i < BC * BR; i++) g->br[i] = 1; }
    g->px  = (GW - PW) / 2;
    g->bvx = 5; g->bvy = -7;
    g->bx  = g->px + (PW - BSIZE) / 2;
    g->by  = PY - BSIZE - 2;
    g->state = ST_WAIT;
}

/* ── Audio helpers ───────────────────────────────────────────────────────── */
struct audio {
    void *G, *out_fn;
    s32   port_h, phase;
    s16  *buf;
    u32   ok;
};

static void audio_init(struct audio *a, struct ps5_example *ex) {
    a->G = ex->G; a->ok = 0;
    ps5_sdk_load_sprx(ex->G, ex->load_mod, "libSceAudioOut.sprx");
    void *ao_init = SYM(ex->G, ex->D, AUDIOOUT_HANDLE, "sceAudioOutInit");
    void *ao_open = SYM(ex->G, ex->D, AUDIOOUT_HANDLE, "sceAudioOutOpen");
    a->out_fn     = SYM(ex->G, ex->D, AUDIOOUT_HANDLE, "sceAudioOutOutput");
    if (!ao_init || !ao_open || !a->out_fn) return;
    NC(ex->G, ao_init, 0, 0, 0, 0, 0, 0);
    a->port_h = (s32)NC(ex->G, ao_open,
                        (u64)ex->user_id, 1, 0, GRAIN, 48000, 1);
    if (a->port_h < 0) return;
    a->buf = (s16 *)NC(ex->G, ex->mmap_fn, 0, 4096, 3, 0x1002, (u64)-1, 0);
    if ((s64)a->buf == -1) { a->buf = 0; return; }
    a->phase = 0; a->ok = 1;
}

static void audio_beep(struct audio *a, s32 freq, s32 grains) {
    if (!a->ok || !a->buf || grains <= 0) return;
    s32 period = (freq > 0) ? (48000 / freq) : 1;
    for (s32 g = 0; g < grains; g++) {
        for (s32 i = 0; i < GRAIN; i++) {
            s16 v = ((a->phase / period) & 1) ? 6000 : -6000;
            a->buf[i * 2]     = v;
            a->buf[i * 2 + 1] = v;
            a->phase++;
        }
        NC(a->G, a->out_fn, (u64)a->port_h, (u64)a->buf, 0, 0, 0, 0);
    }
}

static void audio_close(struct audio *a, struct ps5_example *ex) {
    if (!a->ok) return;
    void *ao_close = SYM(ex->G, ex->D, AUDIOOUT_HANDLE, "sceAudioOutClose");
    if (ao_close) NC(ex->G, ao_close, (u64)a->port_h, 0, 0, 0, 0, 0);
    if (a->buf && ex->munmap_fn)
        NC(ex->G, ex->munmap_fn, (u64)a->buf, 4096, 0, 0, 0, 0);
    a->ok = 0;
}

/* ── Rendering ───────────────────────────────────────────────────────────── */
static void draw(u32 *fb, struct game *g) {
    ps5sdk_fb_fill(fb, GW, GH, 0x00060614);
    ps5sdk_fb_rect(fb, GW, GH, 0, HUD_H - 2, GW, 2, 0x00282840);

    /* title centered */
    ps5sdk_fb_str(fb, GW, GH, 864, 20, "ARKANOID", 0x00ffee40, 0, FSC, 1);

    /* score / lives */
    ps5sdk_fb_str(fb, GW, GH,  20, 20, "SCORE:", 0x00c0c0c0, 0, FSC, 1);
    ps5sdk_fb_dec(fb, GW, GH, 164, 20, (u32)g->score,  0x00ffff40, 0, FSC, 1);
    ps5sdk_fb_str(fb, GW, GH, 1584, 20, "LIVES:", 0x00c0c0c0, 0, FSC, 1);
    ps5sdk_fb_dec(fb, GW, GH, 1728, 20,
                  (u32)(g->lives > 0 ? g->lives : 0), 0x00ff8040, 0, FSC, 1);

    /* bricks */
    for (int r = 0; r < BR; r++) {
        for (int c = 0; c < BC; c++) {
            if (!g->br[r * BC + c]) continue;
            s32 x = BX0 + c * (BW + BG);
            s32 y = BY0 + r * (BH + BG);
            ps5sdk_fb_rect(fb, GW, GH, x,   y,   BW,   BH,   0x00101018);
            ps5sdk_fb_rect(fb, GW, GH, x+2, y+2, BW-4, BH-4, row_color[r]);
        }
    }

    /* paddle */
    ps5sdk_fb_rect(fb, GW, GH, g->px,   PY,   PW,   PH,   0x001040a0);
    ps5sdk_fb_rect(fb, GW, GH, g->px+2, PY+2, PW-4, PH-4, 0x0050a0ff);

    /* ball */
    ps5sdk_fb_rect(fb, GW, GH, g->bx,   g->by,   BSIZE,   BSIZE,   0x00ffffff);
    ps5sdk_fb_rect(fb, GW, GH, g->bx+4, g->by+4, BSIZE-8, BSIZE-8, 0x00a0a0c0);

    /* overlays */
    if (g->state == ST_WAIT)
        ps5sdk_fb_str(fb, GW, GH, 780, GH/2,
                      "CROSS TO LAUNCH", 0x00ffffff, 0, FSC, 1);
    if (g->state == ST_WIN) {
        ps5sdk_fb_str(fb, GW, GH, 832, GH/2 - 48,
                      "YOU WIN!", 0x00ffff00, 0, 4, 1);
        ps5sdk_fb_str(fb, GW, GH, 768, GH/2 + 48,
                      "CROSS TO RESTART", 0x00ffffff, 0, FSC, 1);
    }
    if (g->state == ST_LOSE) {
        ps5sdk_fb_str(fb, GW, GH, 816, GH/2 - 48,
                      "GAME OVER", 0x00ff4040, 0, 4, 1);
        ps5sdk_fb_str(fb, GW, GH, 768, GH/2 + 48,
                      "CROSS TO RESTART", 0x00ffffff, 0, FSC, 1);
    }
}

/* ── Physics ─────────────────────────────────────────────────────────────── */
static u32 physics(struct game *g, u32 buttons, struct audio *a) {
    u32 event = 0;  /* bit0=brick, bit1=paddle, bit2=lost */

    /* paddle */
    if (buttons & PS5SDK_PAD_BTN_LEFT)  g->px -= PSPD;
    if (buttons & PS5SDK_PAD_BTN_RIGHT) g->px += PSPD;
    if (g->px < 0)       g->px = 0;
    if (g->px + PW > GW) g->px = GW - PW;

    if (g->state != ST_PLAY) {
        if (g->state == ST_WAIT) {
            g->bx = g->px + (PW - BSIZE) / 2;
            g->by = PY - BSIZE - 2;
        }
        return 0;
    }

    /* move */
    g->bx += g->bvx;
    g->by += g->bvy;

    /* wall bounces */
    if (g->bx < 0)             { g->bx = 0;           g->bvx = -g->bvx; }
    if (g->bx + BSIZE > GW)    { g->bx = GW - BSIZE;  g->bvx = -g->bvx; }
    if (g->by < HUD_H)         { g->by = HUD_H;        g->bvy = -g->bvy; }

    /* paddle bounce */
    if (g->bvy > 0 &&
        g->by + BSIZE >= PY && g->by < PY + PH &&
        g->bx + BSIZE > g->px && g->bx < g->px + PW) {
        s32 co = (g->bx + BSIZE/2) - (g->px + PW/2);
        g->bvx = co / 18;
        if (g->bvx >  7) g->bvx =  7;
        if (g->bvx < -7) g->bvx = -7;
        if (g->bvx == 0) g->bvx = (co >= 0) ? 2 : -2;
        g->bvy = -7;
        g->by  = PY - BSIZE - 1;
        event |= 2;
    }

    /* bottom: lose life */
    if (g->by > GH) {
        g->lives--;
        event |= 4;
        if (g->lives <= 0) g->state = ST_LOSE;
        else { game_reset(g, 0); }
        return event;
    }

    /* brick collision (one per frame) */
    s32 hit = 0;
    for (int r = 0; r < BR && !hit; r++) {
        for (int c = 0; c < BC && !hit; c++) {
            if (!g->br[r * BC + c]) continue;
            s32 rx = BX0 + c * (BW + BG);
            s32 ry = BY0 + r * (BH + BG);
            if (g->bx + BSIZE <= rx || g->bx >= rx + BW) continue;
            if (g->by + BSIZE <= ry || g->by >= ry + BH) continue;

            g->br[r * BC + c] = 0;
            g->left--;
            g->score += 10 + (BR - 1 - r) * 5;
            event |= 1;

            /* bounce on shortest penetration axis */
            s32 ol = (g->bx + BSIZE) - rx,  or2 = (rx + BW) - g->bx;
            s32 ot = (g->by + BSIZE) - ry,  ob  = (ry + BH) - g->by;
            s32 mh = ol < or2 ? ol : or2;
            s32 mv = ot < ob  ? ot : ob;
            if (mh < mv) g->bvx = -g->bvx; else g->bvy = -g->bvy;

            if (g->left == 0) g->state = ST_WIN;
            hit = 1;
        }
    }

    (void)a;
    return event;
}

/* ── Entry point ─────────────────────────────────────────────────────────── */
__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    /* pad */
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libScePad.sprx");
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_read = SYM(ex.G, ex.D, PAD_HANDLE, "scePadRead");
    if (pad_init) NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0);
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    /* audio */
    struct audio snd;
    audio_init(&snd, &ex);

    /* videoout takeover */
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

    s32 video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    u64 phys = 0; void *vmem = 0;
    u8 *pad_buf = 0;
    s32 final_score = 0;

    if (pad_h >= 0 && ex.mmap_fn) {
        pad_buf = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
        if ((s64)pad_buf == -1) pad_buf = 0;
    }

    if (video_h >= 0 && alloc_dm && map_dm && vid_reg && vid_flip) {
        NC(ex.G, alloc_dm, 0, 0x300000000ULL, FB_TOT, 0x200000, 3, (u64)&phys);
        NC(ex.G, map_dm, (u64)&vmem, FB_TOT, 0x33, 0, phys, 0x200000);
        if (vmem) {
            u8 attr[64]; void *fbs[2];
            ex_zero(attr, sizeof(attr));
            *(u32 *)(attr +  0) = 0x80000000;
            *(u32 *)(attr +  4) = 1;
            *(u32 *)(attr + 12) = GW;
            *(u32 *)(attr + 16) = GH;
            *(u32 *)(attr + 20) = GW;
            fbs[0] = vmem;
            fbs[1] = (u8 *)vmem + FB_AL;

            s32 rr = (s32)NC(ex.G, vid_reg, (u64)video_h, 0, (u64)fbs, 2, (u64)attr, 0);
            if (rr == 0) {
                if (vid_rate) NC(ex.G, vid_rate, (u64)video_h, 0, 0, 0, 0, 0);

                struct game g;
                game_reset(&g, 1);

                u32 frame = 0, prev_btns = 0, done = 0;

                while (!done) {
                    u32 raw = 0, btns = 0;
                    if (pad_read && pad_h >= 0 && pad_buf)
                        ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &raw, &btns);

                    u32 pressed = btns & ~prev_btns;
                    prev_btns = btns;

                    if (pressed & PS5SDK_PAD_BTN_OPTIONS) done = 1;
                    if (pressed & PS5SDK_PAD_BTN_CROSS) {
                        if (g.state == ST_WIN || g.state == ST_LOSE)
                            game_reset(&g, 1);
                        else if (g.state == ST_WAIT)
                            g.state = ST_PLAY;
                    }

                    u32 ev = physics(&g, btns, &snd);

                    /* sound on events */
                    if (ev & 1) audio_beep(&snd, 660, 1);   /* brick */
                    if (ev & 2) audio_beep(&snd, 880, 2);   /* paddle */
                    if (ev & 4) audio_beep(&snd, 220, 5);   /* life lost */

                    s32 idx = (s32)(frame & 1u);
                    draw((u32 *)fbs[idx], &g);

                    NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);
                    frame++;
                }
                final_score = g.score;
            }
        }
    }

    audio_close(&snd, &ex);
    if (pad_buf && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)pad_buf, 0x1000, 0, 0, 0, 0);
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "Arkanoid\nScore: %d\naudio: %s",
        final_score, snd.ok ? "on" : "off");
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
}
