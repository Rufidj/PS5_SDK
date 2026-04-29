# PS5 SDK Examples

Small examples to test `src/ps5_sdk.h` without relying on the NES emulator code.

## Portability Notes

- Recent examples are being migrated to dynamic module-handle resolution (runtime `load_mod` + symbol-by-name) instead of assuming fixed module handle IDs.
- This improves behavior across consoles where handle numbering may differ.
- `SYM(...)` now includes a portable fallback: if a static handle fails, it auto-loads the mapped module path and retries `dlsym` with the runtime handle.
- Dump research reference: [reports/mi_dump_scan_report_notes.md](/home/ruben/EmuC0re/reports/mi_dump_scan_report_notes.md)
- `portable`: uses runtime symbol resolution only (`SYM` + `ex_sym_module`) and does not depend on local RAM offsets.
- `context-sensitive`: may still rely on local runtime state (`EBOOT_VIDOUT`, `EBOOT_GS_THREAD`) or takeover paths that can differ per context/firmware.

## Stability Matrix

- `stable baseline`: `hello_dialog`, `kernel_info`, `pad_read_dialog`, `audio_beep`, `pad_party`, `sdk_dashboard`
- `experimental`: `pad_party_rgb`, `pad_lightbar_live`, `videoout_takeover*`, `hello_square*`, `gnm_*`
- Recommendation: run examples one by one (`./build_sdk_example.sh <example> <YOUR_PS5_IP>`).

## Lifecycle & Cleanup Rules

All SDK examples must be safe to run repeatedly (launch many times in a row).

Mandatory cleanup rules for developers:

- Close all opened file descriptors and sockets before exiting.
- Unmap/free all temporary buffers (`mmap`, direct memory, transfer buffers).
- Close/free dialogs if they were opened.
- Restore/close taken-over runtime handles (VideoOut, Pad, Audio, etc.).
- Ensure no async transfer state is left active on exit.

Global exit behavior:

- Clean return-to-menu is implemented in C examples through `ex_finish(&ex, ext, status, code)`.
- Keep launcher Lua generic; app lifecycle/exit should be owned by each `.c` payload.
- Official clean-exit reference for developers: `hello_dialog` (dialog + `ex_finish(...)` at the end).
- `exit_app` is experimental and may crash depending on runtime context; keep it only as an optional test.

Known launcher behavior:

- The launcher stays generic; clean app exit must be implemented by each payload in C.
- Some launchers still do socket/fd cleanup to reduce stale listeners between runs.

## Examples

