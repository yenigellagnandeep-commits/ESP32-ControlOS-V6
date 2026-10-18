#!/usr/bin/env python3
"""Guard the two ESP32 sketches against functional drift and security regressions."""

from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def normalize_sketch(source: str, config_marker: str) -> str:
    start = source.find(config_marker)
    end = source.find("namespace", start)
    if start < 0 or end < 0:
        fail(f"Could not locate configuration preamble marker: {config_marker!r}")

    source = source[:start] + "// CONFIGURATION PREAMBLE\n" + source[end:]
    substitutions = (
        (
            "The network name and passphrase are set in your private <code>config.local.h</code> file.",
            "The network name and passphrase are configured before upload.",
        ),
        (
            "The network name and passphrase are set at the top of this sketch.",
            "The network name and passphrase are configured before upload.",
        ),
        (
            "Access point not started. Set a private SSID and a WPA2 passphrase of 8-63 characters in config.local.h.",
            "Access point not started. Set a private SSID and a WPA2 passphrase of 8-63 characters before upload.",
        ),
        (
            "Access point not started. Set a private SSID and a WPA2 passphrase of 8-63 characters in the Wi-Fi settings at the top of the sketch.",
            "Access point not started. Set a private SSID and a WPA2 passphrase of 8-63 characters before upload.",
        ),
    )
    for old, new in substitutions:
        source = source.replace(old, new)
    return source


def main() -> None:
    main_path = ROOT / "ESP32-ControlOS-V6.ino"
    single_path = ROOT / "ESP32-ControlOS-V6-SingleFile" / "ESP32-ControlOS-V6-SingleFile.ino"
    ignore_path = ROOT / ".gitignore"
    example_path = ROOT / "config.example.h"

    for path in (main_path, single_path, ignore_path, example_path):
        if not path.is_file():
            fail(f"Required project file is missing: {path.relative_to(ROOT)}")

    main_source = main_path.read_text(encoding="utf-8")
    single_source = single_path.read_text(encoding="utf-8")
    ignore_source = ignore_path.read_text(encoding="utf-8")
    example_source = example_path.read_text(encoding="utf-8")

    if "config.local.h" not in ignore_source.splitlines():
        fail(".gitignore must ignore config.local.h")
    ignored = subprocess.run(
        ["git", "check-ignore", "-q", "config.local.h"],
        cwd=ROOT,
        check=False,
    )
    if ignored.returncode != 0:
        fail("config.local.h must be ignored by Git")
    tracked = subprocess.run(
        ["git", "ls-files", "--error-unmatch", "config.local.h"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=False,
    )
    if tracked.returncode == 0:
        fail("config.local.h must never be tracked or committed")

    if "CHANGE_ME_PRIVATE_PASSWORD" not in example_source:
        fail("config.example.h must use the documented placeholder password")

    for label, source in (("main", main_source), ("single-file", single_source)):
        for required in (
            "hasValidAccessPointConfig()",
            'strcmp(kAccessPointPassword, kPlaceholderPassword) != 0',
            "hasValidControlToken()",
            "escapeHtml(scannedName)",
            "WiFi.scanDelete()",
        ):
            if required not in source:
                fail(f"{label} sketch is missing required safety behavior: {required}")

        for function in ("void ledOn()", "void ledOff()", "void restartEsp()"):
            start = source.find(function)
            if start < 0:
                fail(f"{label} sketch is missing {function}")
            end = source.find("\nvoid ", start + len(function))
            body = source[start:end if end >= 0 else len(source)]
            if "hasValidControlToken()" not in body:
                fail(f"{label} sketch's {function} handler is missing token validation")

    normalized_main = normalize_sketch(
        main_source, '#if __has_include("config.local.h")'
    )
    normalized_single = normalize_sketch(
        single_source, "// Single-file Wi-Fi configuration"
    )
    if normalized_main != normalized_single:
        fail(
            "Main and single-file sketches have drifted beyond the documented "
            "configuration/setup text differences. Keep shared firmware logic synchronized."
        )

    print("PASS: private config remains ignored and untracked")
    print("PASS: both sketches retain password, control-token, and Wi-Fi-name protections")
    print("PASS: main and single-file firmware logic are synchronized")


if __name__ == "__main__":
    main()
