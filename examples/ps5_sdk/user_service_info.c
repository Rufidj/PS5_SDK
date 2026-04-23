#include "example_common.h"

static void clear_bytes(u8 *p, u32 size, u8 value) {
    for (u32 i = 0; i < size; i++) p[i] = value;
}

static void safe_copy_name(char *dst, const char *src, u32 max) {
    u32 i = 0;
    if (!dst || max == 0) return;
    if (src) {
        while (i + 1 < max && src[i]) {
            char c = src[i];
            dst[i] = (c >= 32 && c <= 126) ? c : '?';
            i++;
        }
    }
    dst[i] = 0;
}

static void append_text(char *dst, u32 dst_size, const char *src) {
    u32 pos = 0;
    while (pos + 1 < dst_size && dst[pos]) pos++;
    while (pos + 1 < dst_size && *src) dst[pos++] = *src++;
    dst[pos] = 0;
}

static void append_hex32(char *dst, u32 dst_size, u32 value) {
    const char *hex = "0123456789abcdef";
    append_text(dst, dst_size, "0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        char tmp[2];
        tmp[0] = hex[(value >> shift) & 0xF];
        tmp[1] = 0;
        append_text(dst, dst_size, tmp);
    }
}

static s32 open_text_raw(struct ps5_example *ex, u8 *buf, const char *text) {
    u8 *dp = buf;
    u8 *pp = buf + 0x88;
    char *msg = (char *)(buf + 0xE0);
    char tmp[160];

    tmp[0] = 0;
    append_text(tmp, sizeof(tmp), text ? text : "");
    ex_zero(buf, 0x400);
    append_text(msg, 160, tmp);
    ex_setup_user_msg(dp, pp, msg, ex->user_id);
    return (s32)NC(ex->G, ex->dlg_open, (u64)dp, 0, 0, 0, 0, 0);
}

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

    NC(ex.G, ex.load_mod, (u64)"libSceUserService.sprx", 0, 0, 0, 0, 0);
    void *get_login_list = SYM(ex.G, ex.D, USERSERVICE_HANDLE, "sceUserServiceGetLoginUserIdList");
    void *get_user_name = SYM(ex.G, ex.D, USERSERVICE_HANDLE, "sceUserServiceGetUserName");

    u8 *work = (u8 *)NC(ex.G, ex.mmap_fn, 0, 0x1000, 3, 0x1002, (u64)-1, 0);
    if ((s64)work == -1) work = 0;

    s32 ids[4];
    for (int i = 0; i < 4; i++) ids[i] = -1;

    s64 list_ret = -1;
    s64 name_ret = -1;
    char name0[64];
    name0[0] = 0;

    if (work && get_login_list) {
        clear_bytes(work, 0x1000, 0);
        for (int i = 0; i < 16; i++) ((s32 *)work)[i] = -1;
        list_ret = (s64)NC(ex.G, get_login_list, (u64)work, 0, 0, 0, 0, 0);
        if (list_ret == 0) {
            for (int i = 0; i < 4; i++) ids[i] = ((s32 *)work)[i];
        }
    }

    if (work && get_user_name) {
        s32 target_id = (ids[0] != -1) ? ids[0] : ex.user_id;
        clear_bytes(work + 0x100, 0x100, 0);
        name_ret = (s64)NC(ex.G, get_user_name, (u64)target_id, (u64)(work + 0x100), 0x100, 0, 0, 0);
        if (name_ret == 0) safe_copy_name(name0, (char *)(work + 0x100), sizeof(name0));
    }

    ext->dbg[0] = (u64)(s64)ids[0];
    ext->dbg[1] = (u64)list_ret;
    ext->dbg[2] = (u64)name_ret;
    ext->status = 0;

    u8 *buf = ex_alloc_dialog(&ex);
    if (!buf) {
        if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
        ext->status = -3;
        return;
    }

    ext->step = 2;
    ex_dialog_begin(&ex);
    char *final_msg = (char *)(buf + 0x240);
    final_msg[0] = 0;
    if (list_ret == 0 && name_ret == 0 && name0[0]) {
        append_text(final_msg, 160, "UserService info\nUsuario activo:\n");
        append_text(final_msg, 160, name0);
        append_text(final_msg, 160, "\nUserService OK");
    } else if (list_ret == 0) {
        append_text(final_msg, 160, "UserService info\nUsuario detectado\nNombre no disponible\nnameRet=");
        append_hex32(final_msg, 160, (u32)name_ret);
    } else {
        append_text(final_msg, 160, "UserService info\nNo se pudo leer la lista\nlistRet=");
        append_hex32(final_msg, 160, (u32)list_ret);
    }
    ret = open_text_raw(&ex, buf, final_msg);
    if (ret == 0) ex_wait_user_close(&ex);
    ex_close_dialog(&ex);

    ex_free_dialog(&ex, buf);
    if (work && ex.munmap_fn) NC(ex.G, ex.munmap_fn, (u64)work, 0x1000, 0, 0, 0, 0);
    ext->step = 99;
}
