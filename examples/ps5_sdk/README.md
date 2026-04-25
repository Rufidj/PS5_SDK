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
- `pad_lightbar_live`: ejemplo interactivo; los botones cambian el color del lightbar del mando durante unos segundos.
- `pad_rgb_cycle`: ciclo corto de tres colores en el lightbar y restauracion al color original.
- `pad_party`: mezcla interactiva de pad + lightbar + beep; cada boton nuevo cambia el color y reproduce un tono.
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
- `videoout_host_state`: inspecciona el estado bruto del host (`EBOOT_VIDOUT`, `EBOOT_GS_THREAD`) y cuantas funciones base de `VideoOut` estan resueltas.
- `videoout_gs_probe`: prueba de forma aislada si `scePthreadCancel` puede parar el thread grafico host sin tocar buffers ni flips.
- `videoout_takeover_probe`: replica la secuencia de `main.c`/`main_nes.c`: cancelar `gs`, cerrar `emu_vid` y probar `sceVideoOutOpen` con fallback de tipos, sin registrar buffers todavia.
- `videoout_takeover_reg_probe`: da un paso mas en la misma secuencia de takeover y prueba `AllocateDirectMemory + MapDirectMemory + RegisterBuffers`, pero todavia sin `SubmitFlip`.
- `videoout_takeover_flip_probe`: extiende el takeover bueno hasta `SetFlipRate + SubmitFlip`, pintando un framebuffer minimo para intentar el primer render real del SDK.
- `fb_animation`: animacion suave de un cuadrado rebotante con ciclo de color completo (hue 0→360); doble buffer real alternando indices 0/1, 360 frames a ~60 fps. Validado en hardware.
- `fb_pad_draw`: control interactivo del framebuffer con el mando; D-pad mueve un cuadrado por la pantalla, botones de cara (CROSS/CIRCLE/TRIANGLE/SQUARE/L1/R1) cambian su color, CROSS para salir (timeout 15 s). Validado en hardware.
- `fb_text`: animacion con HUD en pantalla usando `ps5sdk_fb_str` y `ps5sdk_fb_dec`; muestra frame counter y coordenadas X/Y actualizandose en vivo. Validado en hardware.
- `arkanoid`: juego completo — ladrillos de 6 colores, bola con fisicas de rebote, paleta controlada con D-pad, puntuacion por fila, 3 vidas, sonidos de beep al chocar con bola/ladrillo/vida perdida; CROSS lanza/reinicia, OPTIONS sale. Requiere libScePad + libSceVideoOut + libSceAudioOut.
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
./build_sdk_example.sh hello_dialog <IP_DE_PS5>
./build_sdk_example.sh formatter_dialog <IP_DE_PS5>
./build_sdk_example.sh progress_dialog <IP_DE_PS5>
./build_sdk_example.sh kernel_info <IP_DE_PS5>
./build_sdk_example.sh sysmodule_resolve <IP_DE_PS5>
./build_sdk_example.sh sysmodule_batch_info <IP_DE_PS5>
./build_sdk_example.sh audioout_resolve <IP_DE_PS5>
./build_sdk_example.sh audioout_open_probe <IP_DE_PS5>
./build_sdk_example.sh audio_beep <IP_DE_PS5>
./build_sdk_example.sh pad_read_dialog <IP_DE_PS5>
./build_sdk_example.sh input_beep <IP_DE_PS5>
./build_sdk_example.sh pad_fx_probe <IP_DE_PS5>
./build_sdk_example.sh pad_lightbar_probe <IP_DE_PS5>
./build_sdk_example.sh pad_lightbar_live <IP_DE_PS5>
./build_sdk_example.sh pad_rgb_cycle <IP_DE_PS5>
./build_sdk_example.sh pad_party <IP_DE_PS5>
./build_sdk_example.sh pad_trigger_probe <IP_DE_PS5>
./build_sdk_example.sh pad_info_probe <IP_DE_PS5>
./build_sdk_example.sh pad_trigger_matrix <IP_DE_PS5>
./build_sdk_example.sh pad_trigger_tune_probe <IP_DE_PS5>
./build_sdk_example.sh pad_trigger_ext_probe <IP_DE_PS5>
./build_sdk_example.sh user_service_info <IP_DE_PS5>
./build_sdk_example.sh system_params_info <IP_DE_PS5>
./build_sdk_example.sh regmgr_read_info <IP_DE_PS5>
./build_sdk_example.sh gpu_info <IP_DE_PS5>
./build_sdk_example.sh sdk_dashboard <IP_DE_PS5>
./build_sdk_example.sh videoout_host_state <IP_DE_PS5>
./build_sdk_example.sh videoout_gs_probe <IP_DE_PS5>
./build_sdk_example.sh videoout_takeover_probe <IP_DE_PS5>
./build_sdk_example.sh videoout_takeover_reg_probe <IP_DE_PS5>
./build_sdk_example.sh videoout_takeover_flip_probe <IP_DE_PS5>
./build_sdk_example.sh fb_animation <IP_DE_PS5>
./build_sdk_example.sh fb_pad_draw <IP_DE_PS5>
./build_sdk_example.sh fb_text <IP_DE_PS5>
./build_sdk_example.sh arkanoid <IP_DE_PS5>
./build_sdk_example.sh videoout_info <IP_DE_PS5>
./build_sdk_example.sh videoout_open_probe <IP_DE_PS5>
./build_sdk_example.sh videoout_open_matrix <IP_DE_PS5>
./build_sdk_example.sh hello_square <IP_DE_PS5>
./build_sdk_example.sh hello_square_debug <IP_DE_PS5>
./build_sdk_example.sh net_resolve_info <IP_DE_PS5>
./build_sdk_example.sh socket_open_probe <IP_DE_PS5>
./build_sdk_example.sh net_init_probe <IP_DE_PS5>
./build_sdk_example.sh netctl_state_info <IP_DE_PS5>
./build_sdk_example.sh rtc_resolve_info <IP_DE_PS5>
./build_sdk_example.sh rtc_tick_info <IP_DE_PS5>
./build_sdk_example.sh rtc_clock_local_info <IP_DE_PS5>
./build_sdk_example.sh mount_points_info <IP_DE_PS5>
./build_sdk_example.sh mount_syscalls_info <IP_DE_PS5>
./build_sdk_example.sh mount_probe_info <IP_DE_PS5>
./build_sdk_example.sh fsstat_probe <IP_DE_PS5>
./build_sdk_example.sh statfs_probe <IP_DE_PS5>
```

El launcher puede mostrar `FTP server not responding after 10s`; es normal en estos payloads de ejemplo porque no levantan FTP.
