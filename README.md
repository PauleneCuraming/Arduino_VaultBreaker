# Arduino VaultBreaker

**Arduino VaultBreaker** is an ESP32-S3-based electronic safe project developed with the Arduino framework and PlatformIO. The firmware brings together several hardware inputs and outputs for an interactive safe-cracking experience.

This repository contains the **firmware source code**. Running the project on a physical device requires a compatible ESP32-S3 board, the connected components, and wiring that matches the pin definitions in `src/main.cpp`.

## Project overview

The firmware uses an ESP32-S3 microcontroller and integrates an I²C LCD, servo control, a multiplexed seven-segment display, and additional inputs and outputs. It also includes Wi-Fi and web-server libraries. Refer to `src/main.cpp` for the implemented behavior and configuration.

## Hardware

The source code and dependencies indicate the following components or interfaces:

| Component / interface | Role |
| --- | --- |
| ESP32-S3 development board | Runs the firmware |
| I²C LCD display | Displays text or status information |
| Servo motor | Provides mechanical movement |
| Four-digit seven-segment display | Displays numeric information |
| Ultrasonic sensor (trigger and echo) | Distance-sensing input |
| Potentiometer | Analog input |
| Push button | Digital input |
| Buzzer | Sound output |
| Touch input | Touch-sensitive input |
| Wi-Fi | Network connectivity used by the firmware |

**Important:** Check the actual hardware model, power requirements, wiring, and the complete pin assignments in `src/main.cpp` before connecting components. The source code is the reference for this particular build; this README is not a wiring diagram.

## Software requirements

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode) or [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html)
- Git (optional, for cloning)
- A USB data cable and a compatible ESP32-S3 board (required to upload and run on hardware)

## Download the project

Clone the repository:

```bash
git clone https://github.com/PauleneCuraming/Arduino_VaultBreaker.git
```

Or choose **Code → Download ZIP** on the [GitHub repository](https://github.com/PauleneCuraming/Arduino_VaultBreaker), then extract the ZIP file.

Open the folder that directly contains `platformio.ini` in VS Code.

## Build with PlatformIO

1. Open the project folder in VS Code.
2. Install the PlatformIO IDE extension if needed, and allow its setup to finish.
3. Open `platformio.ini` and confirm the environment matches your actual board.
4. Open **Terminal → New Terminal** in the project folder.
5. Run:

```bash
pio run -e esp32-s3-devkitm-1
```

A successful build ends with `SUCCESS`. Building checks whether the firmware compiles; it does **not** run the safe on your computer.

If PowerShell does not recognize `pio`, but PlatformIO Core is installed in the default Windows location, run:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e esp32-s3-devkitm-1
```

## Upload to the ESP32-S3

1. Verify the physical board matches the environment selected in `platformio.ini`.
2. Connect the board using a USB data cable.
3. Check the detected serial devices:

```bash
pio device list
```

4. Upload the firmware:

```bash
pio run -e esp32-s3-devkitm-1 -t upload
```

If multiple serial ports are available, identify the correct one before uploading. Do not use the DevKitM-1 environment for a different board without confirming compatibility.

The `platformio.ini` also defines a `4d_systems_esp32s3_gen4_r8n16` environment. Select that environment only if it matches your hardware.

## Libraries

PlatformIO manages the dependencies declared in `platformio.ini`:

- `marcoschwartz/LiquidCrystal_I2C@^1.1.4`
- `madhephaestus/ESP32Servo@^3.1.3`
- `deanisme/SevSeg@^3.7.2`

The firmware also uses Arduino framework libraries such as `WiFi`, `WebServer`, and `Wire`.

## Project structure

```text
SafeCracker-main/
├── include/
├── lib/
├── src/
│   └── main.cpp       # Main firmware source
├── test/
├── .gitignore
└── platformio.ini     # Board environments and dependencies
```

PlatformIO generates a local `.pio/` directory for build artifacts. You do not need to commit that directory to GitHub.

## Troubleshooting

- **`pio` is not recognized:** Use PlatformIO's Build command in VS Code, or invoke the Windows executable as shown above.
- **Build fails:** Check the error message, board environment, and dependencies.
- **No device appears in `pio device list`:** Check the USB data cable, drivers, and Windows Device Manager.
- **Upload fails:** Verify the selected board and port, and follow your board's instructions for entering download mode if required.

## Notes

- Firmware compilation has been verified for the `esp32-s3-devkitm-1` environment; hardware operation has not been verified here.
- Do not commit real Wi-Fi passwords, tokens, or other secrets to this public repository. Review `src/main.cpp` before publishing updates.
- Pin assignments and operating instructions should be checked against the physical prototype before use.

## Repository

[PauleneCuraming/Arduino_VaultBreaker](https://github.com/PauleneCuraming/Arduino_VaultBreaker)
