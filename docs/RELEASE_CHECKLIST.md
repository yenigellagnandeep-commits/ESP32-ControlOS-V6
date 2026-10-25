# Release Checklist

Use this checklist before tagging a release or describing a build as ready for others.

## Source and build

- [ ] Choose the exact firmware commit to release.
- [ ] Confirm the root and single-file sketches are synchronized.
- [ ] Confirm the GitHub Actions build and repository checks pass.
- [ ] Record the ESP32 Arduino core version and selected board target.
- [ ] Confirm the access-point SSID is intentional and the passphrase is a private WPA2 passphrase of 8–63 characters.
- [ ] Confirm no local configuration, passwords, tokens, or personal data are staged.

## Physical-board verification

- [ ] Upload the exact release commit to the intended board.
- [ ] Confirm the access point starts and the dashboard opens at `http://192.168.4.1`.
- [ ] Confirm status readings load and refresh.
- [ ] Confirm Wi-Fi scan returns results and the page remains usable.
- [ ] Confirm LED ON/OFF behavior for the board's configured pin and polarity.
- [ ] Confirm restart works and the board becomes reachable again.
- [ ] Record failures, board model, firmware commit, and observations in the hardware test report.

## Public evidence and privacy

- [ ] Use screenshots from the release firmware, not a mockup.
- [ ] Blur personal names and all nearby Wi-Fi network names.
- [ ] Inspect the full-resolution images after blurring.
- [ ] Check screenshots for passwords, tokens, browser account details, serial numbers, and private notifications.
- [ ] Verify every image link in the README opens correctly.

## Publish

- [ ] Update the changelog/release notes with actual changes.
- [ ] Include known limitations and board-specific assumptions.
- [ ] Create a version tag only after the checks above are complete.
