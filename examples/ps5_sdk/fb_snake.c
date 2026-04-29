#include "example_common.h"

/* ── Layout ─────────────────────────────────────────────────────────────── */
enum {
    GW = 1920, GH = 1080,
    FB_SZ  = GW * GH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    FB_TOT = FB_AL * 2,
    HUD_H  = 80,
    FSC    = 3,

    /* Grid for snake */
    BS = 30,             /* Block size */
    GC = GW / BS,        /* Grid columns: 64 */
    GR = (GH - HUD_H) / BS, /* Grid rows: 33 */

    /* Maximum snake length */
    MAX_SNAKE = GC * GR,
};

/* ── Game state ──────────────────────────────────────────────────────────── */
enum { ST_WAIT = 0, ST_PLAY = 1, ST_LOSE = 2 };
enum { DIR_UP = 0, DIR_RIGHT = 1, DIR_DOWN = 2, DIR_LEFT = 3 };

struct point {
    s32 x, y;
};

struct game {
    struct point s[MAX_SNAKE]; /* snake body */
    s32 len;
    s32 dir, next_dir;
    struct point apple;
    s32 score;
    u32 state;
    s32 speed_ticks, tick_count;
    u32 seed;
};

static void spawn_apple(struct game *g, struct ps5_example *ex) {
    (void)ex;
    for (;;) {
        g->seed = g->seed * 1103515245 + 12345;
        g->apple.x = (g->seed >> 16) % GC;
        g->seed = g->seed * 1103515245 + 12345;
        g->apple.y = (g->seed >> 16) % GR;
        
        /* Check if apple is on snake */
        s32 on_snake = 0;
        for (s32 i = 0; i < g->len; i++) {
            if (g->s[i].x == g->apple.x && g->s[i].y == g->apple.y) {
                on_snake = 1;
                break;
            }
        }
        if (!on_snake) break;
    }
}

static void game_reset(struct game *g, struct ps5_example *ex) {
    g->len = 3;
    g->s[0].x = GC / 2; g->s[0].y = GR / 2;
    g->s[1].x = g->s[0].x - 1; g->s[1].y = g->s[0].y;
    g->s[2].x = g->s[0].x - 2; g->s[2].y = g->s[0].y;
    g->dir = DIR_RIGHT;
    g->next_dir = DIR_RIGHT;
    g->score = 0;
    g->speed_ticks = 8;
    g->tick_count = 0;
    g->state = ST_WAIT;
    g->seed = 12345;
    spawn_apple(g, ex);
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
                        (u64)ex->user_id, 1, 0, 256, 48000, 1);
    if (a->port_h < 0) return;
    a->buf = (s16 *)NC(ex->G, ex->mmap_fn, 0, 4096, 3, 0x1002, (u64)-1, 0);
    if ((s64)a->buf == -1) { a->buf = 0; return; }
    a->phase = 0; a->ok = 1;
}

