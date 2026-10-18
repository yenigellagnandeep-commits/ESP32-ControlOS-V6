# ESP32 ControlOS V6

A local ESP32 access-point dashboard for device monitoring, nearby Wi-Fi analysis, recent device history, and built-in LED control.

## Features

- Device dashboard for free heap, uptime, CPU frequency, flash size, firmware size, IP address, and connected clients
- Wi-Fi analyzer that scans only when you open or refresh its page
- Recent free-heap and client history, sampled every five seconds and held in a 100-reading ring buffer (about 8 minutes 20 seconds)
- Chronological system log at `/logs`
- JSON endpoints at `/status` and `/history`
- LED control and device restart actions

## Hardware

- ESP32 DevKit V1 or another ESP32 board supported by the Espressif Arduino core
- USB cable for programming
- The sketch defaults to GPIO 2 for the LED. Some boards use a different built-in LED pin or have no built-in LED; adjust `LED_PIN` in the sketch for your hardware.

## Configure, build, and upload

1. Install the Espressif ESP32 board package in Arduino IDE.
2. Open `ESP32-ControlOS-V6.ino` from the repository folder.
3. Replace `CHANGE_ME_PRIVATE_PASSWORD` with a private WPA2 passphrase between 8 and 63 characters. The firmware refuses to start the access point while the placeholder or an invalid-length password remains.
4. Select the board that matches your hardware and its serial port, then upload.
5. Open Serial Monitor at 115200 baud. Connect to the `ESP32_Control_OS` Wi-Fi network and visit [http://192.168.4.1](http://192.168.4.1).

The ESP32 runs its own access point; it does not join your home Wi-Fi network. The dashboard is available to devices connected to that access point. Anyone connected can control the LED and restart the device, so share the passphrase only with trusted users and do not reuse another network's password.

## Routes

| Method | Path | Purpose |
| --- | --- | --- |
| GET | `/` | Device dashboard |
| GET | `/wifi` | Wi-Fi analyzer; opening the page starts a scan |
| GET | `/system` | Hardware details |
| GET | `/charts` | Recent heap and client history |
| GET | `/history` | Recent readings as JSON |
| GET | `/status` | Current device status as JSON |
| GET | `/settings` | Device settings |
| GET | `/logs` | Bounded chronological system log |
| GET | `/about` | Firmware information |
| POST | `/on` and `/off` | Turn the LED on or off |
| POST | `/restart` | Restart the ESP32 |

Scanned network names are escaped before display. Dashboard refreshes do not trigger Wi-Fi scans.

## Build check

GitHub Actions compiles the sketch for the ESP32 Dev Module when a pull request targets `main` or code is pushed to `main`. A successful build confirms compilation; test the access point and LED on the target board after uploading.

## License

MIT. See [LICENSE](LICENSE).
