# Security policy

## Protect your access point password

The ESP32 access point protects its dashboard with WPA2. Anyone who knows that password can view the dashboard, control the configured LED, and restart the board. Use a passphrase that is at least 8 characters long and do not reuse a password from another network.

Keep config.local.h private. It is ignored by Git. Do not paste the passphrase into a public issue, pull request, screenshot, or serial log.

The dashboard is designed for a trusted local Wi-Fi connection. State-changing forms include a random per-boot token to help block cross-site form submissions. The token does not create separate accounts: people connected to the ESP32 access point can still load the dashboard and use its controls.

## Reporting a security problem

Please report security problems privately to the repository maintainer. If GitHub's private vulnerability reporting is enabled for this repository, use that feature. Do not publish passwords or other secrets in an issue.
