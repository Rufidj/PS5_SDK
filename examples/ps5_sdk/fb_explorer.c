#include "example_common.h"

#define O_RDONLY    0x0000
#define DT_DIR      4
#define DT_REG      8

enum {
    GW = 1920, GH = 1080,
    FB_SZ  = GW * GH * 4,
    FB_AL  = (FB_SZ + 0x1FFFFF) & ~0x1FFFFF,
    FB_TOT = FB_AL * 2,
    HUD_H  = 80,
    FSC    = 3,
    LIST_Y = 100,
    LIST_STEP = 36,
    MAX_FILES = 512
};

struct file_entry {
    char name[64];
    u8 is_dir;
};

struct explorer_state {
    char cwd[512];
    struct file_entry files[MAX_FILES];
    s32 count;
    s32 cursor;
    s32 scroll;
};

static s32 str_len(const char *s) {
    s32 n = 0;
    while (s && s[n]) n++;
    return n;
}

static void str_cpy(char *d, const char *s) {
    while (*s) *d++ = *s++;
    *d = 0;
}

static s32 str_cmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void path_up(char *path) {
    s32 len = str_len(path);
    if (len <= 1) return;
    if (path[len - 1] == '/') {
        len--;
        path[len] = 0;
    }
    while (len > 0 && path[len - 1] != '/') {
        len--;
    }
    if (len <= 1) {
        path[0] = '/';
        path[1] = 0;
    } else {
        path[len - 1] = 0;
    }
}

static void path_append(char *path, const char *name) {
    s32 len = str_len(path);
    if (len > 0 && path[len - 1] != '/') {
        path[len++] = '/';
    }
    while (*name && len < 510) {
        path[len++] = *name++;
    }
    path[len] = 0;
}

