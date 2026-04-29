@echo off
setlocal

if "%~1"=="" (
  echo [?] Uso: build_dump.bat ^<IP_PS5^>
  exit /b 0
)

set IP=%~1
set OUT_DIR=examples\build\master_dump
set LUA_FILE=%OUT_DIR%\master_dump.lua

echo [*] Compilando master_dump.bin...
make -f Makefile.dump clean || goto :err
make -f Makefile.dump || goto :err

if not exist master_dump.bin (
  echo [!] Error en la compilacion.
  exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
copy /Y dump.lua "%LUA_FILE%" >nul || goto :err

echo [*] Inyectando binario en %LUA_FILE%...
python convert.py master_dump.bin "%LUA_FILE%" || goto :err

echo [*] Lanzando payload hacia %IP% con %LUA_FILE%...
python dump_launcher.py "%IP%" --skip-upload --launcher "%LUA_FILE%" || goto :err

echo Done.
exit /b 0

:err
echo [!] Error en build_dump.bat
exit /b 1
