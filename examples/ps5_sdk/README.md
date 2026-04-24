# PS5 SDK Examples

Ejemplos pequenos para probar `src/ps5_sdk.h` sin depender del codigo del emulador NES.

## Ejemplos

- `hello_dialog`: abre un cuadro de texto simple con `Hola mundo` y espera a que el usuario pulse aceptar.
- `formatter_dialog`: prueba `ps5_sdk_snprintf` con `%d`, `%x` y `%p` dentro de un cuadro de texto.
- `progress_dialog`: abre una barra de progreso y actualiza texto/porcentaje.
- `kernel_info`: llama funciones simples de libkernel y muestra `pid`, `uid`, `gid`, `cpumode` y tiempo de proceso.
- `sysmodule_resolve`: resuelve funciones de `libSceSysmodule` y muestra sus punteros.
- `sysmodule_batch_info`: carga varios modulos base de una vez y muestra sus handles/retornos con un coste de memoria muy bajo.
- `audioout_resolve`: carga/resuelve `libSceAudioOut` sin abrir salida de audio todavia.
- `audioout_open_probe`: inicializa AudioOut, intenta abrir un puerto con parametros conocidos y lo cierra si abre.
- `audio_beep`: abre AudioOut, reproduce un beep corto y cierra el puerto.
- `pad_read_dialog`: inicializa `libScePad`, lee botones durante unos segundos y muestra valores crudos.
- `input_beep`: combina `libScePad` y `libSceAudioOut`; cada boton nuevo reproduce un tono distinto.
- `pad_fx_probe`: sonda ligera de `libScePad` para ver si resuelven APIs avanzadas como `scePadSetTriggerEffect`, `scePadSetLightBar` y `scePadGetControllerInformation`.
- `pad_lightbar_probe`: intento visual y ligero de cambiar el color del lightbar y restaurarlo despues.
- `pad_rgb_cycle`: ciclo corto de tres colores en el lightbar y restauracion al color original.
- `pad_trigger_probe`: primer intento conservador de `scePadSetTriggerEffect` para notar si `R2` ofrece resistencia durante un instante y luego resetear.
- `pad_info_probe`: lee los primeros campos crudos de `scePadGetControllerInformation` para entender mejor capacidades y layout del mando.
- `pad_trigger_matrix`: prueba varias disposiciones minimas de bytes para `scePadSetTriggerEffect` y compara retornos sin gastar mucha memoria.
- `pad_trigger_tune_probe`: afina solo el layout valido en offset `0` con varias intensidades/modos para intentar notar efecto real y encontrar un reset aceptado.
- `pad_trigger_ext_probe`: compara `scePadSetTriggerEffect` usando `scePadGetHandle`, `scePadOpenExt` y `scePadOpenExt2` para ver si la ruta avanzada necesita otro handle.
- `user_service_info`: consulta `libSceUserService`, lista usuarios logueados e intenta mostrar el nombre.
- `system_params_info`: consulta `libSceSystemService` y muestra idioma, formato de fecha/hora y zona horaria.
- `regmgr_read_info`: lee unas pocas claves de `libSceRegMgr` en modo solo lectura.
- `gpu_info`: consulta `libSceGnmDriver` en modo solo lectura para mostrar reloj de GPU y `userPaEnabled`.
- `sdk_dashboard`: panel seguro con `MsgDialog` que combina usuario, hora local, red, sistema y GPU en una sola vista.
- `sdk_live_launcher`: mini app interactiva con mando; menu vivo en `MsgDialog` para navegar entre dashboard, pad, audio y sistema.
- `videoout_info`: carga `libSceVideoOut` y resuelve funciones basicas sin abrir ni registrar buffers.
- `videoout_open_probe`: abre y cierra VideoOut sin registrar buffers ni hacer flips.
- `videoout_open_matrix`: prueba varias combinaciones de `sceVideoOutOpen` para encontrar una que abra en este contexto.
- `hello_square`: intenta tomar `VideoOut`, registrar un framebuffer y dibujar un cuadrado simple en pantalla.
- `hello_square_debug`: recorre el mismo camino grafico pero vuelve con un dialogo de codigos de retorno para depurar `VideoOut`.
- `net_resolve_info`: carga `libSceNet`/`libSceNetCtl` y resuelve funciones socket de libkernel sin abrir sockets.
- `socket_open_probe`: abre y cierra un socket UDP sin hacer bind ni enviar trafico.
- `net_init_probe`: inicializa `libSceNet`, prueba `sceNetSocket`, cierra y termina Net.
- `netctl_state_info`: consulta `libSceNetCtl` para leer el estado actual de red.
- `rtc_resolve_info`: carga `libSceRtc` y prueba a resolver funciones tipicas de reloj/fecha.
- `rtc_tick_info`: llama a `sceRtcGetCurrentTick` y muestra el tick actual.
- `rtc_clock_local_info`: llama a `sceRtcGetCurrentClockLocalTime` e intenta mostrar fecha/hora local.
- `mount_points_info`: prueba en modo solo lectura varios puntos de montaje comunes y muestra si se pueden abrir/listar.
- `mount_syscalls_info`: resuelve `mount` y `unmount` de `libkernel` sin intentar usarlos.
- `mount_probe_info`: resuelve helpers de montaje/FS (`mount`, `unmount`, `nmount`, `statfs`, `getfsstat`) sin invocarlos.
- `fsstat_probe`: llama a `getfsstat(NULL, 0, 0)` para pedir el numero de filesystems visibles sin montar nada.
- `statfs_probe`: prueba `statfs(path, buf)` sobre varias rutas conocidas usando dialogos simples de una en una.

