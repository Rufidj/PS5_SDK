# SDK Build & Run Guide

This guide explains requirements and exact steps to compile and launch payload examples on PS5.

## Requirements

### Common

- PS5 reachable on LAN
- `git`
- Python 3.10+ (`python3 --version` / `python --version`)
- `make`
- `gcc`
- `objcopy` (usually from `binutils`)

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y git build-essential binutils make gcc python3
```

### Windows

Recommended: **MSYS2 MinGW64** shell.

Install packages in MSYS2:

```bash
pacman -S --needed git make mingw-w64-x86_64-gcc mingw-w64-x86_64-binutils python
```

If you use `cmd.exe`/PowerShell, ensure `make`, `gcc`, `objcopy`, and `python` are in `PATH`.

## Project Layout for Launchers

Each script now creates an isolated launcher per payload to avoid `dump.lua` corruption/overwrite:

- SDK example: `examples/build/<example>/<example>.lua`
- Priv example: `examples/build/priv_<example>/<example>.lua`
- Hello SDK: `examples/build/hello_sdk/hello_sdk.lua`
- Master dump: `examples/build/master_dump/master_dump.lua`

## Linux Script Usage

### SDK example

```bash
./build_sdk_example.sh <example> <ps5_ip>
# example:
./build_sdk_example.sh pad_party YOUR_PS5_IP
```

### Priv example

```bash
./build_priv_example.sh <example> <ps5_ip>
```

### Hello SDK

```bash
./build_hello_sdk.sh <ps5_ip>
```

### Master dump

```bash
./build_dump.sh <ps5_ip>
```

## Windows `.bat` Scripts

Use these from `cmd.exe` (or PowerShell). They mirror the Linux scripts:

- `build_sdk_example.bat`
- `build_priv_example.bat`
- `build_hello_sdk.bat`
- `build_dump.bat`

Examples:

```bat
build_sdk_example.bat pad_party YOUR_PS5_IP
```

## Troubleshooting

- `ConnectionRefusedError` / connection refused:
  - PS5 payload endpoint is not listening yet. Relaunch exploit entry and retry.
- `FTP server not responding after 10s`:
  - Normal for many examples (they do not host FTP).
- Build errors about missing `gcc`/`objcopy`:
  - Install toolchain packages and confirm they are in `PATH`.

## Repeated Launch Stability

If launching one payload works but the next one crashes, the usual cause is resource leakage
(open sockets/fds, pending dialogs, mmap buffers, or VideoOut handles) from the previous run.

Developer checklist (mandatory for new examples):

- Always close every fd/socket opened by the payload before `_start` exits.
- Always unmap every `mmap`/direct-memory allocation you created.
- Always terminate/free message dialogs if opened.
- Always close/restore VideoOut or controller/audio handles you takeover.
- Avoid background loops that survive exit conditions.

Launcher-side note:

- `dump.lua` now performs best-effort socket cleanup (`ftp_srv`, `ftp_data`, `web_sock`, `log_sock`) at the end of execution to reduce stale state across runs.
