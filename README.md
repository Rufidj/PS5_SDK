# PS5_SDK
A lightweight, standalone C SDK for PS5 firmware 13.00. Focuses on live RAM EH_offset alignment, module handles, and direct system interaction without standard libc dependencies.

<img width="1920" height="1080" alt="breakout" src="https://github.com/user-attachments/assets/fa37f637-d62b-415a-94d2-925a3a991d82" />

https://github.com/user-attachments/assets/d0a36556-1ff1-4cdf-aa42-5b9d8fadbc89

<img width="1920" height="1080" alt="square" src="https://github.com/user-attachments/assets/36c7b924-a969-43a9-a5fa-0c40755b5cec" />

https://github.com/user-attachments/assets/b2b82ff3-529d-4d83-990e-058984e72da6

## Key Features
* **Firmware 13.00 Ready:** Tailored for the latest public offsets and kernel structures.
* **Bare Metal Approach:** No standard libc dependencies for smaller, more efficient, and stealthier payloads.
* **GPU Access:** Native rendering examples included, featuring a functional Breakout (Arkanoid) clone PoC.
* **System Interaction:** Direct handling of system dialogs (notification, progress, formatter), kernel info retrieval, and sysmodule resolving.

## Build

To compile the examples included in the SDK, use the following commands:

```bash
make -f Makefile.sdk_examples EXAMPLE=hello_dialog
make -f Makefile.sdk_examples EXAMPLE=formatter_dialog
make -f Makefile.sdk_examples EXAMPLE=progress_dialog
make -f Makefile.sdk_examples EXAMPLE=kernel_info
make -f Makefile.sdk_examples EXAMPLE=sysmodule_resolve

Launch

Use the provided helper script to compile and send the payload to your console in one step:
Bash

# Usage: ./build_sdk_example.sh <example_name> <PS5_IP>

./build_sdk_example.sh hello_dialog YOUR_PS5_IP
./build_sdk_example.sh kernel_info YOUR_PS5_IP
./build_sdk_example.sh sysmodule_resolve YOUR_PS5_IP

Special Thanks & Credits

This project stands on the shoulders of giants. Special thanks to:

    Gezine: For Luac0re, the essential Lua exploit environment that made this research possible.

    egycnq: For EmuC0re, the NES emulator port that demonstrated the true potential of homebrew on PS5.

Their work has been a technical pillar and a huge inspiration for the development of this SDK.
Acknowledgements

Part of this project's content and codebase has been developed with the assistance of various AI models, including Claude Code, ChatGPT Codex, and Gemini.
License

This project is licensed under the MIT License. See the LICENSE file for more details.
Disclaimer

This project is for educational and research purposes only. The author is not responsible for any damage caused to your hardware or software. Use at your own risk.
