#!/bin/bash
set -e

EXAMPLE="${1:-hello_dialog}"
IP="${2:-192.168.0.147}"
BIN="build/examples/${EXAMPLE}.bin"
OUT_DIR="examples/build/${EXAMPLE}"
LUA_FILE="${OUT_DIR}/${EXAMPLE}.lua"
LAUNCHER_SRC="dump_min.lua"

# Some examples (like fb_explorer_ftp) require the full launcher context/sockets.
case "${EXAMPLE}" in
  fb_explorer_ftp) LAUNCHER_SRC="dump.lua" ;;
esac

echo "[*] Compilando ejemplo SDK: ${EXAMPLE}"
make -f Makefile.sdk_examples EXAMPLE="${EXAMPLE}" clean
make -f Makefile.sdk_examples EXAMPLE="${EXAMPLE}"

mkdir -p "${OUT_DIR}"
cp "${LAUNCHER_SRC}" "${LUA_FILE}"

echo "[*] Inyectando ${BIN} en ${LUA_FILE}..."
python3 convert.py "${BIN}" "${LUA_FILE}"

rm -f "${OUT_DIR}/last_dump.lua"

echo "[*] Lanzando payload hacia ${IP} con ${LUA_FILE}..."
python3 dump_launcher.py "${IP}" --skip-upload --launcher "${LUA_FILE}"
