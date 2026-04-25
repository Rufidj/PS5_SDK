#include "example_common.h"

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
    char tmp[160];

    tmp[0] = 0;
    append_text(tmp, sizeof(tmp), text ? text : "");
    ps5_sdk_dialog_zero(buf, PS5SDK_DLG_BUF_SIZE);
    ps5_sdk_ascii_copy(ps5_sdk_dialog_msg(buf), 160, tmp);
    ps5_sdk_dialog_setup_user_msg(
        ps5_sdk_dialog_dp(buf),
        ps5_sdk_dialog_pp(buf),
        ps5_sdk_dialog_msg(buf),
        ex->user_id
    );
    return (s32)NC(ex->G, ex->dlg_open, (u64)ps5_sdk_dialog_dp(buf), 0, 0, 0, 0, 0);
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
        list_ret = ps5_sdk_user_get_login_ids(ex.G, get_login_list, ids, 4, work, 0x1000);
    }

    if (work && get_user_name) {
        s32 target_id = (ids[0] != -1) ? ids[0] : ex.user_id;
        name_ret = ps5_sdk_user_get_name(ex.G, get_user_name, target_id, name0, sizeof(name0), work + 0x100, 0x100);
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
