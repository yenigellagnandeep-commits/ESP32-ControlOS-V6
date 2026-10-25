# Hardware Session — 25 October 2026

## Scope

This record captures what is visible in the project owner's submitted screenshots from a live ESP32 ControlOS session. It is an owner-reported demonstration record, not an independently witnessed or automated hardware test.

## Observed in the screenshots

- The dashboard displayed **ESP32 connected**.
- The device dashboard displayed live free heap, uptime, CPU frequency, flash size, firmware size, connected-client count, and access-point address.
- The Wi-Fi scan page returned **7 nearby 2.4 GHz networks**.
- The device-details page displayed hardware/runtime readings.
- The history page displayed free-heap and connected-client trends and reported 39 readings over approximately 3 minutes.
- The logs page showed sampled uptime, free heap, and client count.
- The settings page exposed the restart control and explained that access-point credentials are configured in the sketch.
- The About page identified the project as ESP32 ControlOS 6.0.

## Example readings in the captured session

| Reading | Captured value |
| --- | ---: |
| CPU frequency | 240 MHz |
| Flash size | 4 MB |
| Firmware size | 936 KB |
| Access-point address | 192.168.4.1 |
| Nearby networks found | 7 |

Free heap, uptime, connected clients, and scan results are time-dependent; these values should not be treated as fixed specifications.

## Privacy handling for public evidence

Before committing screenshots to a public repository:

- Blur the owner's personal name wherever it appears.
- Blur **all** nearby Wi-Fi network names, not just the strongest network.
- Never include Wi-Fi passphrases, access tokens, or other secrets.
- Reopen each processed image and inspect it at full resolution before publishing.

The Wi-Fi scan can reveal nearby household or business network identifiers. Keep that information obscured in public evidence.

## What this record does not prove

The screenshots alone do not prove long-duration stability, behavior across all ESP32 variants, security against every attack, or repeatability under controlled conditions. Automated compilation and repository checks are tracked separately by GitHub Actions. For future testing, record the board model, firmware commit, Arduino-ESP32 core version, test steps, expected result, actual result, and any failure details.

## Related documentation

- [Project README](../README.md)
- [Architecture and Design](ARCHITECTURE.md)
- [General hardware test report](HARDWARE_TEST_REPORT.md)