- `hello_dialog`: opens a simple text dialog with `Hello world`, waits for accept, and exits cleanly back to PS5 menu using `ex_finish(...)`.
- `exit_app` (`experimental`): optional exit test; may crash on some contexts/firmwares.
- `formatter_dialog`: tests `ps5_sdk_snprintf` with `%d`, `%x`, and `%p` inside a text dialog.
- `progress_dialog`: opens a progress bar and updates text/percentage.
- `kernel_info`: calls simple libkernel functions and shows `pid`, `uid`, `gid`, `cpumode`, and process time.
- `sysmodule_resolve`: resolves functions from `libSceSysmodule` and displays their pointers.
- `sysmodule_batch_info` (`portable`): loads several base modules at once and shows their handles/returns with very low memory cost.
- `audioout_resolve` (`portable`): loads/resolves `libSceAudioOut` without opening audio output yet.
- `audioout_open_probe` (`portable`): initializes AudioOut, attempts to open a port with known parameters, and closes it if it opens.
- `audio_beep` (`portable`): opens AudioOut, plays a short beep, and closes the port.
- `pad_read_dialog`: initializes `libScePad`, reads buttons for a few seconds, and displays raw values.
- `input_beep`: combines `libScePad` and `libSceAudioOut`; each new button plays a different tone.
- `pad_fx_probe`: lightweight `libScePad` probe to see if advanced APIs like `scePadSetTriggerEffect`, `scePadSetLightBar`, and `scePadGetControllerInformation` resolve.
- `pad_lightbar_probe`: lightweight visual attempt to change the lightbar color and restore it afterwards.
- `pad_lightbar_live`: interactive example; face buttons change the controller's lightbar color for a few seconds.
- `pad_rgb_cycle`: short cycle of three colors on the lightbar and restoration to the original color.
- `pad_party` (`portable`): interactive mix pad + beep (safe mode, no lightbar writes); stable baseline across consoles.
- `pad_party_rgb` (`context-sensitive`): experimental variant with lightbar + beep per button; may fail depending on runtime/controller context.
- `pad_trigger_probe`: first conservative attempt at `scePadSetTriggerEffect` to notice if `R2` offers resistance for an instant and then reset.
- `pad_info_probe`: reads the first raw fields of `scePadGetControllerInformation` to better understand controller capabilities and layout.
- `pad_trigger_matrix`: tests various minimal byte layouts for `scePadSetTriggerEffect` and compares returns without spending much memory.
- `pad_trigger_tune_probe`: fine-tunes only the valid layout at offset `0` with various intensities/modes to try to notice a real effect and find an accepted reset.
- `pad_trigger_ext_probe`: compares `scePadSetTriggerEffect` using `scePadGetHandle`, `scePadOpenExt`, and `scePadOpenExt2` to see if the advanced route requires another handle.
- `user_service_info` (`portable`): queries `libSceUserService`, lists logged-in users, and attempts to display the name.
- `system_params_info` (`portable`): queries `libSceSystemService` and displays language, date/time format, and time zone.
- `regmgr_read_info` (`portable`): reads a few keys from `libSceRegMgr` in read-only mode.
- `gpu_info` (`portable`): queries `libSceGnmDriver` in read-only mode to show GPU clock and `userPaEnabled`.
- `sdk_dashboard` (`portable`): safe dashboard with `MsgDialog` combining user, local time, network, system, and GPU in a single view.
- `sdk_live_launcher`: interactive mini app with controller; live menu in `MsgDialog` to navigate between dashboard, pad, audio, and system.
- `videoout_info`: loads `libSceVideoOut` and resolves basic functions without opening or registering buffers.
- `videoout_host_state` (`context-sensitive`): inspects raw host state (`EBOOT_VIDOUT`, `EBOOT_GS_THREAD`) and how many base `VideoOut` functions are resolved.
- `videoout_gs_probe` (`context-sensitive`): tests in isolation if `scePthreadCancel` can stop the host graphics thread without touching buffers or flips.
- `videoout_takeover_probe` (`context-sensitive`): replicates the `main.c`/`main_nes.c` sequence: cancel `gs`, close `emu_vid`, and test `sceVideoOutOpen` with type fallbacks, without registering buffers yet.
- `videoout_takeover_reg_probe` (`context-sensitive`): takes takeover sequence one step further testing `AllocateDirectMemory + MapDirectMemory + RegisterBuffers`, but without `SubmitFlip` yet.
- `videoout_takeover_flip_probe` (`context-sensitive`): extends the successful takeover to `SetFlipRate + SubmitFlip`, drawing a minimal framebuffer to attempt the first real SDK render.
- `fb_animation`: smooth bouncing square animation with full color cycle (hue 0→360); true double buffering alternating indices 0/1, 360 frames at ~60 fps. Validated on hardware.
- `fb_pad_draw`: interactive framebuffer control with the controller; D-pad moves a square, face buttons (CROSS/CIRCLE/TRIANGLE/SQUARE/L1/R1) change its color, CROSS to exit (timeout 15s). Validated on hardware.
- `fb_text`: on-screen HUD animation using `ps5sdk_fb_str` and `ps5sdk_fb_dec`; displays frame counter and updating X/Y coordinates live. Validated on hardware.
- `fb_explorer`: interactive visual file explorer using the GPU framebuffer; navigate directories, identify folders/files, and verify read access safely. Requires libScePad + libSceVideoOut.
- `fb_3d_cube`: realtime software 3D renderer displaying a rotating wireframe cube using Bresenham lines, integer-math 3D projection, and sin/cos look-up tables.
- `fb_synthwave`: realtime 3D software rendering of an Outrun/Synthwave aesthetic moving grid with a glowing retro sun, mountains, and neon color gradients.
- `arkanoid`: full game — 6-color bricks, ball with bounce physics, D-pad controlled paddle, row-based scoring, 3 lives, beep sounds on collision with ball/brick/life lost; CROSS launches/restarts, OPTIONS exits. Requires libScePad + libSceVideoOut + libSceAudioOut.
- `fb_snake`: full snake game — eat apples to grow, D-pad to move, walls and self-collision kills, score tracking, dynamic speed scaling, beep sounds; CROSS launches/restarts, OPTIONS exits. Requires libScePad + libSceVideoOut + libSceAudioOut.
- `videoout_open_probe`: opens and closes VideoOut without registering buffers or making flips.
- `videoout_open_matrix`: tests various `sceVideoOutOpen` combinations to find one that opens in this context.
- `hello_square`: attempts to take `VideoOut`, register a framebuffer, and draw a simple square on screen.
- `hello_square_debug`: walks the same graphic path but returns with a return code dialog to debug `VideoOut`.
- `net_resolve_info`: loads `libSceNet`/`libSceNetCtl` and resolves libkernel socket functions without opening sockets.
- `socket_open_probe`: opens and closes a UDP socket without binding or sending traffic.
- `net_init_probe`: initializes `libSceNet`, tests `sceNetSocket`, closes, and terminates Net.
- `netctl_state_info`: queries `libSceNetCtl` to read the current network state.
- `rtc_resolve_info`: loads `libSceRtc` and tests resolving typical clock/date functions.
- `rtc_tick_info`: calls `sceRtcGetCurrentTick` and displays the current tick.
- `rtc_clock_local_info`: calls `sceRtcGetCurrentClockLocalTime` and attempts to show local date/time.
- `mount_points_info`: tests several common mount points in read-only mode to show if they can be opened/listed.
- `mount_syscalls_info`: resolves `mount` and `unmount` from `libkernel` without attempting to use them.
- `mount_probe_info`: resolves mount/FS helpers (`mount`, `unmount`, `nmount`, `statfs`, `getfsstat`) without invoking them.
- `fsstat_probe`: calls `getfsstat(NULL, 0, 0)` to request the number of visible filesystems without mounting anything.
- `statfs_probe`: tests `statfs(path, buf)` on several known paths using simple dialogs one by one.
- `fs_write_test`: tests `sceKernelOpen`, `sceKernelWrite`, and `sceKernelRead` by attempting to write and read a string to a file in `/data/`.

