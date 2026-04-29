@echo off
setlocal

set IP=%~1
if "%IP%"=="" set IP=192.168.0.147

set OUT_DIR=examples\build\hello_sdk
set LUA_FILE=%OUT_DIR%\hello_sdk.lua

echo [*] Compilando hello_sdk.bin...
make -f Makefile.hello_sdk clean || goto :err
make -f Makefile.hello_sdk || goto :err

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
copy /Y dump.lua "%LUA_FILE%" >nul || goto :err

echo [*] Inyectando hello_sdk.bin en %LUA_FILE%...
python convert.py hello_sdk.bin "%LUA_FILE%" || goto :err

echo [*] Lanzando payload hacia %IP% con %LUA_FILE%...
python dump_launcher.py "%IP%" --skip-upload --launcher "%LUA_FILE%" || goto :err

echo Done.
exit /b 0

:err
echo [!] Error en build_hello_sdk.bat
exit /b 1
