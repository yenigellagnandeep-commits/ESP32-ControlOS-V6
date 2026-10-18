#pragma once

// Copy this file to config.local.h and set your own Wi-Fi name and passphrase.
// config.local.h is ignored by Git so your passphrase stays out of commits.
#define CONTROL_OS_WIFI_SSID "ESP32_Control_OS"
#define CONTROL_OS_WIFI_PASSWORD "CHANGE_ME_PRIVATE_PASSWORD"

// GPIO 2 is common on ESP32 DevKit boards. Change these for your board if needed.
#define CONTROL_OS_LED_PIN 2
#define CONTROL_OS_LED_ACTIVE_LOW 0
