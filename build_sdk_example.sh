#!/bin/bash
set -e

EXAMPLE="${1:-hello_dialog}"
IP="${2:-192.168.0.147}"
BIN="build/examples/${EXAMPLE}.bin"

echo "[*] Compilando ejemplo SDK: ${EXAMPLE}"
make -f Makefile.sdk_examples EXAMPLE="${EXAMPLE}" clean
make -f Makefile.sdk_examples EXAMPLE="${EXAMPLE}"

echo "[*] Inyectando ${BIN} en dump.lua..."
python3 convert.py "${BIN}" dump.lua

echo "[*] Lanzando payload hacia ${IP}..."
python3 dump_launcher.py "${IP}" --skip-upload