static void audio_beep(struct audio *a, s32 freq, s32 grains) {
    if (!a->ok || !a->buf || grains <= 0) return;
    s32 period = (freq > 0) ? (48000 / freq) : 1;
    for (s32 g = 0; g < grains; g++) {
        for (s32 i = 0; i < 256; i++) {
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
    ps5sdk_fb_fill(fb, GW, GH, 0x00061406); /* dark green background */
    ps5sdk_fb_rect(fb, GW, GH, 0, HUD_H - 2, GW, 2, 0x00284028);

    /* title centered */
    ps5sdk_fb_str(fb, GW, GH, 864, 20, "SNAKE PS5", 0x0040ee40, 0, FSC, 1);

    /* score */
    ps5sdk_fb_str(fb, GW, GH,  20, 20, "SCORE:", 0x00c0c0c0, 0, FSC, 1);
    ps5sdk_fb_dec(fb, GW, GH, 164, 20, (u32)g->score,  0x00ffff40, 0, FSC, 1);

    /* draw apple */
    s32 ax = g->apple.x * BS;
    s32 ay = HUD_H + g->apple.y * BS;
    ps5sdk_fb_rect(fb, GW, GH, ax + 2, ay + 2, BS - 4, BS - 4, 0x00ff3030);

    /* draw snake */
    for (s32 i = 0; i < g->len; i++) {
        s32 sx = g->s[i].x * BS;
        s32 sy = HUD_H + g->s[i].y * BS;
        u32 color = (i == 0) ? 0x0080ff80 : 0x0030a030;
        ps5sdk_fb_rect(fb, GW, GH, sx + 1, sy + 1, BS - 2, BS - 2, color);
    }

    /* overlays */
    if (g->state == ST_WAIT)
        ps5sdk_fb_str(fb, GW, GH, 780, GH/2, "CROSS TO LAUNCH", 0x00ffffff, 0, FSC, 1);
    if (g->state == ST_LOSE) {
        ps5sdk_fb_str(fb, GW, GH, 816, GH/2 - 48, "GAME OVER", 0x00ff4040, 0, 4, 1);
        ps5sdk_fb_str(fb, GW, GH, 768, GH/2 + 48, "CROSS TO RESTART", 0x00ffffff, 0, FSC, 1);
    }
}

/* ── Physics ─────────────────────────────────────────────────────────────── */
static u32 physics(struct game *g, u32 pressed, struct audio *a, struct ps5_example *ex) {
    u32 event = 0;  /* bit0=apple, bit1=die */

    if (g->state != ST_PLAY) return 0;

    /* handle input */
    if ((pressed & PS5SDK_PAD_BTN_UP) && g->dir != DIR_DOWN) g->next_dir = DIR_UP;
    if ((pressed & PS5SDK_PAD_BTN_DOWN) && g->dir != DIR_UP) g->next_dir = DIR_DOWN;
    if ((pressed & PS5SDK_PAD_BTN_LEFT) && g->dir != DIR_RIGHT) g->next_dir = DIR_LEFT;
    if ((pressed & PS5SDK_PAD_BTN_RIGHT) && g->dir != DIR_LEFT) g->next_dir = DIR_RIGHT;

    g->tick_count++;
    if (g->tick_count < g->speed_ticks) return 0;
    g->tick_count = 0;

    g->dir = g->next_dir;

    struct point next_head = g->s[0];
    if (g->dir == DIR_UP) next_head.y--;
    else if (g->dir == DIR_DOWN) next_head.y++;
    else if (g->dir == DIR_LEFT) next_head.x--;
    else if (g->dir == DIR_RIGHT) next_head.x++;

    /* check wall collision */
    if (next_head.x < 0 || next_head.x >= GC || next_head.y < 0 || next_head.y >= GR) {
        g->state = ST_LOSE;
        return 2;
    }

    /* check self collision */
    for (s32 i = 0; i < g->len - 1; i++) {
        if (next_head.x == g->s[i].x && next_head.y == g->s[i].y) {
            g->state = ST_LOSE;
            return 2;
        }
    }

    /* move body */
    if (next_head.x == g->apple.x && next_head.y == g->apple.y) {
        g->score += 10;
        event |= 1;
        if (g->len < MAX_SNAKE) g->len++;
        if (g->score % 50 == 0 && g->speed_ticks > 2) g->speed_ticks--;
        spawn_apple(g, ex);
    }

    for (s32 i = g->len - 1; i > 0; i--) {
        g->s[i] = g->s[i - 1];
    }
    g->s[0] = next_head;

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
    void *rel_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelReleaseDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");
    void *vid_rate  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSetFlipRate");

    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    s32 video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    if (video_h < 0) {
        if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
        if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);
        video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    }
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

                struct game *g = (struct game *)NC(ex.G, ex.mmap_fn, 0, 0x8000, 3, 0x1002, (u64)-1, 0);
                if ((s64)g != -1) {
                    game_reset(g, &ex);

                    u32 frame = 0, prev_btns = 0, done = 0;

                    while (!done) {
                        u32 raw = 0, btns = 0;
                        if (pad_read && pad_h >= 0 && pad_buf)
                            ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &raw, &btns);

                        u32 pressed = btns & ~prev_btns;
                        prev_btns = btns;

                        if (pressed & PS5SDK_PAD_BTN_OPTIONS) done = 1;
                        if (pressed & PS5SDK_PAD_BTN_CROSS) {
                            if (g->state == ST_LOSE)
                                game_reset(g, &ex);
                            else if (g->state == ST_WAIT)
                                g->state = ST_PLAY;
                        }

                        u32 ev = physics(g, pressed, &snd, &ex);

                        /* sound on events */
                        if (ev & 1) audio_beep(&snd, 880, 2);   /* eat apple */
                        if (ev & 2) audio_beep(&snd, 220, 5);   /* die */

                        s32 idx = (s32)(frame & 1u);
                        draw((u32 *)fbs[idx], g);

                        NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);
                        frame++;
                    }
                    final_score = g->score;
                    NC(ex.G, ex.munmap_fn, (u64)g, 0x8000, 0, 0, 0, 0);
                }
            }
        }
    }

    audio_close(&snd, &ex);
    if (pad_buf && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)pad_buf, 0x1000, 0, 0, 0, 0);
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    if (vmem && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)vmem, FB_TOT, 0, 0, 0, 0);
    if (rel_dm && phys) NC(ex.G, rel_dm, phys, FB_TOT, 0, 0, 0, 0);

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) { ext->status = -3; return; }
    ex_dialog_begin(&ex);
    ret = ex_open_text(&ex, buf,
        "Snake\nScore: %d\naudio: %s",
        final_score, snd.ok ? "on" : "off");
    if (ret == 0) {
        ext->status = 0;
        ex_wait_user_close(&ex);
    } else {
        ext->status = -4;
    }
    ex_close_dialog(&ex);
    ex_free_dialog(&ex, buf);
    ext->step = 99;
    ex_finish(&ex, ext, ext->status, 0);
}
