# ESP32-ControlOS-V6

A small ESP32 access-point dashboard for monitoring device health and controlling the built-in LED.

## Hardware

- ESP32 DevKit V1 (or another ESP32 board with Wi-Fi)
- USB cable for programming

## Build and upload

1. Install the Espressif ESP32 board package in Arduino IDE.
2. Open `os_v6.ino`.
3. Before flashing, replace the default AP password in the source with a private WPA2 passphrase of at least 8 characters. The current default is public and should not be used on a deployed device.
4. Select your ESP32 board and serial port, then upload.
5. Open Serial Monitor at 115200 baud. Join the `ESP32_Control_OS` Wi-Fi network and browse to `http://192.168.4.1`.

The dashboard serves locally from the ESP32. It does not connect to a home Wi-Fi network.

## Features

- Heap, uptime, CPU frequency, flash size, and access-point client count
- Built-in LED on GPIO 2
- Wi-Fi scan on the Wi-Fi Analyzer page
- A bounded, chronological system log at `/logs`
- JSON status endpoint at `/status`

Wi-Fi scans run only when the analyzer page is opened, so refreshing the dashboard does not repeatedly interrupt radio activity.

## Security note

The source currently contains the default password `12345678`. Change it before uploading the firmware. The serial monitor reports the SSID and IP address but does not print the password.
