#include "example_common.h"

#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_CREAT     0x0200
#define O_TRUNC     0x0400
#define DT_DIR      4
#define DT_REG      8

#define AF_INET     2
#define SOCK_STREAM 1
#define IPPROTO_TCP 6
#define SOL_SOCKET  0xFFFF
#define SO_REUSEADDR 0x0004
#define FIONBIO     0x8004667e
#define MSG_DONTWAIT 0x80

#define PS5SDK_PAD_BTN_SHARE    0x00000010u
#define PS5SDK_PAD_BTN_L3       0x00000100u
#define PS5SDK_PAD_BTN_R3       0x00000200u
#define PS5SDK_PAD_BTN_TOUCHPAD 0x00020000u

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

struct ftp_ctx {
    s32 srv, ctrl, data_srv;
    u8 my_ip[4];
    char line[512];
    u32 lpos;
    u8 active;
};

struct file_entry {
    char name[64];
    u8 is_dir;
    u64 size;
};

struct explorer_state {
    char cwd[512];
    struct file_entry files[MAX_FILES];
    s32 count;
    s32 cursor;
    s32 scroll;
    
    /* Text Viewer & Clipboard */
    s32 viewer_mode;
    s32 viewer_scroll;
    char viewer_content[32768];
    char clipboard_path[512];
    s32 clipboard_active;
    
    /* New: Progress, Delete & Status */
    s32 delete_confirm;
    s32 copy_active;
    u64 copy_total;
    u64 copy_done;
    u64 copy_start_tick;
    u8 *dlg_buf;
    u64 last_dlg_tick;
    s32 dirty;
    char status_msg[128];
    
    /* Virtual Keyboard */
    s32 kb_active;
    s32 kb_cur_x;
    s32 kb_cur_y;
    char kb_buf[64];
    char kb_title[64];

    struct ftp_ctx ftp;
    s32 lua_srv, lua_data;
    void *g_socket, *g_bind, *g_listen, *g_accept, *g_recv, *g_send;
    void *g_fcntl,  *g_gsn, *g_setsockopt, *g_close, *g_connect, *g_ioctl;
    /* libSceNet variants for sceNetSocket-based fds (ctrl socket) */
    void *g_net_accept, *g_net_recv, *g_net_send, *g_net_close;
};

static const char *kb_layout[] = {
    "ABCDEFGHIJKLM",
    "NOPQRSTUVWXYZ",
    "0123456789._-",
    " <SPC>  <DEL>  <DONE>  <CAN> "
};

#define KB_ROWS 4
#define KB_COLS 13

static s32 str_len(const char *s) {
    s32 n = 0;
    while (s && s[n]) n++;
    return n;
}

static void str_cpy(char *d, const char *s) {
    while (*s) *d++ = *s++;
    *d = 0;
}

static void str_cpy_n(char *d, const char *s, s32 cap) {
    if (!d || cap <= 0) return;
    s32 i = 0;
    if (s) {
        while (s[i] && i < cap - 1) {
            d[i] = s[i];
            i++;
        }
    }
    d[i] = 0;
}