## Build

```bash
make -f Makefile.sdk_examples EXAMPLE=hello_dialog
make -f Makefile.sdk_examples EXAMPLE=formatter_dialog
make -f Makefile.sdk_examples EXAMPLE=progress_dialog
make -f Makefile.sdk_examples EXAMPLE=fb_snake
make -f Makefile.sdk_examples EXAMPLE=fs_write_test
make -f Makefile.sdk_examples EXAMPLE=fb_explorer
make -f Makefile.sdk_examples EXAMPLE=fb_3d_cube
make -f Makefile.sdk_examples EXAMPLE=fb_synthwave
```

## Advanced Pad State

- `scePadSetLightBar`: validated on real hardware. `set=0` and `reset=0`; controller color changes and restores correctly.
- `scePadSetTriggerEffect`: the correct path uses `scePadGetHandle`, not `scePadOpenExt/OpenExt2`.
- `scePadSetTriggerEffect`: various minimal structures already return `0`, so the API is accessible.
- `scePadSetTriggerEffect`: we have not yet found a combination that produces a noticeable effect on `R2`, so the exact structure content is still being fine-tuned.

## Run

```bash
./build_sdk_example.sh hello_dialog <YOUR_PS5_IP>
./build_sdk_example.sh exit_app <YOUR_PS5_IP>
./build_sdk_example.sh formatter_dialog <YOUR_PS5_IP>
./build_sdk_example.sh progress_dialog <YOUR_PS5_IP>
./build_sdk_example.sh kernel_info <YOUR_PS5_IP>
./build_sdk_example.sh sysmodule_resolve <YOUR_PS5_IP>
./build_sdk_example.sh sysmodule_batch_info <YOUR_PS5_IP>
./build_sdk_example.sh audioout_resolve <YOUR_PS5_IP>
./build_sdk_example.sh audioout_open_probe <YOUR_PS5_IP>
./build_sdk_example.sh audio_beep <YOUR_PS5_IP>
./build_sdk_example.sh pad_read_dialog <YOUR_PS5_IP>
./build_sdk_example.sh input_beep <YOUR_PS5_IP>
./build_sdk_example.sh pad_fx_probe <YOUR_PS5_IP>
./build_sdk_example.sh pad_lightbar_probe <YOUR_PS5_IP>
./build_sdk_example.sh pad_lightbar_live <YOUR_PS5_IP>
./build_sdk_example.sh pad_rgb_cycle <YOUR_PS5_IP>
./build_sdk_example.sh pad_party <YOUR_PS5_IP>
./build_sdk_example.sh pad_party_rgb <YOUR_PS5_IP>
./build_sdk_example.sh pad_trigger_probe <YOUR_PS5_IP>
./build_sdk_example.sh pad_info_probe <YOUR_PS5_IP>
./build_sdk_example.sh pad_trigger_matrix <YOUR_PS5_IP>
./build_sdk_example.sh pad_trigger_tune_probe <YOUR_PS5_IP>
./build_sdk_example.sh pad_trigger_ext_probe <YOUR_PS5_IP>
./build_sdk_example.sh user_service_info <YOUR_PS5_IP>
./build_sdk_example.sh system_params_info <YOUR_PS5_IP>
./build_sdk_example.sh regmgr_read_info <YOUR_PS5_IP>
./build_sdk_example.sh gpu_info <YOUR_PS5_IP>
./build_sdk_example.sh sdk_dashboard <YOUR_PS5_IP>
./build_sdk_example.sh videoout_host_state <YOUR_PS5_IP>
./build_sdk_example.sh videoout_gs_probe <YOUR_PS5_IP>
./build_sdk_example.sh videoout_takeover_probe <YOUR_PS5_IP>
./build_sdk_example.sh videoout_takeover_reg_probe <YOUR_PS5_IP>
./build_sdk_example.sh videoout_takeover_flip_probe <YOUR_PS5_IP>
./build_sdk_example.sh fb_animation <YOUR_PS5_IP>
./build_sdk_example.sh fb_pad_draw <YOUR_PS5_IP>
./build_sdk_example.sh fb_text <YOUR_PS5_IP>
./build_sdk_example.sh arkanoid <YOUR_PS5_IP>
./build_sdk_example.sh fb_snake <YOUR_PS5_IP>
./build_sdk_example.sh videoout_info <YOUR_PS5_IP>
./build_sdk_example.sh videoout_open_probe <YOUR_PS5_IP>
./build_sdk_example.sh videoout_open_matrix <YOUR_PS5_IP>
./build_sdk_example.sh hello_square <YOUR_PS5_IP>
./build_sdk_example.sh hello_square_debug <YOUR_PS5_IP>
./build_sdk_example.sh net_resolve_info <YOUR_PS5_IP>
./build_sdk_example.sh socket_open_probe <YOUR_PS5_IP>
./build_sdk_example.sh net_init_probe <YOUR_PS5_IP>
./build_sdk_example.sh netctl_state_info <YOUR_PS5_IP>
./build_sdk_example.sh rtc_resolve_info <YOUR_PS5_IP>
./build_sdk_example.sh rtc_tick_info <YOUR_PS5_IP>
./build_sdk_example.sh rtc_clock_local_info <YOUR_PS5_IP>
./build_sdk_example.sh mount_points_info <YOUR_PS5_IP>
./build_sdk_example.sh mount_syscalls_info <YOUR_PS5_IP>
./build_sdk_example.sh mount_probe_info <YOUR_PS5_IP>
./build_sdk_example.sh fsstat_probe <YOUR_PS5_IP>
./build_sdk_example.sh statfs_probe <YOUR_PS5_IP>
./build_sdk_example.sh fs_write_test <YOUR_PS5_IP>
./build_sdk_example.sh fb_explorer <YOUR_PS5_IP>
./build_sdk_example.sh fb_3d_cube <YOUR_PS5_IP>
./build_sdk_example.sh fb_synthwave <YOUR_PS5_IP>
```

The launcher may display `FTP server not responding after 10s`; this is normal for these example payloads because they don't host an FTP server.
