#!/bin/bash
set -e

# Configuration
EXAMPLE="${1:-hello_dialog}"
PS5_IP="${2:-YOUR_PS5_IP}"
BIN="build/examples/${EXAMPLE}.bin"

echo "[*] Compiling SDK Example: ${EXAMPLE}..."
make -f Makefile.sdk_examples EXAMPLE="${EXAMPLE}" clean
make -f Makefile.sdk_examples EXAMPLE="${EXAMPLE}"

echo "[*] Injecting ${BIN} into payload.lua..."
python3 convert.py "${BIN}" payload.lua

echo "[*] Sending payload to PS5 at ${PS5_IP}..."
python3 launcher.py "${PS5_IP}" --skip-upload

echo "[+] Done!"
