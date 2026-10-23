#!/usr/bin/env python3
"""Validate repository-local Markdown links, images, and documented firmware routes."""

from pathlib import Path
import re
import sys
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
LINK_RE = re.compile(r"!?(?:\[[^\]]*\])\(([^)]+)\)")
ROUTE_RE = re.compile(r'server\.on\(\s*"([^"]+)"\s*,\s*HTTP_(GET|POST)\b')
TABLE_ROW_RE = re.compile(r"^\|\s*(GET|POST)\s*\|\s*([^|]+)\|", re.MULTILINE)


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def markdown_target(raw_target: str) -> str:
    """Remove optional Markdown title syntax and angle-bracket wrappers."""
    target = raw_target.strip()
    if target.startswith("<") and ">" in target:
        return target[1:target.index(">")]
    return target.split()[0] if target.split() else ""


def local_target_exists(markdown_path: Path, target: str) -> bool:
    parsed = urlsplit(target)
    if parsed.scheme or parsed.netloc:
        return True  # External links are outside this local, offline check.
    target_path = unquote(parsed.path)
    if not target_path:
        return True  # Fragment-only links refer to headings in the same document.
    if target_path.startswith("/"):
        return (ROOT / target_path.lstrip("/")).exists()
    return (markdown_path.parent / target_path).exists() or (ROOT / target_path).exists()


def check_markdown_links() -> None:
    broken: list[str] = []
    markdown_files = sorted(
        path for path in ROOT.rglob("*.md") if ".git" not in path.parts
    )
    if not markdown_files:
        fail("No Markdown files found to validate.")

    for markdown_path in markdown_files:
        source = markdown_path.read_text(encoding="utf-8")
        for match in LINK_RE.finditer(source):
            target = markdown_target(match.group(1))
            if not target or not local_target_exists(markdown_path, target):
                line = source.count("\n", 0, match.start()) + 1
                broken.append(
                    f"{markdown_path.relative_to(ROOT)}:{line}: {target or '(empty link)'}"
                )

    if broken:
        fail("Broken repository-local Markdown links or images:\n  " + "\n  ".join(broken))
    print(f"PASS: local Markdown links and images resolve ({len(markdown_files)} files)")


def check_readme_routes() -> None:
    sketch_path = ROOT / "ESP32-ControlOS-V6.ino"
    readme_path = ROOT / "README.md"
    sketch = sketch_path.read_text(encoding="utf-8")
    readme = readme_path.read_text(encoding="utf-8")

    registered = set(ROUTE_RE.findall(sketch))
    documented: set[tuple[str, str]] = set()
    for method, path_cell in TABLE_ROW_RE.findall(readme):
        for path in re.findall(r"/[A-Za-z0-9_-]*", path_cell):
            documented.add((method, path))

    missing = sorted(registered - documented)
    if missing:
        summary = ", ".join(f"{method} {path}" for method, path in missing)
        fail(f"README route table is missing firmware routes: {summary}")
    print(f"PASS: README route table covers all {len(registered)} registered GET/POST routes")


def main() -> None:
    check_markdown_links()
    check_readme_routes()


if __name__ == "__main__":
    main()
