# Screenshot Guide

Screenshots are useful portfolio evidence when they are real, readable, and privacy-safe.

## Capture checklist

1. Open the dashboard served by the ESP32 and wait for live readings to load.
2. Capture the dashboard, Wi-Fi scan, device details, history, logs, settings, and About pages as separate images.
3. Use a consistent browser zoom and crop unnecessary browser chrome where practical.
4. Blur the author's personal name wherever it appears.
5. Blur **every** nearby Wi-Fi network name in the scan table. Network names can identify households or organizations.
6. Inspect each image at full resolution to confirm the blur is complete.
7. Do not show access-point passphrases, tokens, serial numbers, account information, or private notifications.
8. Save final images in `docs/screenshots/` using stable descriptive names:
   - `01-dashboard.png`
   - `02-wifi-scan.png`
   - `03-device-details.png`
   - `04-history.png`
   - `05-logs.png`
   - `06-settings.png`
   - `07-about.png`

## Evidence integrity

Do not edit readings to make the hardware look better, fabricate test results, or present a design mockup as a live device. Cropping and privacy blurring are appropriate; the remaining interface and readings should represent the captured session.

When a screenshot set is replaced, open every README image link and verify that the image is the intended sanitized file.
