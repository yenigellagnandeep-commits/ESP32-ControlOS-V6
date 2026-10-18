# ESP32 ControlOS V6

[![ESP32 firmware build](https://github.com/yenigellagnandeep-commits/ESP32-ControlOS-V6/actions/workflows/esp32-build.yml/badge.svg?branch=main)](https://github.com/yenigellagnandeep-commits/ESP32-ControlOS-V6/actions/workflows/esp32-build.yml)

A private, local dashboard for monitoring and controlling an ESP32 board. It runs on the ESP32 itself and does not need an internet connection, external sensors, or an OLED display.

## What it does

- Shows live free memory, uptime, CPU frequency, flash size, firmware size, connected clients, and access point address.
- Scans nearby 2.4 GHz Wi-Fi networks only when you open the Wi-Fi scan page.
- Draws recent free-memory and client-count trends from readings held in RAM.
- Controls the board's configured built-in LED and can restart the board.
- Serves its pages and JSON data locally, without sending readings to a cloud service.

## Hardware

- An ESP32 board supported by the Espressif Arduino core.
- A USB cable for setup and uploading.

The default LED setting is GPIO 2, which is common on ESP32 DevKit boards. LED pins and polarity vary by board; some boards do not have a built-in LED. Use the settings in config.local.h if your board needs a different pin or active-low setting.

## Set up a private Wi-Fi password

1. Open the sketch folder.
2. Copy config.example.h to a new file named config.local.h.
3. In config.local.h, change CONTROL_OS_WIFI_PASSWORD to a private passphrase that is 8 to 63 characters long. You may also change the network name.
4. Keep config.local.h private. Git ignores this file so it is not included in commits.

The example file intentionally contains a placeholder password. If you forget to make a private configuration file, the firmware refuses to start the access point.

## Build and upload

1. Install the Espressif ESP32 board package in Arduino IDE.
2. Open ESP32-ControlOS-V6.ino from this folder.
3. Select the board and serial port that match your hardware. The automated build uses **ESP32 Dev Module**.
4. Connect the board by USB and upload the sketch.
5. Open Serial Monitor at **115200 baud**. It prints the access point name and local address, but never prints the passphrase.
6. Connect your phone or computer to the ESP32 Wi-Fi network and visit [http://192.168.4.1](http://192.168.4.1).

### Optional single-file sketch

`single-file/ESP32-ControlOS-V6-SingleFile/ESP32-ControlOS-V6-SingleFile.ino` is available if you prefer one sketch file. Open the `single-file/ESP32-ControlOS-V6-SingleFile` folder in Arduino IDE, then replace the SSID and password constants near the top before uploading. The committed password is a placeholder, and the access point will not start until you set a private WPA2 passphrase that is 8 to 63 characters long.

The ESP32 creates its own password-protected Wi-Fi network. It does not connect to your home router or to the networks shown by the scan page. While connected to the ESP32 network, your phone or computer may show that there is no internet connection; that is expected.

## Pages and routes

| Method | Path | Purpose |
| --- | --- | --- |
| GET | / | Live device dashboard |
| GET | /wifi | Scan nearby Wi-Fi networks |
| GET | /system | Device details |
| GET | /charts | Automatically refreshed history chart |
| GET | /logs | Recent readings in time order |
| GET | /settings | Restart the board and view setup note |
| GET | /about | Features, privacy, and project information |
| GET | /status | Current readings as JSON |
| GET | /history | Recent readings as JSON |
| POST | /on and /off | Turn the configured LED on or off |
| POST | /restart | Restart the ESP32 |

Wi-Fi names are escaped before display. Signal is shown as the radio's measured dBm value; the page does not invent a signal-quality percentage. Dashboard polling does not start additional Wi-Fi scans.

## History and storage

The ESP32 keeps up to 100 samples in memory, normally one sample every five seconds. That is about eight minutes and twenty seconds of readings. A Wi-Fi scan can briefly delay sampling. The readings are not saved permanently and are cleared when the board restarts.

## Safety and privacy

The WPA2 access point is the network access gate. State-changing forms also use a random token created at boot to help block cross-site form submissions. Anyone connected to the access point can still load the dashboard and use its controls, so share its password only with people you trust. Do not reuse your home Wi-Fi password.

The firmware does not send device readings to a cloud service. The nearby network scan displays names and radio measurements only on the local dashboard.

## Troubleshooting

- **The access point does not appear:** Check Serial Monitor. The firmware will not start until the placeholder is replaced with a passphrase that is 8–63 characters long. Also check that the board started successfully.
- **The dashboard does not open:** Connect to the ESP32's own Wi-Fi network and open http://192.168.4.1.
- **The LED does not respond:** Your board may use another pin or active-low wiring. Set CONTROL_OS_LED_PIN and CONTROL_OS_LED_ACTIVE_LOW in your private configuration file.
- **The Wi-Fi scan is empty:** The ESP32 scan covers 2.4 GHz networks. Try again closer to the router.

## Build check

GitHub Actions compiles the sketch for **ESP32 Dev Module** on pull requests to main and pushes to main. A passing build confirms that the sketch compiles for that target. Check LED behavior and other board-specific features on the actual ESP32 before relying on them.

## Hardware verification

**Project-owner hardware test (2026-10-18):** The firmware was uploaded and run on a physical ESP32. The screenshots below were captured during that session and document the pages and readings visible at the time. They do not establish that every control was independently tested.

> **Screenshot context:** The Settings screenshot reflects the single-file sketch’s credentials-at-the-top setup note; the main sketch uses the ignored `config.local.h` file described above. The images are preserved unchanged. The non-dashboard connection badge shown in these captures was corrected afterward.

### Dashboard

![ESP32 dashboard with live readings and LED controls](docs/screenshots/01-dashboard.png)

### Nearby Wi-Fi scan

![Nearby Wi-Fi scan showing network names, signal strength, channels, and security](docs/screenshots/02-wifi-scan.png)

### Device details

![ESP32 device details and hardware readings](docs/screenshots/03-device-details.png)

### Recent history

![Chart of ESP32 memory and connected-client readings](docs/screenshots/04-history.png)

### Recent logs

![Table of ESP32 uptime, free-memory, and connected-client samples](docs/screenshots/05-logs.png)

### Settings

![ESP32 settings page with the device restart control](docs/screenshots/06-settings.png)

### About

![About page describing ESP32 ControlOS features and privacy](docs/screenshots/07-about.png)

## License

MIT. See [LICENSE](LICENSE).
