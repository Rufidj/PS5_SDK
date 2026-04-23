# PS5 SDK Examples
![Author](https://img.shields.io/badge/Author-Rufidj-blue?style=flat-square)
![Firmware](https://img.shields.io/badge/PS5-13.00-black?style=flat-square)

Small, standalone examples to test `src/ps5_sdk.h` functionality without dependencies from external emulators or complex codebases.

## Examples Overview

* **hello_dialog**: Opens a simple text box displaying "Hello World" and waits for the user to dismiss it.
* **formatter_dialog**: Tests `ps5_sdk_snprintf` with `%d`, `%x`, and `%p` formatters inside a system dialog.
* **progress_dialog**: Displays a native progress bar and dynamically updates both text and percentage.
* **kernel_info**: Calls basic `libkernel` functions to retrieve and display PID, UID, GID, CPU mode, and process uptime.
* **sysmodule_resolve**: Resolves symbols from `libSceSysmodule` and prints their memory pointers.
* **audioout_resolve**: Loads and resolves `libSceAudioOut` without initializing the audio output yet.
* **audioout_open_probe**: Initializes AudioOut, attempts to open a port with known parameters, and closes it upon success.
* **audio_beep**: Opens AudioOut, plays a short beep tone, and safely closes the port.
* **pad_read_dialog**: Initializes `libScePad`, polls controller inputs for a few seconds, and displays raw button values.
* **input_beep**: A hybrid test using `libScePad` and `libSceAudioOut`; each button press triggers a distinct audio tone.
* **user_service_info**: Queries `libSceUserService` to list logged-in users and attempts to retrieve their usernames.
* **system_params_info**: Uses `libSceSystemService` to display system language, date/time format, and timezone.
* **regmgr_read_info**: Performs read-only queries on specific keys within `libSceRegMgr`.
* **gpu_info**: Queries `libSceGnmDriver` in read-only mode to display GPU clock speeds and `userPaEnabled` status.
* **videoout_info**: Loads `libSceVideoOut` and resolves basic functions without opening or registering buffers.
* **videoout_open_probe**: Opens and closes VideoOut without buffer registration or frame flipping.
* **net_resolve_info**: Loads `libSceNet`/`libSceNetCtl` and resolves socket functions from `libkernel` without opening active sockets.
* **socket_open_probe**: Opens and closes a UDP socket without binding or sending traffic.
* **net_init_probe**: Initializes `libSceNet`, tests `sceNetSocket`, then closes and terminates the network stack.

## Build Instructions

To compile a specific example using the provided Makefile, run:

```bash
make -f Makefile.sdk_examples EXAMPLE=hello_dialog
make -f Makefile.sdk_examples EXAMPLE=formatter_dialog
make -f Makefile.sdk_examples EXAMPLE=progress_dialog
