# Physical Hardware Test Report

**Project:** ESP32 ControlOS V6  
**Board:** ESP32 DevKit V1  
**Test method:** Manual tests on the physical board, reported by the project owner  
**Test date / firmware commit:** Not recorded

## Results

The project owner reports that the dashboard controls and the failure-handling scenarios below were tested on the physical ESP32. These are owner-reported manual results, not results produced by the automated compile workflow.

| Area | What was tested | Result |
| --- | --- | --- |
| Dashboard and status | Dashboard loads and device readings are displayed | Tested; owner reports the test session was successful |
| LED control | ON and OFF controls | Tested; owner reports the test session was successful |
| Wi-Fi scan | Scan page and nearby-network results | Tested; owner reports the test session was successful |
| Dashboard pages and data routes | Main pages and status/history views | Tested; owner reports the test session was successful |
| Failed Wi-Fi connection | Behavior when a client cannot connect to the ESP32 access point | Tested; owner reports the test session was successful |
| Invalid requests | Requests with invalid input or authorization | Tested; owner reports the test session was successful |
| Restart | Restart control and recovery after the board reboots | Tested; owner reports the test session was successful |

## Scope and limitations

- This report records manual hardware testing reported by the project owner.
- Exact test dates, firmware commit, request examples, serial output, and per-case observations were not supplied, so they are intentionally not invented here.
- The GitHub Actions compile job checks that firmware builds for its configured target. It does not run these physical-board tests.
- For stronger reproducibility, future test sessions should record the firmware commit, test steps, expected behavior, observed behavior, and any failures or fixes.

## Re-test record

For the next hardware test, record:

- **Date:**
- **Firmware commit:**
- **Board / configuration:**
- **Issues found and fixes:**
- **Serial Monitor notes:**