static void swap_entries(struct file_entry *a, struct file_entry *b) {
    struct file_entry tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sort_files(struct explorer_state *st) {
    for (s32 i = 0; i < st->count - 1; i++) {
        for (s32 j = 0; j < st->count - i - 1; j++) {
            s32 swap = 0;
            if (str_cmp(st->files[j+1].name, "..") == 0) {
                swap = 1;
            } else if (str_cmp(st->files[j].name, "..") == 0) {
                swap = 0;
            } else if (st->files[j].is_dir != st->files[j+1].is_dir) {
                if (!st->files[j].is_dir && st->files[j+1].is_dir) swap = 1;
            } else {
                if (str_cmp(st->files[j].name, st->files[j+1].name) > 0) swap = 1;
            }
            if (swap) swap_entries(&st->files[j], &st->files[j+1]);
        }
    }
}

static void load_dir(struct explorer_state *st, struct ps5_example *ex, void *kopen, void *getdents, void *kclose) {
    st->count = 0;
    st->cursor = 0;
    st->scroll = 0;

    s32 fd = (s32)NC(ex->G, kopen, (u64)st->cwd, O_RDONLY, 0, 0, 0, 0);
    if (fd < 0) {
        str_cpy(st->files[0].name, "<ACCESS DENIED>");
        st->files[0].is_dir = 0;
        st->count = 1;
        return;
    }

    u8 *db = (u8 *)NC(ex->G, ex->mmap_fn, 0, 0x8000, 3, 0x1002, (u64)-1, 0);
    if ((s64)db != -1) {
        s32 n = (s32)NC(ex->G, getdents, (u64)fd, (u64)db, 0x8000, 0, 0, 0);
        if (n > 0) {
            s32 off = 0;
            while (off < n && st->count < MAX_FILES) {
                u16 reclen = *(u16*)(db + off + 4);
                u8 type = *(u8*)(db + off + 6);
                char *name = (char*)(db + off + 8);

                if (reclen == 0) break;
                if (!(name[0] == '.' && name[1] == 0)) { /* only skip '.' */
                    st->files[st->count].is_dir = (type == DT_DIR);
                    s32 k = 0;
                    while (name[k] && k < 63) {
                        st->files[st->count].name[k] = name[k];
                        k++;
                    }
                    st->files[st->count].name[k] = 0;
                    st->count++;
                }
                off += reclen;
            }
        }
        NC(ex->G, ex->munmap_fn, (u64)db, 0x8000, 0, 0, 0, 0);
    }
    NC(ex->G, kclose, (u64)fd, 0, 0, 0, 0, 0);

    if (st->count == 0) {
        str_cpy(st->files[0].name, "<EMPTY DIR>");
        st->files[0].is_dir = 0;
        st->count = 1;
    } else {
        sort_files(st);
    }
}

static void draw(u32 *fb, struct explorer_state *st) {
    /* Base background */
    ps5sdk_fb_fill(fb, GW, GH, 0x000D0E15);

    /* Top Header Area */
    ps5sdk_fb_rect(fb, GW, GH, 0, 0, GW, 140, 0x00151822);
    /* Gradient-like accent line (2 rects) */
    ps5sdk_fb_rect(fb, GW, GH, 0, 140, GW/2, 3, 0x00FF2A6D);
    ps5sdk_fb_rect(fb, GW, GH, GW/2, 140, GW/2, 3, 0x0005D9E8);

    /* Header Title */
    ps5sdk_fb_str(fb, GW, GH, 40, 35, "PS5 SYSTEM EXPLORER", 0x0005D9E8, 0, 4, 1);
    
    /* Path Container */
    ps5sdk_fb_rect(fb, GW, GH, 40, 90, GW - 80, 36, 0x00202433);
    ps5sdk_fb_str(fb, GW, GH, 50, 98, st->cwd, 0x00E0E0E0, 0, 2, 1);

    /* Main List Container */
    s32 list_bg_y = 170;
    s32 list_bg_h = GH - list_bg_y - 60;
    ps5sdk_fb_rect(fb, GW, GH, 40, list_bg_y, GW - 80, list_bg_h, 0x0011131A);

    /* Footer */
    ps5sdk_fb_rect(fb, GW, GH, 0, GH - 40, GW, 40, 0x00151822);
    ps5sdk_fb_str(fb, GW, GH, 40, GH - 30, "UP/DOWN: Navigate   CROSS: Enter   CIRCLE: Back   OPTIONS: Exit", 0x008A95A5, 0, 2, 1);

    s32 max_visible = (list_bg_h - 20) / LIST_STEP;
    for (s32 i = 0; i < max_visible && (st->scroll + i) < st->count; i++) {
        s32 idx = st->scroll + i;
        s32 y = list_bg_y + 10 + i * LIST_STEP;
        
        u32 color = st->files[idx].is_dir ? 0x0005D9E8 : 0x00A0AABF;
        if (str_cmp(st->files[idx].name, "<ACCESS DENIED>") == 0 || str_cmp(st->files[idx].name, "<EMPTY DIR>") == 0) {
            color = 0x00FF2A6D;
        }

        if (idx == st->cursor) {
            /* Highlight row */
            ps5sdk_fb_rect(fb, GW, GH, 45, y - 4, GW - 90, LIST_STEP, 0x00293145);
            color = 0x00FFFFFF; /* White for selected text */
        }

        char prefix[8] = "      ";
        if (st->files[idx].is_dir) { prefix[0] = '['; prefix[4] = ']'; }

        char line[256];
        s32 k = 0;
        while (prefix[k] && k < 7) { line[k] = prefix[k]; k++; }
        
        char *nm = st->files[idx].name;
        s32 n_idx = 0;
        while (nm[n_idx] && k < 255) line[k++] = nm[n_idx++];
        line[k] = 0;

        ps5sdk_fb_str(fb, GW, GH, 50, y, line, color, 0, FSC, 1);
    }
    
    /* Scrollbar */
    if (st->count > max_visible) {
        s32 sb_h = (max_visible * list_bg_h) / st->count;
        if (sb_h < 20) sb_h = 20;
        s32 sb_y = list_bg_y + (st->scroll * (list_bg_h - sb_h)) / (st->count - max_visible);
        ps5sdk_fb_rect(fb, GW, GH, GW - 48, list_bg_y + 2, 6, list_bg_h - 4, 0x00202433);
        ps5sdk_fb_rect(fb, GW, GH, GW - 48, sb_y, 6, sb_h, 0x0005D9E8);
    }
}

__attribute__((section(".text._start")))
void _start(u64 eboot_base, u64 dlsym_addr, struct ext_args *ext) {
    if (!ext) return;
    ext->step = 1;

    struct ps5_example ex;
    s32 ret = ex_init(&ex, eboot_base, dlsym_addr, ext);
    if (ret != 0) { ext->status = ret; return; }

    /* Pad */
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libScePad.sprx");
    void *pad_init = SYM(ex.G, ex.D, PAD_HANDLE, "scePadInit");
    void *pad_geth = SYM(ex.G, ex.D, PAD_HANDLE, "scePadGetHandle");
    void *pad_read = SYM(ex.G, ex.D, PAD_HANDLE, "scePadRead");
    if (pad_init) NC(ex.G, pad_init, 0, 0, 0, 0, 0, 0);
    s32 pad_h = pad_geth ? (s32)NC(ex.G, pad_geth, (u64)ex.user_id, 0, 0, 0, 0, 0) : -1;

    /* FS syscalls */
    void *kopen    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelOpen);
    void *getdents = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelGetdents);
    void *kclose   = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelClose);

    /* VideoOut */
    void *cancel_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_scePthreadCancel);
    void *alloc_dm  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");

    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);

    if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
    if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
    if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);

    s32 video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    u64 phys = 0; void *vmem = 0;
    u8 *pad_buf = 0;

    if (pad_h >= 0 && ex.mmap_fn) {
        pad_buf = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
        if ((s64)pad_buf == -1) pad_buf = 0;
    }

    if (video_h >= 0 && alloc_dm && map_dm && vid_reg && vid_flip && kopen && getdents) {
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
                struct explorer_state *st = (struct explorer_state *)NC(ex.G, ex.mmap_fn, 0, sizeof(struct explorer_state) + 0x1000, 3, 0x1002, (u64)-1, 0);
                if ((s64)st != -1) {
                    str_cpy(st->cwd, "/");
                    load_dir(st, &ex, kopen, getdents, kclose);

                    u32 frame = 0, prev_btns = 0, done = 0;
                    s32 list_bg_h = GH - 170 - 60;
                    s32 max_visible = (list_bg_h - 20) / LIST_STEP;

                    while (!done) {
                        u32 raw = 0, btns = 0;
                        if (pad_read && pad_h >= 0 && pad_buf)
                            ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &raw, &btns);

                        u32 pressed = btns & ~prev_btns;
                        prev_btns = btns;

                        if (pressed & PS5SDK_PAD_BTN_OPTIONS) done = 1;
                        if (pressed & PS5SDK_PAD_BTN_DOWN) {
                            if (st->cursor < st->count - 1) st->cursor++;
                        }
                        if (pressed & PS5SDK_PAD_BTN_UP) {
                            if (st->cursor > 0) st->cursor--;
                        }

                        if (st->cursor < st->scroll) st->scroll = st->cursor;
                        if (st->cursor >= st->scroll + max_visible) st->scroll = st->cursor - max_visible + 1;

                        if (pressed & PS5SDK_PAD_BTN_CROSS) {
                            if (st->files[st->cursor].is_dir) {
                                if (str_cmp(st->files[st->cursor].name, "..") == 0) {
                                    path_up(st->cwd);
                                } else {
                                    path_append(st->cwd, st->files[st->cursor].name);
                                }
                                load_dir(st, &ex, kopen, getdents, kclose);
                            }
                        }
                        if (pressed & PS5SDK_PAD_BTN_CIRCLE) {
                            path_up(st->cwd);
                            load_dir(st, &ex, kopen, getdents, kclose);
                        }

                        s32 idx = (s32)(frame & 1u);
                        draw((u32 *)fbs[idx], st);

                        NC(ex.G, vid_flip, (u64)video_h, (u64)idx, 1, 0, 0, 0);
                        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);
                        frame++;
                    }
                    NC(ex.G, ex.munmap_fn, (u64)st, sizeof(struct explorer_state) + 0x1000, 0, 0, 0, 0);
                }
            }
        }
    }

    if (pad_buf && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)pad_buf, 0x1000, 0, 0, 0, 0);
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);

    ext->step = 99;
}
