# Contributing

Thanks for helping improve ESP32 ControlOS.

## Keep the project focused

This project is a dashboard for the ESP32 board itself. Please keep new features within that scope and document any board-specific assumptions, especially around the built-in LED.

## Before opening a pull request

- Keep Wi-Fi credentials out of tracked files. Use the ignored config.local.h file for your own passphrase.
- Describe what changed and which ESP32 board you used.
- Check that the GitHub Actions firmware build passes.
- Test hardware-specific behavior on a real board before saying that it works on all ESP32 boards.

The automated build targets the ESP32 Dev Module. A successful build checks compilation; it cannot confirm the behavior of every board's LED or radio.