static s32 str_cmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static s32 clamp_s32(s32 v, s32 lo, s32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static s32 char_index_in_set(char c, const char *set) {
    s32 i = 0;
    while (set[i]) {
        if (set[i] == c) return i;
        i++;
    }
    return 0;
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

static void load_dir(struct explorer_state *st, struct ps5_example *ex, void *kopen, void *getdents, void *kclose, void *lseek) {
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
                if (!(name[0] == '.' && name[1] == 0)) {
                    st->files[st->count].is_dir = (type == DT_DIR);
                    s32 k = 0;
                    while (name[k] && k < 63) {
                        st->files[st->count].name[k] = name[k];
                        k++;
                    }
                    st->files[st->count].name[k] = 0;
                    
                    /* Get size if file */
                    st->files[st->count].size = 0;
                    if (type != DT_DIR) {
                        char full[512];
                        str_cpy(full, st->cwd);
                        path_append(full, st->files[st->count].name);
                        s32 ffd = (s32)NC(ex->G, kopen, (u64)full, O_RDONLY, 0, 0, 0, 0);
                        if (ffd >= 0) {
                            st->files[st->count].size = (u64)NC(ex->G, lseek, (u64)ffd, 0, 2, 0, 0, 0); /* SEEK_END */
                            NC(ex->G, kclose, (u64)ffd, 0, 0, 0, 0, 0);
                        }
                    }
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

static void draw_viewer(u32 *fb, struct explorer_state *st) {
    ps5sdk_fb_fill(fb, GW, GH, 0x000D0E15);
    ps5sdk_fb_rect(fb, GW, GH, 0, 0, GW, 80, 0x00151822);
    ps5sdk_fb_rect(fb, GW, GH, 0, 80, GW, 3, 0x00FF2A6D);
    ps5sdk_fb_str(fb, GW, GH, 40, 25, "TEXT VIEWER", 0x00FF2A6D, 0, 4, 1);
    
    ps5sdk_fb_rect(fb, GW, GH, 40, 100, GW - 80, GH - 160, 0x0011131A);
    
    s32 y = 120;
    s32 line_count = 0;
    char *c = st->viewer_content;
    char line[128];
    s32 l_idx = 0;
    
    while (*c && y < GH - 100) {
        if (*c == '\n' || l_idx >= 127) {
            line[l_idx] = 0;
            if (line_count >= st->viewer_scroll) {
                ps5sdk_fb_str(fb, GW, GH, 60, y, line, 0x00E0E0E0, 0, 2, 1);
                y += 24;
            }
            line_count++;
            l_idx = 0;
        } else if (*c != '\r') {
            line[l_idx++] = *c;
        }
        c++;
    }
    
    ps5sdk_fb_rect(fb, GW, GH, 0, GH - 40, GW, 40, 0x00151822);
    ps5sdk_fb_str(fb, GW, GH, 40, GH - 30, "UP/DOWN: Scroll   CIRCLE: Exit Viewer", 0x008A95A5, 0, 2, 1);
}

static void ftp_puts(struct explorer_state *st, void *G, s32 fd, const char *s) {
    if (fd < 0) return;
    s32 n = str_len(s);
    /* ctrl fd was accepted from a sceNetSocket listener — use sceNetSend */
    if (st->g_net_send)
        NC(G, st->g_net_send, (u64)fd, (u64)s, (u64)n, 0, 0, 0);
    else if (st->g_send)
        NC(G, st->g_send, (u64)fd, (u64)s, (u64)n, 0, 0, 0);
}

static void draw(u32 *fb, struct explorer_state *st) {
    ps5sdk_fb_fill(fb, GW, GH, 0x000D0E15);
    ps5sdk_fb_rect(fb, GW, GH, 0, 0, GW, 140, 0x00151822);
    ps5sdk_fb_rect(fb, GW, GH, 0, 140, GW/2, 3, 0x00FF2A6D);
    ps5sdk_fb_rect(fb, GW, GH, GW/2, 140, GW/2, 3, 0x0005D9E8);

    ps5sdk_fb_str(fb, GW, GH, 40, 35, "PS5 EXPLORER + FTP", 0x0005D9E8, 0, 4, 1);
    
    if(st->ftp.active){
        char m[128];
        if (st->ftp.ctrl >= 0)
            ps5_sdk_snprintf(m,128,"FTP %d.%d.%d.%d:2121 OK",
                st->ftp.my_ip[0],st->ftp.my_ip[1],st->ftp.my_ip[2],st->ftp.my_ip[3]);
        else
            str_cpy(m, "FTP :2121 WAITING...");
        ps5sdk_fb_str(fb, GW, GH, GW-430, 35, m, 0x0000FF00, 0, 2, 1);
    } else {
        ps5sdk_fb_str(fb, GW, GH, GW-350, 35, "RIGHT: FTP :2121", 0x00A0A0A0, 0, 2, 1);
    }

    ps5sdk_fb_rect(fb, GW, GH, 40, 90, GW - 80, 36, 0x00202433);
    ps5sdk_fb_str(fb, GW, GH, 50, 98, st->cwd, 0x00E0E0E0, 0, 2, 1);

    s32 list_bg_y = 170;
    s32 list_bg_h = GH - list_bg_y - 60;
    ps5sdk_fb_rect(fb, GW, GH, 40, list_bg_y, GW - 80, list_bg_h, 0x0011131A);

    ps5sdk_fb_rect(fb, GW, GH, 0, GH - 40, GW, 40, 0x00151822);
    ps5sdk_fb_str(fb, GW, GH, 40, GH - 30, "X: Enter  SQ: Copy  TRI: Paste  L1: Del  R1: Rename  O: Back  OPT: Exit", 0x008A95A5, 0, 2, 1);
    
    if (st->status_msg[0]) {
        ps5sdk_fb_str(fb, GW, GH, 40, 148, st->status_msg, 0x0005D9E8, 0, 2, 1);
    }
    if (st->kb_active) {
        ps5sdk_fb_rect(fb, GW, GH, 120, GH - 220, GW - 240, 140, 0x00151822);
        ps5sdk_fb_rect(fb, GW, GH, 120, GH - 220, GW - 240, 3, 0x0005D9E8);
        ps5sdk_fb_str(fb, GW, GH, 150, GH - 200, st->kb_title, 0x00FFFFFF, 0, 2, 1);
        ps5sdk_fb_str(fb, GW, GH, 150, GH - 165, st->kb_buf, 0x0005D9E8, 0, 3, 1);
        ps5sdk_fb_str(fb, GW, GH, 150, GH - 130, "L/R mover  U/D letra  TRI +  SQ -  X OK  O cancelar", 0x008A95A5, 0, 2, 1);
    }

    s32 max_visible = (list_bg_h - 20) / LIST_STEP;
    for (s32 i = 0; i < max_visible && (st->scroll + i) < st->count; i++) {
        s32 idx = st->scroll + i;
        s32 y = list_bg_y + 10 + i * LIST_STEP;
        
        u32 color = st->files[idx].is_dir ? 0x0005D9E8 : 0x00A0AABF;
        if (str_cmp(st->files[idx].name, "<ACCESS DENIED>") == 0 || str_cmp(st->files[idx].name, "<EMPTY DIR>") == 0) {
            color = 0x00FF2A6D;
        }

        if (idx == st->cursor) {
            ps5sdk_fb_rect(fb, GW, GH, 45, y - 4, GW - 90, LIST_STEP, 0x00293145);
            color = 0x00FFFFFF;
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
        
        if (!st->files[idx].is_dir && st->files[idx].size > 0) {
            char sz_str[32];
            if (st->files[idx].size < 1024) ps5_sdk_snprintf(sz_str, 32, "%d B", (s32)st->files[idx].size);
            else if (st->files[idx].size < 1024*1024) ps5_sdk_snprintf(sz_str, 32, "%d KB", (s32)(st->files[idx].size / 1024));
            else ps5_sdk_snprintf(sz_str, 32, "%d MB", (s32)(st->files[idx].size / (1024*1024)));
            ps5sdk_fb_str(fb, GW, GH, GW - 250, y, sz_str, color, 0, 2, 1);
        }
    }
    
    if (st->delete_confirm) {
        ps5sdk_fb_rect(fb, GW, GH, GW/2 - 300, GH/2 - 100, 600, 200, 0x00151822);
        ps5sdk_fb_rect(fb, GW, GH, GW/2 - 300, GH/2 - 100, 600, 3, 0x00FF2A6D);
        ps5sdk_fb_str(fb, GW, GH, GW/2 - 250, GH/2 - 60, "DELETE FILE?", 0x00FFFFFF, 0, 3, 1);
        ps5sdk_fb_str(fb, GW, GH, GW/2 - 250, GH/2 - 20, st->files[st->cursor].name, 0x008A95A5, 0, 2, 1);
        ps5sdk_fb_str(fb, GW, GH, GW/2 - 250, GH/2 + 40, "SQ: Confirm  O: Cancel", 0x00FF2A6D, 0, 2, 1);
    }
    
    if (st->count > max_visible) {
        s32 sb_h = (max_visible * list_bg_h) / st->count;
        if (sb_h < 20) sb_h = 20;
        s32 sb_y = list_bg_y + (st->scroll * (list_bg_h - sb_h)) / (st->count - max_visible);
        ps5sdk_fb_rect(fb, GW, GH, GW - 48, list_bg_y + 2, 6, list_bg_h - 4, 0x00202433);
        ps5sdk_fb_rect(fb, GW, GH, GW - 48, sb_y, 6, sb_h, 0x0005D9E8);
    }
}

static void draw_keyboard(u32 *fb, struct explorer_state *st) {
    ps5sdk_fb_rect(fb, GW, GH, 0, 0, GW, GH, 0x00000000);

    s32 kw = 900, kh = 500;
    s32 kx = (GW - kw) / 2, ky = (GH - kh) / 2;
    ps5sdk_fb_rect(fb, GW, GH, kx, ky, kw, kh, 0x00151822);
    ps5sdk_fb_rect(fb, GW, GH, kx, ky, kw, 4, 0x0005D9E8);
    ps5sdk_fb_str(fb, GW, GH, kx + 40, ky + 30, st->kb_title, 0x008A95A5, 0, 2, 1);
    
    ps5sdk_fb_rect(fb, GW, GH, kx + 40, ky + 80, kw - 80, 60, 0x000D0E15);
    ps5sdk_fb_str(fb, GW, GH, kx + 60, ky + 95, st->kb_buf, 0x00FFFFFF, 0, 3, 1);
    
    for (int r = 0; r < 3; r++) {
        const char *row = kb_layout[r];
        s32 c_idx = 0;
        for (int c = 0; c < str_len(row); c++) {
            s32 bx = kx + 60 + c_idx * 60, by = ky + 180 + r * 70;
            u32 color = (st->kb_cur_y == r && st->kb_cur_x == c_idx) ? 0x0005D9E8 : 0x00202433;
            u32 tcolor = (st->kb_cur_y == r && st->kb_cur_x == c_idx) ? 0 : 0x008A95A5;
            ps5sdk_fb_rect(fb, GW, GH, bx, by, 50, 50, color);
            char k[2] = { row[c], 0 };
            ps5sdk_fb_str(fb, GW, GH, bx + 18, by + 15, k, tcolor, 0, 2, 1);
            c_idx++;
        }
    }

    /* Action row: fixed boxes to avoid string parsing edge-cases */
    {
        s32 ay = ky + 180 + 3 * 70;
        struct action_btn { const char *label; s32 x; s32 w; s32 sel_min; s32 sel_max; };
        const struct action_btn acts[] = {
            { "SPC",  kx +  60, 140, 0, 3  },
            { "DEL",  kx + 220, 140, 4, 6  },
            { "DONE", kx + 380, 180, 7, 9  },
            { "CAN",  kx + 580, 140, 10, 12}
        };
        for (u32 i = 0; i < 4; i++) {
            u8 selected = (st->kb_cur_y == 3 && st->kb_cur_x >= acts[i].sel_min && st->kb_cur_x <= acts[i].sel_max);
            u32 color = selected ? 0x0005D9E8 : 0x00202433;
            u32 tcolor = selected ? 0 : 0x008A95A5;
            ps5sdk_fb_rect(fb, GW, GH, acts[i].x, ay, acts[i].w, 50, color);
            ps5sdk_fb_str(fb, GW, GH, acts[i].x + 20, ay + 15, acts[i].label, tcolor, 0, 2, 1);
        }
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
    void *kread    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelRead);
    void *kwrite   = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelWrite);
    void *klseek   = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelLseek);
    void *kmkdir   = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "sceKernelMkdir");
    void *kunlink  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelUnlink);
    void *krename  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelRename);
    
    /* Network: use libkernel for basic I/O, but libSceNet for socket creation (libkernel socket() is restricted) */
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceNet.sprx");
    #define LIBSCENET_HANDLE 0x1e
    void *g_socket     = SYM(ex.G, ex.D, LIBSCENET_HANDLE, "sceNetSocket");
    void *g_bind       = SYM(ex.G, ex.D, LIBSCENET_HANDLE, "sceNetBind");
    void *g_listen     = SYM(ex.G, ex.D, LIBSCENET_HANDLE, "sceNetListen");
    /* sceNetAccept/Recv/Send must be used for sockets created via sceNetSocket.
     * libkernel accept/recv/send are used for the Lua-owned data socket (syscall fd). */
    void *g_net_accept = SYM(ex.G, ex.D, LIBSCENET_HANDLE, "sceNetAccept");
    void *g_net_recv   = SYM(ex.G, ex.D, LIBSCENET_HANDLE, "sceNetRecv");
    void *g_net_send   = SYM(ex.G, ex.D, LIBSCENET_HANDLE, "sceNetSend");
    void *g_net_close  = SYM(ex.G, ex.D, LIBSCENET_HANDLE, "sceNetSocketClose");

    void *g_setsockopt = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "setsockopt");
    void *g_gsn        = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "getsockname");
    void *g_accept     = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "accept");
    void *g_close      = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "close");
    void *g_recv       = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "recvfrom");
    void *g_send       = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "sendto");
    void *g_poll       = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "poll");

    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceRtc.sprx");
    void *get_tick = SYM(ex.G, ex.D, RTC_HANDLE, "sceRtcGetCurrentTick");
    
    /* VideoOut */
    void *cancel_fn = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_scePthreadCancel);
    void *alloc_dm  = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelAllocateDirectMemory);
    void *map_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelMapDirectMemory);
    void *rel_dm    = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, KERNEL_sceKernelReleaseDirectMemory);
    ps5_sdk_load_sprx(ex.G, ex.load_mod, "libSceVideoOut.sprx");
    void *vid_open  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutOpen");
    void *vid_close = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutClose");
    void *vid_reg   = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutRegisterBuffers");
    void *vid_flip  = SYM(ex.G, ex.D, VIDEOOUT_HANDLE, "sceVideoOutSubmitFlip");

    s32 emu_vid   = *(s32 *)(eboot_base + EBOOT_VIDOUT);
    u64 gs_thread = *(u64 *)(eboot_base + EBOOT_GS_THREAD);
    s32 video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    if (video_h < 0) {
        /* Fallback: only if non-destructive open failed, do a minimal takeover attempt. */
        if (cancel_fn && gs_thread) NC(ex.G, cancel_fn, gs_thread, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 300000, 0, 0, 0, 0, 0);
        if (vid_close && emu_vid >= 0) NC(ex.G, vid_close, (u64)emu_vid, 0, 0, 0, 0, 0);
        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 100000, 0, 0, 0, 0, 0);
        video_h = vid_open ? ps5sdk_vo_open(ex.G, vid_open) : -1;
    }
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
                    
                    /* WE DO NOT EX_ZERO THE STRUCT TO PREVENT CRASHES ON LARGE MEMSET! */
                    
                    /* MUST initialize all values to 0 explicitly just in case */
                    st->status_msg[0] = 0; st->kb_active = 0; st->delete_confirm = 0; st->clipboard_active = 0;
                    st->viewer_mode = 0; st->viewer_scroll = 0; st->copy_active = 0;
                    
                    /* Async transfer state (now stored in viewer_content padding) */
                    s32 *xf_type = (s32 *)(st->viewer_content + 32000);
                    *xf_type = 0;
                    
                    st->g_socket = g_socket; st->g_bind = g_bind; st->g_listen = g_listen; st->g_accept = g_accept;
                    st->g_recv = g_recv; st->g_send = g_send; st->g_fcntl = 0; st->g_gsn = g_gsn;
                    st->g_setsockopt = g_setsockopt; st->g_close = g_close; st->g_connect = 0;
                    st->g_ioctl = 0;
                    st->g_net_accept = g_net_accept; st->g_net_recv = g_net_recv;
                    st->g_net_send   = g_net_send;   st->g_net_close = g_net_close;
                    st->ftp.srv = -1; st->ftp.ctrl = -1; st->ftp.data_srv = -1; st->ftp.active = 0; st->ftp.lpos = 0;
                    st->cursor = 0; st->count = 0; st->scroll = 0;
                    for(int i=0; i<4; i++) st->ftp.my_ip[i] = 0;

                    /* Read the Lua-owned listening socket fds passed via ext->dbg[]:
                     * dbg[4] (+0x50) = FTP ctrl  listen socket (port 2121, syscall fd)
                     * dbg[5] (+0x58) = FTP data  listen socket (port 2122, syscall fd) */
                    st->ftp.srv  = (s32)(*(u64 *)((u8 *)ext + 0x50));
                    st->lua_data = (s32)(*(u64 *)((u8 *)ext + 0x58));
                    st->lua_srv  = st->ftp.srv;

                    str_cpy(st->cwd, "/");
                    load_dir(st, &ex, kopen, getdents, kclose, klseek);
                    st->dirty = 2;

                    u32 frame = 0, prev_btns = 0, done = 0;
                    s32 list_bg_h = GH - 170 - 60;
                    s32 max_visible = (list_bg_h - 20) / LIST_STEP;

                    while (!done) {
                        u32 raw = 0, btns = 0;
                        if (pad_read && pad_h >= 0 && pad_buf)
                            ps5_sdk_pad_read_buttons(ex.G, pad_read, pad_h, pad_buf, &raw, &btns);

                        u32 pressed = btns & ~prev_btns;
                        prev_btns = btns;

                        if (pressed) st->dirty = 2;

                        if (st->kb_active) {
                            const char *chset = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._-";
                            s32 len = str_len(st->kb_buf);
                            if (len <= 0) { st->kb_buf[0] = 'A'; st->kb_buf[1] = 0; len = 1; }
                            st->kb_cur_x = clamp_s32(st->kb_cur_x, 0, len - 1);

                            if (pressed & (PS5SDK_PAD_BTN_CIRCLE | PS5SDK_PAD_BTN_OPTIONS)) {
                                st->kb_active = 0;
                                st->dirty = 2;
                            }
                            if (pressed & PS5SDK_PAD_BTN_LEFT) {
                                st->kb_cur_x = clamp_s32(st->kb_cur_x - 1, 0, len - 1);
                                st->dirty = 2;
                            }
                            if (pressed & PS5SDK_PAD_BTN_RIGHT) {
                                if (st->kb_cur_x < len - 1) st->kb_cur_x++;
                                st->dirty = 2;
                            }
                            if (pressed & (PS5SDK_PAD_BTN_UP | PS5SDK_PAD_BTN_DOWN)) {
                                s32 idx = char_index_in_set(st->kb_buf[st->kb_cur_x], chset);
                                s32 max = str_len(chset) - 1;
                                if (pressed & PS5SDK_PAD_BTN_UP) idx = (idx + 1 > max) ? 0 : idx + 1;
                                if (pressed & PS5SDK_PAD_BTN_DOWN) idx = (idx - 1 < 0) ? max : idx - 1;
                                st->kb_buf[st->kb_cur_x] = chset[idx];
                                st->dirty = 2;
                            }
                            if (pressed & PS5SDK_PAD_BTN_TRIANGLE) {
                                if (len < 63) {
                                    st->kb_buf[len] = 'A';
                                    st->kb_buf[len + 1] = 0;
                                    st->kb_cur_x = len;
                                }
                                st->dirty = 2;
                            }
                            if (pressed & PS5SDK_PAD_BTN_SQUARE) {
                                if (len > 1) {
                                    s32 i = st->kb_cur_x;
                                    while (st->kb_buf[i]) { st->kb_buf[i] = st->kb_buf[i + 1]; i++; }
                                    len--;
                                    if (st->kb_cur_x >= len) st->kb_cur_x = len - 1;
                                    st->kb_cur_x = clamp_s32(st->kb_cur_x, 0, len - 1);
                                }
                                st->dirty = 2;
                            }
                            if (pressed & PS5SDK_PAD_BTN_CROSS) {
                                char full_old[512], full_new[512];
                                str_cpy(full_old, st->cwd); path_append(full_old, st->files[st->cursor].name);
                                str_cpy(full_new, st->cwd); path_append(full_new, st->kb_buf);
                                if (krename && str_len(st->kb_buf) > 0) {
                                    if ((s32)NC(ex.G, krename, (u64)full_old, (u64)full_new, 0, 0, 0, 0) == 0) str_cpy(st->status_msg, "RENAMED");
                                    else str_cpy(st->status_msg, "RENAME FAILED");
                                } else {
                                    str_cpy(st->status_msg, "RENAME FAILED");
                                }
                                st->kb_active = 0;
                                load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                st->dirty = 2;
                            }
                        } else if (st->viewer_mode) {
                            if (pressed & (PS5SDK_PAD_BTN_CIRCLE | PS5SDK_PAD_BTN_OPTIONS)) { st->viewer_mode = 0; st->dirty = 2; }
                            if (btns & PS5SDK_PAD_BTN_DOWN) { st->viewer_scroll++; st->dirty = 2; }
                            if (btns & PS5SDK_PAD_BTN_UP) { if (st->viewer_scroll > 0) { st->viewer_scroll--; st->dirty = 2; } }
                        } else if (st->delete_confirm) {
                            if (pressed & PS5SDK_PAD_BTN_SQUARE) {
                                char *full = st->viewer_content;
                                str_cpy(full, st->cwd);
                                path_append(full, st->files[st->cursor].name);
                                if ((s32)NC(ex.G, kunlink, (u64)full, 0, 0, 0, 0, 0) == 0) str_cpy(st->status_msg, "DELETED");
                                else str_cpy(st->status_msg, "DELETE FAILED");
                                st->delete_confirm = 0;
                                load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                st->dirty = 2;
                            }
                            if (pressed & PS5SDK_PAD_BTN_CIRCLE) { st->delete_confirm = 0; st->dirty = 2; }
                        } else {
                            if (pressed & PS5SDK_PAD_BTN_OPTIONS) done = 1;
                            if (pressed & PS5SDK_PAD_BTN_DOWN) {
                                if (st->cursor < st->count - 1) { st->cursor++; st->dirty = 2; }
                            }
                            if (pressed & PS5SDK_PAD_BTN_UP) {
                                if (st->cursor > 0) { st->cursor--; st->dirty = 2; }
                            }

                            if (st->cursor < st->scroll) { st->scroll = st->cursor; st->dirty = 2; }
                            if (st->cursor >= st->scroll + max_visible) { st->scroll = st->cursor - max_visible + 1; st->dirty = 2; }

                            if (pressed & PS5SDK_PAD_BTN_CROSS) {
                                if (st->files[st->cursor].is_dir) {
                                    if (str_cmp(st->files[st->cursor].name, "..") == 0) {
                                        path_up(st->cwd);
                                    } else {
                                        path_append(st->cwd, st->files[st->cursor].name);
                                    }
                                    load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                    st->dirty = 2;
                                } else {
                                    /* Open text viewer */
                                    char *full = st->viewer_content;
                                    str_cpy(full, st->cwd);
                                    path_append(full, st->files[st->cursor].name);
                                    s32 fd = (s32)NC(ex.G, kopen, (u64)full, O_RDONLY, 0, 0, 0, 0);
                                    if (fd >= 0) {
                                        s32 n = (s32)NC(ex.G, kread, (u64)fd, (u64)st->viewer_content, sizeof(st->viewer_content)-1, 0, 0, 0);
                                        if (n >= 0) st->viewer_content[n] = 0;
                                        else str_cpy(st->viewer_content, "Error reading file.");
                                        NC(ex.G, kclose, (u64)fd, 0, 0, 0, 0, 0);
                                        st->viewer_mode = 1;
                                        st->viewer_scroll = 0;
                                        st->dirty = 2;
                                    } else {
                                        str_cpy(st->status_msg, "CANNOT OPEN FILE");
                                        st->dirty = 2;
                                    }
                                }
                            }
                            if (pressed & PS5SDK_PAD_BTN_CIRCLE) {
                                path_up(st->cwd);
                                load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                st->dirty = 2;
                            }
                            
                            if (pressed & PS5SDK_PAD_BTN_L1) {
                                if (!st->files[st->cursor].is_dir && str_cmp(st->files[st->cursor].name, "..") != 0) {
                                    st->delete_confirm = 1;
                                    st->dirty = 2;
                                }
                            }
                            
                            if (pressed & PS5SDK_PAD_BTN_SQUARE) {
                                if (!st->files[st->cursor].is_dir) {
                                    str_cpy(st->clipboard_path, st->cwd);
                                    path_append(st->clipboard_path, st->files[st->cursor].name);
                                    st->clipboard_active = 1;
                                    st->copy_total = st->files[st->cursor].size;
                                    str_cpy(st->status_msg, "COPIED TO CLIPBOARD");
                                    st->dirty = 2;
                                }
                            }
                            
                            if (pressed & PS5SDK_PAD_BTN_TRIANGLE) {
                                if (st->clipboard_active) {
                                    char dst[512];
                                    str_cpy(dst, st->cwd);
                                    char *fn = st->clipboard_path;
                                    s32 len = str_len(fn);
                                    while (len > 0 && fn[len-1] != '/') len--;
                                    path_append(dst, fn + len);
                                    
                                    s32 fd_in = (s32)NC(ex.G, kopen, (u64)st->clipboard_path, O_RDONLY, 0, 0, 0, 0);
                                    s32 fd_out = (s32)NC(ex.G, kopen, (u64)dst, O_WRONLY | O_CREAT | O_TRUNC, 0666, 0, 0, 0);
                                    
                                    if (fd_in >= 0 && fd_out >= 0) {
                                        st->copy_active = 1;
                                        st->copy_done = 0;
                                        if (get_tick) {
                                            NC(ex.G, get_tick, (u64)&st->copy_start_tick, 0, 0, 0, 0, 0);
                                            st->last_dlg_tick = st->copy_start_tick;
                                        }
                                        
                                        st->dlg_buf = ex_alloc_dialog(&ex);
                                        if (st->dlg_buf) {
                                            ex_dialog_begin(&ex);
                                            ex_open_progress(&ex, st->dlg_buf, "Iniciando copia...");
                                        }

                                        u8 *buf = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x40000, 3, 0x1002, (u64)-1, 0);
                                        if ((s64)buf != -1) {
                                            s32 rd;
                                            while ((rd = (s32)NC(ex.G, kread, (u64)fd_in, (u64)buf, 0x40000, 0, 0, 0)) > 0) {
                                                NC(ex.G, kwrite, (u64)fd_out, (u64)buf, (u64)rd, 0, 0, 0);
                                                st->copy_done += rd;
                                                
                                                u64 cur_tick = 0;
                                                if (get_tick) NC(ex.G, get_tick, (u64)&cur_tick, 0, 0, 0, 0, 0);
                                                
                                                if (st->dlg_buf && (cur_tick - st->last_dlg_tick > 200000)) {
                                                    st->last_dlg_tick = cur_tick;
                                                    u32 pct = (u32)((st->copy_done * 100) / st->copy_total);
                                                    u64 ms = (cur_tick - st->copy_start_tick) / 1000;
                                                    
                                                    if (ex.dlg_update) NC(ex.G, ex.dlg_update, 0, 0, 0, 0, 0, 0);
                                                    if (ex.dlg_set_val) NC(ex.G, ex.dlg_set_val, 0, (u64)pct, 0, 0, 0, 0);
                                                    if (ex.dlg_set_msg && ms > 0) {
                                                        char tmsg[128];
                                                        s32 speed_kb = (s32)(st->copy_done / ms);
                                                        ps5_sdk_snprintf(tmsg, 128, "Copiando: %d%% (%d KB/s)", pct, speed_kb);
                                                        NC(ex.G, ex.dlg_set_msg, 0, (u64)tmsg, 0, 0, 0, 0);
                                                    }
                                                }
                                            }
                                            NC(ex.G, ex.munmap_fn, (u64)buf, 0x40000, 0, 0, 0, 0);
                                            str_cpy(st->status_msg, "PASTE SUCCESSFUL");
                                        }
                                        
                                        if (st->dlg_buf) {
                                            ex_close_dialog(&ex);
                                            ex_free_dialog(&ex, st->dlg_buf);
                                            st->dlg_buf = 0;
                                        }
                                        st->copy_active = 0;
                                        st->dirty = 2;
                                    } else {
                                        str_cpy(st->status_msg, "PASTE FAILED");
                                        st->dirty = 2;
                                    }
                                    if (fd_in >= 0) NC(ex.G, kclose, (u64)fd_in, 0, 0, 0, 0, 0);
                                    if (fd_out >= 0) NC(ex.G, kclose, (u64)fd_out, 0, 0, 0, 0, 0);
                                    load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                }
                            }
                            
                            if (pressed & PS5SDK_PAD_BTN_R1) {
                                if (st->cursor >= 0 && st->cursor < st->count &&
                                    str_cmp(st->files[st->cursor].name, "..") != 0) {
                                    str_cpy_n(st->kb_title, "RENAME FILE", (s32)sizeof(st->kb_title));
                                    str_cpy_n(st->kb_buf, st->files[st->cursor].name, (s32)sizeof(st->kb_buf));
                                    st->kb_active = 1;
                                    st->kb_cur_x = 0;
                                    st->kb_cur_y = 0;
                                    st->dirty = 2;
                                }
                            }

                            if (pressed & PS5SDK_PAD_BTN_RIGHT) {
                                if (!st->ftp.active) {
                                    st->ftp.active = 1;
                                    st->ftp.srv = st->lua_srv;  /* restore Lua-owned listen fd */
                                    str_cpy(st->status_msg, "FTP: SERVER STARTED");
                                    st->dirty = 2;
                                } else {
                                    if (st->ftp.ctrl >= 0) { NC(ex.G, st->g_net_close ? st->g_net_close : st->g_close, (u64)st->ftp.ctrl, 0, 0, 0, 0, 0); st->ftp.ctrl = -1; }
                                    /* srv and lua_data belong to Lua — never close them */
                                    st->ftp.active = 0;
                                    str_cpy(st->status_msg, "FTP STOPPED");
                                }
                                st->dirty = 2;
                            }
                        }

                        /* Process async transfer if active */
                        s32 *xf_type = (s32 *)(st->viewer_content + 32000);
                        s32 *xf_fd   = (s32 *)(st->viewer_content + 32004);
                        s32 *xf_dc   = (s32 *)(st->viewer_content + 32008);
                        u8 **xf_buf  = (u8 **)(st->viewer_content + 32012);

                        if (*xf_type > 0) {
                            u8 dpfd[8]; *(s32*)(dpfd+0) = *xf_dc; 
                            *(u16*)(dpfd+4) = (*xf_type == 1) ? 0x0001 : 0x0004; /* POLLIN or POLLOUT */
                            *(u16*)(dpfd+6) = 0;
                            s32 pr = g_poll ? (s32)NC(ex.G, g_poll, (u64)dpfd, 1, 0, 0, 0, 0) : -1;
                            if (pr > 0) {
                                if (*xf_type == 1) { /* STOR */
                                    s32 rd = (s32)NC(ex.G, st->g_recv, (u64)*xf_dc, (u64)*xf_buf, 0x40000, 0, 0, 0);
                                    if (rd > 0) {
                                        NC(ex.G, kwrite, (u64)*xf_fd, (u64)*xf_buf, (u64)rd, 0, 0, 0);
                                    } else {
                                        NC(ex.G, ex.munmap_fn, (u64)*xf_buf, 0x40000, 0, 0, 0, 0);
                                        NC(ex.G, kclose, (u64)*xf_fd, 0, 0, 0, 0, 0);
                                        NC(ex.G, st->g_close, (u64)*xf_dc, 0, 0, 0, 0, 0);
                                        *xf_type = 0;
                                        ftp_puts(st, ex.G, st->ftp.ctrl, "226 Transfer complete\r\n");
                                        str_cpy(st->status_msg, "FTP: FILE RECEIVED");
                                        load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                        st->dirty = 2;
                                    }
                                } else if (*xf_type == 2) { /* RETR */
                                    s32 rd = (s32)NC(ex.G, kread, (u64)*xf_fd, (u64)*xf_buf, 0x40000, 0, 0, 0);
                                    if (rd > 0) {
                                        NC(ex.G, st->g_send, (u64)*xf_dc, (u64)*xf_buf, (u64)rd, 0, 0, 0);
                                    } else {
                                        NC(ex.G, ex.munmap_fn, (u64)*xf_buf, 0x40000, 0, 0, 0, 0);
                                        NC(ex.G, kclose, (u64)*xf_fd, 0, 0, 0, 0, 0);
                                        NC(ex.G, st->g_close, (u64)*xf_dc, 0, 0, 0, 0, 0);
                                        *xf_type = 0;
                                        ftp_puts(st, ex.G, st->ftp.ctrl, "226 Transfer complete\r\n");
                                        str_cpy(st->status_msg, "FTP: FILE SENT");
                                        st->dirty = 2;
                                    }
                                }
                            }
                        }

                        /* Draw FIRST — frame is always updated before any FTP operation */
                        if (st->dirty > 0) {
                            s32 l_idx = (s32)(frame & 1u);
                            if (st->viewer_mode) draw_viewer((u32 *)fbs[l_idx], st);
                            else draw((u32 *)fbs[l_idx], st);
                            /* Keyboard rendered in simple HUD mode via draw(). */
                            NC(ex.G, vid_flip, (u64)video_h, (u64)l_idx, 1, 0, 0, 0);
                            st->dirty--;
                            frame++;
                        }

                        /* FTP after draw — poll(0) is non-blocking */
                        if (st->ftp.active && st->ftp.srv >= 0) {
                            /* Always check for new incoming connections to prevent FileZilla timeouts
                             * when it tries to open multiple simultaneous connections. */
                            u8 pfd[8];
                            *(s32*)(pfd+0) = st->ftp.srv;
                            *(u16*)(pfd+4) = 0x0001; *(u16*)(pfd+6) = 0;
                            s32 pr = g_poll ? (s32)NC(ex.G, g_poll, (u64)pfd, 1, 0, 0, 0, 0) : -1;
                            if (pr > 0) {
                                /* ftp.srv was created via sceNetSocket — use sceNetAccept */
                                s32 cl = st->g_net_accept
                                    ? (s32)NC(ex.G, st->g_net_accept, (u64)st->ftp.srv, 0, 0, 0, 0, 0)
                                    : (s32)NC(ex.G, st->g_accept,     (u64)st->ftp.srv, 0, 0, 0, 0, 0);
                                if (cl >= 0) {
                                    if (st->ftp.ctrl < 0) {
                                        st->ftp.ctrl = cl;
                                        ftp_puts(st, ex.G, st->ftp.ctrl, "220 PS5 FTP Ready\r\n");
                                        u8 la[16] = {0}; u32 lalen = 16;
                                        if (st->g_gsn)
                                            NC(ex.G, st->g_gsn, (u64)st->ftp.ctrl, (u64)la, (u64)&lalen, 0, 0, 0);
                                        st->ftp.my_ip[0] = la[4]; st->ftp.my_ip[1] = la[5];
                                        st->ftp.my_ip[2] = la[6]; st->ftp.my_ip[3] = la[7];
                                        str_cpy(st->status_msg, "FTP: CLIENT CONNECTED");
                                        st->dirty = 2;
                                    } else {
                                        /* Second connection while one is active: close silently.
                                         * FileZilla must be set to 1 simultaneous connection. */
                                        NC(ex.G, st->g_close, (u64)cl, 0, 0, 0, 0, 0);
                                    }
                                }
                            }
                            
                            if (st->ftp.ctrl >= 0) {
                                s32 rr; u8 b;
                                while ((rr = st->g_net_recv
                                    ? (s32)NC(ex.G, st->g_net_recv, (u64)st->ftp.ctrl, (u64)&b, 1, MSG_DONTWAIT, 0, 0)
                                    : (s32)NC(ex.G, st->g_recv,     (u64)st->ftp.ctrl, (u64)&b, 1, MSG_DONTWAIT, 0, 0)) > 0) {
                                    if (b == '\r') continue;
                                    if (b == '\n') {
                                        st->ftp.line[st->ftp.lpos] = 0;
                                        /* Parse verb */
                                        char *ln = st->ftp.line;
                                        s32 vi = 0;
                                        while (ln[vi] && ln[vi] != ' ' && vi < 7) vi++;
                                        char verb[8]; s32 ki = 0;
                                        while (ki < vi) { verb[ki] = ln[ki]; ki++; }
                                        verb[ki] = 0;
                                        const char *farg = (ln[vi] == ' ') ? ln + vi + 1 : ln + vi;
                                        if (str_cmp(verb, "QUIT") == 0) {
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "221 Bye\r\n");
                                            NC(ex.G, st->g_net_close ? st->g_net_close : st->g_close, (u64)st->ftp.ctrl, 0, 0, 0, 0, 0); st->ftp.ctrl = -1;
                                            /* data_srv stays — Lua owns it */
                                        } else if (str_cmp(verb, "USER") == 0 || str_cmp(verb, "PASS") == 0) {
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "230 Logged in\r\n");
                                        } else if (str_cmp(verb, "SYST") == 0) {
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "215 UNIX Type: L8\r\n");
                                        } else if (str_cmp(verb, "FEAT") == 0 || str_cmp(verb, "OPTS") == 0) {
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "211 End\r\n");
                                        } else if (str_cmp(verb, "TYPE") == 0 || str_cmp(verb, "MODE") == 0 ||
                                                   str_cmp(verb, "STRU") == 0 || str_cmp(verb, "NOOP") == 0 || str_cmp(verb, "ABOR") == 0) {
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "200 OK\r\n");
                                        } else if (str_cmp(verb, "PWD") == 0 || str_cmp(verb, "XPWD") == 0) {
                                            char resp[560]; s32 ri = 0;
                                            const char *pp = "257 \""; while (*pp) resp[ri++] = *pp++;
                                            s32 pj = 0; while (st->cwd[pj] && ri < 540) resp[ri++] = st->cwd[pj++];
                                            resp[ri++] = '"'; resp[ri++] = '\r'; resp[ri++] = '\n'; resp[ri] = 0;
                                            ftp_puts(st, ex.G, st->ftp.ctrl, resp);
                                        } else if (str_cmp(verb, "CWD") == 0 || str_cmp(verb, "XCWD") == 0) {
                                            if (str_cmp(farg, "..") == 0) path_up(st->cwd);
                                            else if (farg[0] == '/') str_cpy(st->cwd, farg);
                                            else path_append(st->cwd, farg);
                                            load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "250 CWD OK\r\n");
                                            st->dirty = 2;
                                        } else if (str_cmp(verb, "CDUP") == 0) {
                                            path_up(st->cwd);
                                            load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "200 CDUP OK\r\n");
                                            st->dirty = 2;
                                        } else if (str_cmp(verb, "MKD") == 0 || str_cmp(verb, "XMKD") == 0) {
                                            char mfull[512];
                                            if (farg[0] == '/') str_cpy(mfull, farg);
                                            else { str_cpy(mfull, st->cwd); path_append(mfull, farg); }
                                            s32 mr = kmkdir ? (s32)NC(ex.G, kmkdir, (u64)mfull, 0755, 0, 0, 0, 0) : -1;
                                            if (mr == 0) {
                                                char mrsp[560]; s32 ri = 0;
                                                const char *pm = "257 \""; while (*pm) mrsp[ri++] = *pm++;
                                                s32 pj = 0; while (mfull[pj] && ri < 540) mrsp[ri++] = mfull[pj++];
                                                mrsp[ri++] = '"'; mrsp[ri++] = '\r'; mrsp[ri++] = '\n'; mrsp[ri] = 0;
                                                ftp_puts(st, ex.G, st->ftp.ctrl, mrsp);
                                                load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                                st->dirty = 2;
                                            } else ftp_puts(st, ex.G, st->ftp.ctrl, "550 Cannot create directory\r\n");
                                        } else if (str_cmp(verb, "SIZE") == 0) {
                                            char sfull[512];
                                            if (farg[0] == '/') str_cpy(sfull, farg);
                                            else { str_cpy(sfull, st->cwd); path_append(sfull, farg); }
                                            s32 sfd = (s32)NC(ex.G, kopen, (u64)sfull, O_RDONLY, 0, 0, 0, 0);
                                            if (sfd >= 0) {
                                                u64 sz = (u64)NC(ex.G, klseek, (u64)sfd, 0, 2, 0, 0, 0);
                                                NC(ex.G, kclose, (u64)sfd, 0, 0, 0, 0, 0);
                                                char sresp[32]; ps5_sdk_snprintf(sresp, 32, "213 %d\r\n", (s32)sz);
                                                ftp_puts(st, ex.G, st->ftp.ctrl, sresp);
                                            } else ftp_puts(st, ex.G, st->ftp.ctrl, "550 Not found\r\n");
                                        } else if (str_cmp(verb, "PASV") == 0 || str_cmp(verb, "EPSV") == 0) {
                                            /* Flush any stale connections from the listen queue.
                                             * FileZilla sometimes opens multiple connections or leaves them dangling.
                                             * If we don't clear them, accept() might pick up a dead connection. */
                                            while (1) {
                                                u8 dpfd[8]; *(s32*)(dpfd+0) = st->lua_data; *(u16*)(dpfd+4) = 0x0001; *(u16*)(dpfd+6) = 0;
                                                s32 pr = g_poll ? (s32)NC(ex.G, g_poll, (u64)dpfd, 1, 0, 0, 0, 0) : -1;
                                                if (pr > 0) {
                                                    s32 dummy = (s32)NC(ex.G, st->g_accept, (u64)st->lua_data, 0, 0, 0, 0, 0);
                                                    if (dummy >= 0) NC(ex.G, g_close, (u64)dummy, 0, 0, 0, 0, 0);
                                                } else break;
                                            }
                                            
                                            /* Revert to using the pre-existing lua_data listening socket (port 2122).
                                             * This is proven to work and avoids issues with creating new sockets. */
                                            st->ftp.data_srv = st->lua_data;
                                            if (st->ftp.data_srv >= 0) {
                                                if (str_cmp(verb, "EPSV") == 0) {
                                                    ftp_puts(st, ex.G, st->ftp.ctrl, "229 Entering Extended Passive Mode (|||2122|)\r\n");
                                                } else {
                                                    char dresp[64];
                                                    ps5_sdk_snprintf(dresp, 64,
                                                        "227 Entering Passive Mode (%d,%d,%d,%d,8,74)\r\n",
                                                        (s32)st->ftp.my_ip[0], (s32)st->ftp.my_ip[1],
                                                        (s32)st->ftp.my_ip[2], (s32)st->ftp.my_ip[3]);
                                                    ftp_puts(st, ex.G, st->ftp.ctrl, dresp);
                                                }
                                                str_cpy(st->status_msg, "FTP: PASV :2122");
                                                st->dirty = 2;
                                            } else {
                                                ftp_puts(st, ex.G, st->ftp.ctrl, "425 No data socket\r\n");
                                            }
                                        } else if (str_cmp(verb, "LIST") == 0 || str_cmp(verb, "NLST") == 0 || str_cmp(verb, "MLSD") == 0) {
                                            if (st->ftp.data_srv < 0) {
                                                ftp_puts(st, ex.G, st->ftp.ctrl, "425 No data connection\r\n");
                                            } else {
                                                ftp_puts(st, ex.G, st->ftp.ctrl, "150 Directory listing\r\n");
                                                u8 dpfd[8]; *(s32*)(dpfd+0) = st->ftp.data_srv; *(u16*)(dpfd+4) = 0x0001; *(u16*)(dpfd+6) = 0;
                                                s32 pr = g_poll ? (s32)NC(ex.G, g_poll, (u64)dpfd, 1, 5000, 0, 0, 0) : -1;
                                                s32 dc = pr > 0 ? (s32)NC(ex.G, st->g_accept, (u64)st->ftp.data_srv, 0, 0, 0, 0, 0) : -1;
                                                if (dc >= 0) {
                                                    for (s32 li = 0; li < st->count; li++) {
                                                        char ll[320];
                                                        if (st->files[li].is_dir)
                                                            ps5_sdk_snprintf(ll, 320, "drwxr-xr-x 1 root root 0 Jan 1 00:00 %s\r\n", st->files[li].name);
                                                        else
                                                            ps5_sdk_snprintf(ll, 320, "-rw-r--r-- 1 root root %d Jan 1 00:00 %s\r\n", (s32)st->files[li].size, st->files[li].name);
                                                        NC(ex.G, st->g_send, (u64)dc, (u64)ll, str_len(ll), 0, 0, 0);
                                                    }
                                                    NC(ex.G, st->g_close, (u64)dc, 0, 0, 0, 0, 0);
                                                    ftp_puts(st, ex.G, st->ftp.ctrl, "226 Directory send OK\r\n");
                                                } else ftp_puts(st, ex.G, st->ftp.ctrl, "425 Data connection failed\r\n");
                                                /* Close the listening socket as it's single-use per PASV */
                                                if (st->ftp.data_srv >= 0 && st->ftp.data_srv != st->lua_data)
                                                    NC(ex.G, g_close, (u64)st->ftp.data_srv, 0, 0, 0, 0, 0);
                                                st->ftp.data_srv = -1;
                                            }

                                        } else if (str_cmp(verb, "STOR") == 0) {
                                            if (st->ftp.data_srv < 0) {
                                                ftp_puts(st, ex.G, st->ftp.ctrl, "425 No data connection\r\n");
                                            } else {
                                                u8 dpfd[8]; *(s32*)(dpfd+0) = st->ftp.data_srv; *(u16*)(dpfd+4) = 0x0001; *(u16*)(dpfd+6) = 0;
                                                s32 pr = g_poll ? (s32)NC(ex.G, g_poll, (u64)dpfd, 1, 5000, 0, 0, 0) : -1;
                                                s32 dc = pr > 0 ? (s32)NC(ex.G, st->g_accept, (u64)st->ftp.data_srv, 0, 0, 0, 0, 0) : -1;
                                                if (dc >= 0) {
                                                    char sfull[512];
                                                    if (farg[0] == '/') str_cpy(sfull, farg);
                                                    else { str_cpy(sfull, st->cwd); path_append(sfull, farg); }
                                                    s32 fd = (s32)NC(ex.G, kopen, (u64)sfull, O_WRONLY | O_CREAT | O_TRUNC, 0666, 0, 0, 0);
                                                    u8 *xbuf = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x40000, 3, 0x1002, (u64)-1, 0);
                                                    if (fd >= 0 && (s64)xbuf != -1) {
                                                        s32 *xf_type = (s32 *)(st->viewer_content + 32000);
                                                        s32 *xf_fd   = (s32 *)(st->viewer_content + 32004);
                                                        s32 *xf_dc   = (s32 *)(st->viewer_content + 32008);
                                                        u8 **xf_buf  = (u8 **)(st->viewer_content + 32012);
                                                        *xf_type = 1; *xf_fd = fd; *xf_dc = dc; *xf_buf = xbuf;
                                                        ftp_puts(st, ex.G, st->ftp.ctrl, "150 Opening data connection\r\n");
                                                    } else {
                                                        if ((s64)xbuf != -1) NC(ex.G, ex.munmap_fn, (u64)xbuf, 0x40000, 0, 0, 0, 0);
                                                        if (fd >= 0) NC(ex.G, kclose, (u64)fd, 0, 0, 0, 0, 0);
                                                        ftp_puts(st, ex.G, st->ftp.ctrl, "550 Cannot create file\r\n");
                                                        NC(ex.G, st->g_close, (u64)dc, 0, 0, 0, 0, 0);
                                                    }
                                                } else ftp_puts(st, ex.G, st->ftp.ctrl, "425 Data connection failed\r\n");
                                                if (st->ftp.data_srv >= 0 && st->ftp.data_srv != st->lua_data) NC(ex.G, g_close, (u64)st->ftp.data_srv, 0, 0, 0, 0, 0);
                                                st->ftp.data_srv = -1;
                                            }
                                        } else if (str_cmp(verb, "RETR") == 0) {
                                            if (st->ftp.data_srv < 0) {
                                                ftp_puts(st, ex.G, st->ftp.ctrl, "425 No data connection\r\n");
                                            } else {
                                                u8 dpfd[8]; *(s32*)(dpfd+0) = st->ftp.data_srv; *(u16*)(dpfd+4) = 0x0001; *(u16*)(dpfd+6) = 0;
                                                s32 pr = g_poll ? (s32)NC(ex.G, g_poll, (u64)dpfd, 1, 5000, 0, 0, 0) : -1;
                                                s32 dc = pr > 0 ? (s32)NC(ex.G, st->g_accept, (u64)st->ftp.data_srv, 0, 0, 0, 0, 0) : -1;
                                                if (dc >= 0) {
                                                    char rfull[512];
                                                    if (farg[0] == '/') str_cpy(rfull, farg);
                                                    else { str_cpy(rfull, st->cwd); path_append(rfull, farg); }
                                                    s32 fd = (s32)NC(ex.G, kopen, (u64)rfull, O_RDONLY, 0, 0, 0, 0);
                                                    u8 *xbuf = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x40000, 3, 0x1002, (u64)-1, 0);
                                                    if (fd >= 0 && (s64)xbuf != -1) {
                                                        s32 *xf_type = (s32 *)(st->viewer_content + 32000);
                                                        s32 *xf_fd   = (s32 *)(st->viewer_content + 32004);
                                                        s32 *xf_dc   = (s32 *)(st->viewer_content + 32008);
                                                        u8 **xf_buf  = (u8 **)(st->viewer_content + 32012);
                                                        *xf_type = 2; *xf_fd = fd; *xf_dc = dc; *xf_buf = xbuf;
                                                        ftp_puts(st, ex.G, st->ftp.ctrl, "150 Opening data connection\r\n");
                                                    } else {
                                                        if ((s64)xbuf != -1) NC(ex.G, ex.munmap_fn, (u64)xbuf, 0x40000, 0, 0, 0, 0);
                                                        if (fd >= 0) NC(ex.G, kclose, (u64)fd, 0, 0, 0, 0, 0);
                                                        ftp_puts(st, ex.G, st->ftp.ctrl, "550 File not found\r\n");
                                                        NC(ex.G, st->g_close, (u64)dc, 0, 0, 0, 0, 0);
                                                    }
                                                } else ftp_puts(st, ex.G, st->ftp.ctrl, "425 Data connection failed\r\n");
                                                if (st->ftp.data_srv >= 0 && st->ftp.data_srv != st->lua_data) NC(ex.G, g_close, (u64)st->ftp.data_srv, 0, 0, 0, 0, 0);
                                                st->ftp.data_srv = -1;
                                            }
                                        } else if (str_cmp(verb, "DELE") == 0) {
                                            char dfull[512];
                                            if (farg[0] == '/') str_cpy(dfull, farg);
                                            else { str_cpy(dfull, st->cwd); path_append(dfull, farg); }
                                            s32 dr = (s32)NC(ex.G, kunlink, (u64)dfull, 0, 0, 0, 0, 0);
                                            if (dr == 0) {
                                                ftp_puts(st, ex.G, st->ftp.ctrl, "250 File deleted\r\n");
                                                load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                                st->dirty = 2;
                                            } else ftp_puts(st, ex.G, st->ftp.ctrl, "550 Delete failed\r\n");
                                        } else if (str_cmp(verb, "RMD") == 0 || str_cmp(verb, "XRMD") == 0) {
                                            char rfull[512];
                                            if (farg[0] == '/') str_cpy(rfull, farg);
                                            else { str_cpy(rfull, st->cwd); path_append(rfull, farg); }
                                            void *krmdir = SYM(ex.G, ex.D, LIBKERNEL_HANDLE, "sceKernelRmdir");
                                            s32 rr2 = krmdir ? (s32)NC(ex.G, krmdir, (u64)rfull, 0, 0, 0, 0, 0) : -1;
                                            if (rr2 == 0) {
                                                ftp_puts(st, ex.G, st->ftp.ctrl, "250 Directory removed\r\n");
                                                load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                                st->dirty = 2;
                                            } else ftp_puts(st, ex.G, st->ftp.ctrl, "550 Remove failed\r\n");
                                        } else if (str_cmp(verb, "RNFR") == 0) {
                                            if (farg[0] == '/') str_cpy(st->clipboard_path, farg);
                                            else { str_cpy(st->clipboard_path, st->cwd); path_append(st->clipboard_path, farg); }
                                            st->clipboard_active = 2;
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "350 Ready for RNTO\r\n");
                                        } else if (str_cmp(verb, "RNTO") == 0) {
                                            if (st->clipboard_active == 2 && krename) {
                                                char rdst[512];
                                                if (farg[0] == '/') str_cpy(rdst, farg);
                                                else { str_cpy(rdst, st->cwd); path_append(rdst, farg); }
                                                /* Try renameat(AT_FDCWD, old, AT_FDCWD, new) which is common on PS5 */
                                                s32 rr3 = (s32)NC(ex.G, krename, (u64)-100, (u64)st->clipboard_path, (u64)-100, (u64)rdst, 0, 0);
                                                if (rr3 == 0) {
                                                    ftp_puts(st, ex.G, st->ftp.ctrl, "250 Rename OK\r\n");
                                                    load_dir(st, &ex, kopen, getdents, kclose, klseek);
                                                    st->dirty = 2;
                                                } else ftp_puts(st, ex.G, st->ftp.ctrl, "550 Rename failed\r\n");
                                            } else ftp_puts(st, ex.G, st->ftp.ctrl, "503 No RNFR\r\n");
                                            st->clipboard_active = 0;
                                        } else {
                                            ftp_puts(st, ex.G, st->ftp.ctrl, "500 Unknown command\r\n");
                                        }
                                        st->ftp.lpos = 0;
                                    } else if (st->ftp.lpos < 510) st->ftp.line[st->ftp.lpos++] = (char)b;
                                }
                                if (rr == 0) {
                                    NC(ex.G, st->g_net_close ? st->g_net_close : st->g_close, (u64)st->ftp.ctrl, 0, 0, 0, 0, 0); st->ftp.ctrl = -1;
                                    /* data_srv stays — Lua owns it, ready for next PASV session */
                                    str_cpy(st->status_msg, "FTP: DISCONNECTED");
                                    st->dirty = 2;
                                }
                            }
                        }

                        if (ex.usleep_fn) NC(ex.G, ex.usleep_fn, 16000, 0, 0, 0, 0, 0);
                    }
                    /* Best-effort cleanup on exit: close runtime sockets/fds if left open. */
                    if (st->ftp.ctrl >= 0) {
                        NC(ex.G, st->g_net_close ? st->g_net_close : st->g_close, (u64)st->ftp.ctrl, 0, 0, 0, 0, 0);
                        st->ftp.ctrl = -1;
                    }
                    {
                        s32 *xf_type = (s32 *)(st->viewer_content + 32000);
                        s32 *xf_fd   = (s32 *)(st->viewer_content + 32004);
                        s32 *xf_dc   = (s32 *)(st->viewer_content + 32008);
                        u8 **xf_buf  = (u8 **)(st->viewer_content + 32012);
                        if (*xf_type > 0) {
                            if (*xf_buf && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)*xf_buf, 0x40000, 0, 0, 0, 0);
                            if (*xf_fd >= 0) NC(ex.G, kclose, (u64)*xf_fd, 0, 0, 0, 0, 0);
                            if (*xf_dc >= 0) NC(ex.G, st->g_close, (u64)*xf_dc, 0, 0, 0, 0, 0);
                            *xf_type = 0; *xf_fd = -1; *xf_dc = -1; *xf_buf = 0;
                        }
                    }
                    if (st->dlg_buf) {
                        ex_close_dialog(&ex);
                        ex_free_dialog(&ex, st->dlg_buf);
                        st->dlg_buf = 0;
                    }
                    NC(ex.G, ex.munmap_fn, (u64)st, sizeof(struct explorer_state) + 0x1000, 0, 0, 0, 0);
                }
            }
        }
    }

    if (pad_buf && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)pad_buf, 0x1000, 0, 0, 0, 0);
    if (vid_close && video_h >= 0) NC(ex.G, vid_close, (u64)video_h, 0, 0, 0, 0, 0);
    if (vmem && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)vmem, FB_TOT, 0, 0, 0, 0);
    if (rel_dm && phys) NC(ex.G, rel_dm, phys, FB_TOT, 0, 0, 0, 0);

    ext->step = 99;
    ex_exit_clean(&ex, 0);
}