## Build

```bash
make -f Makefile.sdk_examples EXAMPLE=hello_dialog
make -f Makefile.sdk_examples EXAMPLE=formatter_dialog
make -f Makefile.sdk_examples EXAMPLE=progress_dialog
```

## Estado Pad Avanzado

- `scePadSetLightBar`: validado en hardware real. `set=0` y `reset=0`; el color del mando cambia y vuelve correctamente.
- `scePadSetTriggerEffect`: la ruta buena usa `scePadGetHandle`, no `scePadOpenExt/OpenExt2`.
- `scePadSetTriggerEffect`: varias estructuras minimas ya devuelven `0`, asi que la API entra bien.
- `scePadSetTriggerEffect`: todavia no hemos encontrado una combinacion que produzca un efecto perceptible en `R2`, asi que el contenido exacto de la estructura sigue en fase de afinado.

## Lanzar

```bash
./build_sdk_example.sh hello_dialog 192.168.0.147
./build_sdk_example.sh formatter_dialog 192.168.0.147
./build_sdk_example.sh progress_dialog 192.168.0.147
./build_sdk_example.sh kernel_info 192.168.0.147
./build_sdk_example.sh sysmodule_resolve 192.168.0.147
./build_sdk_example.sh sysmodule_batch_info 192.168.0.147
./build_sdk_example.sh audioout_resolve 192.168.0.147
./build_sdk_example.sh audioout_open_probe 192.168.0.147
./build_sdk_example.sh audio_beep 192.168.0.147
./build_sdk_example.sh pad_read_dialog 192.168.0.147
./build_sdk_example.sh input_beep 192.168.0.147
./build_sdk_example.sh pad_fx_probe 192.168.0.147
./build_sdk_example.sh pad_lightbar_probe 192.168.0.147
./build_sdk_example.sh pad_rgb_cycle 192.168.0.147
./build_sdk_example.sh pad_trigger_probe 192.168.0.147
./build_sdk_example.sh pad_info_probe 192.168.0.147
./build_sdk_example.sh pad_trigger_matrix 192.168.0.147
./build_sdk_example.sh pad_trigger_tune_probe 192.168.0.147
./build_sdk_example.sh pad_trigger_ext_probe 192.168.0.147
./build_sdk_example.sh user_service_info 192.168.0.147
./build_sdk_example.sh system_params_info 192.168.0.147
./build_sdk_example.sh regmgr_read_info 192.168.0.147
./build_sdk_example.sh gpu_info 192.168.0.147
./build_sdk_example.sh sdk_dashboard 192.168.0.147
./build_sdk_example.sh videoout_info 192.168.0.147
./build_sdk_example.sh videoout_open_probe 192.168.0.147
./build_sdk_example.sh videoout_open_matrix 192.168.0.147
./build_sdk_example.sh hello_square 192.168.0.147
./build_sdk_example.sh hello_square_debug 192.168.0.147
./build_sdk_example.sh net_resolve_info 192.168.0.147
./build_sdk_example.sh socket_open_probe 192.168.0.147
./build_sdk_example.sh net_init_probe 192.168.0.147
./build_sdk_example.sh netctl_state_info 192.168.0.147
./build_sdk_example.sh rtc_resolve_info 192.168.0.147
./build_sdk_example.sh rtc_tick_info 192.168.0.147
./build_sdk_example.sh rtc_clock_local_info 192.168.0.147
./build_sdk_example.sh mount_points_info 192.168.0.147
./build_sdk_example.sh mount_syscalls_info 192.168.0.147
./build_sdk_example.sh mount_probe_info 192.168.0.147
./build_sdk_example.sh fsstat_probe 192.168.0.147
./build_sdk_example.sh statfs_probe 192.168.0.147
```

El launcher puede mostrar `FTP server not responding after 10s`; es normal en estos payloads de ejemplo porque no levantan FTP.
