@echo off
setlocal

set EXAMPLE=%~1
if "%EXAMPLE%"=="" set EXAMPLE=hello_dialog

set IP=%~2
if "%IP%"=="" set IP=192.168.0.147

set BIN=build\examples\%EXAMPLE%.bin
set OUT_DIR=examples\build\%EXAMPLE%
set LUA_FILE=%OUT_DIR%\%EXAMPLE%.lua
set LAUNCHER_SRC=dump_min.lua
if /I "%EXAMPLE%"=="fb_explorer_ftp" set LAUNCHER_SRC=dump.lua

echo [*] Compilando ejemplo SDK: %EXAMPLE%
make -f Makefile.sdk_examples EXAMPLE=%EXAMPLE% clean || goto :err
make -f Makefile.sdk_examples EXAMPLE=%EXAMPLE% || goto :err

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
copy /Y %LAUNCHER_SRC% "%LUA_FILE%" >nul || goto :err

echo [*] Inyectando %BIN% en %LUA_FILE%...
python convert.py "%BIN%" "%LUA_FILE%" || goto :err

echo [*] Lanzando payload hacia %IP% con %LUA_FILE%...
python dump_launcher.py "%IP%" --skip-upload --launcher "%LUA_FILE%" || goto :err

echo Done.
exit /b 0

:err
echo [!] Error en build_sdk_example.bat
exit /b 1
