# PS5_SDK

A lightweight, standalone C SDK for PS5, featuring multi-firmware compatibility with a focus on dynamic RAM EH_offset alignment. It supports automated module handle resolution and direct system interaction without standard libc dependencies, battle-tested on firmwares 13.00 and 13.20.

![breakout](https://github.com/user-attachments/assets/fa37f637-d62b-415a-94d2-925a3a991d82)

https://github.com/user-attachments/assets/620e8cc1-ab46-4a55-8de9-f0328e8113b0

![square](https://github.com/user-attachments/assets/36c7b924-a969-43a9-a5fa-0c40755b5cec)

https://github.com/user-attachments/assets/dc456673-fcae-43e6-b2ef-631752ebc78f



## Key Features

- **Universal Firmware Compatibility:** Architected to be compatible with all firmware versions by using dynamic offset resolution.
- **Battle-Tested:** While theoretically universal, the SDK has been thoroughly tested and verified on Firmwares 13.00 and 13.20.
- **Bare Metal Approach:** No standard libc dependencies for smaller, more efficient, and stealthier payloads.
- **System Interaction:** Direct handling of system dialogs (notification, progress, formatter), kernel info retrieval, and sysmodule resolving.

## Build

Compila los ejemplos incluidos en el SDK con estos comandos:

```bash
make -f Makefile.sdk_examples EXAMPLE=hello_dialog
make -f Makefile.sdk_examples EXAMPLE=formatter_dialog
make -f Makefile.sdk_examples EXAMPLE=progress_dialog
make -f Makefile.sdk_examples EXAMPLE=kernel_info
make -f Makefile.sdk_examples EXAMPLE=sysmodule_resolve
```
## Launch
```
./build_sdk_example.sh hello_dialog YOUR_PS5_IP
./build_sdk_example.sh kernel_info YOUR_PS5_IP
./build_sdk_example.sh sysmodule_resolve YOUR_PS5_IP
```

## Special Thanks & Credits

This project stands on the shoulders of giants. Special thanks to:

    Gezine: For [LuaC0re](https://github.com/Gezine/Luac0re), the essential Lua exploit environment that made this research possible.

    egycnq: For [EmuC0re](hhttps://github.com/egycnq/EmuC0re), the NES emulator port that demonstrated the true potential of homebrew on PS5.

Their work has been a technical pillar and a huge inspiration for the development of this SDK.
Acknowledgements

Part of this project's content and codebase has been developed with the assistance of various AI models, including Claude Code, ChatGPT Codex, and Gemini.
License

This project is licensed under the MIT License. See the LICENSE file for more details.
Disclaimer

This project is for educational and research purposes only. The author is not responsible for any damage caused to your hardware or software. Use at your own risk.
