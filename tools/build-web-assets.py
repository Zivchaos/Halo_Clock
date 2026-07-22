#!/usr/bin/env python3
"""Build deterministic, flash-resident HALO CST Web UI assets."""

from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WEBUI = ROOT / "webui"
OUTPUT = ROOT / "src" / "WebUiAssets.h"
FAVICON = ROOT / "assets" / "branding" / "halo-cst-favicon.svg"
ALERT_AREAS = WEBUI / "red-alert-areas.js"
REQUIRED = (WEBUI / "index.html", WEBUI / "styles.css", WEBUI / "app.js", ALERT_AREAS, FAVICON)
STYLE_MARKER = '<link rel="stylesheet" href="styles.css">'
SCRIPT_MARKER = '<script src="app.js" defer></script>'
RAW_DELIMITER = "HALOWEB"


def fail(message: str) -> "NoReturn":
    print(f"WEB ASSET ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def read_utf8(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as error:
        fail(f"cannot read {path.relative_to(ROOT)}: {error}")


def minify_css(source: str) -> str:
    source = re.sub(r"/\*.*?\*/", "", source, flags=re.DOTALL)
    source = re.sub(r"\s+", " ", source)
    source = re.sub(r"\s*([{}:;,>+~])\s*", r"\1", source)
    source = source.replace(";}", "}")
    return source.strip()


def minify_javascript(source: str) -> str:
    # Preserve statement boundaries and strings; remove only blank and
    # comment-only lines plus indentation. This is intentionally conservative.
    lines = []
    for line in source.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            continue
        lines.append(stripped)
    return "\n".join(lines)


def minify_html(source: str) -> str:
    source = re.sub(r"<!--.*?-->", "", source, flags=re.DOTALL)
    source = re.sub(r">\s+<", "><", source)
    return source.strip()


def raw_literal(name: str, value: str) -> str:
    terminator = f"){RAW_DELIMITER}\""
    if terminator in value:
        fail(f"{name} contains reserved raw-string terminator {terminator}")
    return f'inline const char {name}[] PROGMEM = R"{RAW_DELIMITER}({value}){RAW_DELIMITER}";'


def main() -> None:
    missing = [str(path.relative_to(ROOT)) for path in REQUIRED if not path.is_file()]
    if missing:
        fail("missing required file(s): " + ", ".join(missing))

    html = read_utf8(REQUIRED[0])
    css = read_utf8(REQUIRED[1])
    javascript = read_utf8(ALERT_AREAS) + "\n" + read_utf8(REQUIRED[2])
    favicon = read_utf8(FAVICON)

    if html.count(STYLE_MARKER) != 1 or html.count(SCRIPT_MARKER) != 1:
        fail("index.html must contain exactly one local stylesheet and script marker")

    source_bytes = sum(len(value.encode("utf-8")) for value in (html, css, javascript))
    combined = html.replace(STYLE_MARKER, f"<style>{minify_css(css)}</style>")
    combined = combined.replace(SCRIPT_MARKER, f"<script>{minify_javascript(javascript)}</script>")
    combined = minify_html(combined)
    favicon = minify_html(favicon)

    unresolved = ("styles.css", "app.js")
    found = [token for token in unresolved if token in combined]
    if re.search(r"\{\{[^{}]+\}\}", combined):
        found.append("{{template token}}")
    if found:
        fail("unresolved source token(s): " + ", ".join(found))

    digest = hashlib.sha256((combined + "\0" + favicon).encode("utf-8")).hexdigest()
    index_bytes = len(combined.encode("utf-8"))
    favicon_bytes = len(favicon.encode("utf-8"))
    header = "\n".join(
        (
            "// GENERATED FILE — DO NOT EDIT.",
            "// Source: webui/index.html, webui/styles.css, webui/app.js, and the HALO CST favicon.",
            f"// Deterministic content SHA-256: {digest}",
            "#pragma once",
            "",
            "#include <Arduino.h>",
            "",
            "namespace WebUiAssets",
            "{",
            f"    inline constexpr size_t SOURCE_BYTES = {source_bytes}U;",
            f"    inline constexpr size_t INDEX_HTML_BYTES = {index_bytes}U;",
            f"    inline constexpr size_t FAVICON_SVG_BYTES = {favicon_bytes}U;",
            f"    {raw_literal('INDEX_HTML', combined)}",
            f"    {raw_literal('FAVICON_SVG', favicon)}",
            "}",
            "",
        )
    )

    try:
        OUTPUT.write_text(header, encoding="utf-8", newline="\n")
    except OSError as error:
        fail(f"cannot write {OUTPUT.relative_to(ROOT)}: {error}")

    print(f"Web UI source bytes : {source_bytes}")
    print(f"Embedded HTML bytes : {index_bytes}")
    print(f"Embedded favicon    : {favicon_bytes}")
    print(f"Generated header    : {len(header.encode('utf-8'))}")
    print(f"Content SHA-256     : {digest}")


if __name__ == "__main__":
    main()
