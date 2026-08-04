# Changelog

Notable project milestones are recorded here. HALO CST follows semantic versioning for stable public releases.

## [1.1.0-israel] — 2026-08-04

### Stable Israeli Red Alert release

- Promoted the optional Israeli Red Alert companion after a 48-hour live soak
  with selected areas retained, responsive local Web UI, stable clock modes,
  and no observed LED glitches.
- Added configurable supported LED, OLED, and button pins to the local Web UI
  with validation, conflict checks, and reboot confirmation.
- Refreshed the local Web UI with glass styling, a balanced overview, weather
  and heap cards, and a one-second Live Clock repaint independent of the
  three-second status polling interval.
- Published the dedicated Israeli release as a stable tag. Red Alert remains
  disabled by default and is always a visual aid only.

## [1.1.0-israel-rc2] â€” 2026-07-30

### Release automation

- Corrected immutable-release publishing order: build the tagged source,
  attach the firmware to a draft release, then publish the release.

## [1.1.0-israel-rc1] â€” 2026-07-30

### Red Alert hardening checkpoint

- Kept the optional Israeli Red Alert companion disabled by default, with
  persisted selected alert areas and a configurable HTTPS relay.
- Corrected request cleanup so successful live checks release their TLS and
  response resources before the worker task exits.
- Added request counts, success/failure totals, duration, recovered heap, and
  per-request low-heap telemetry to the local Web UI.
- Added a minute-transition request guard and isolated live TLS work on the
  ESP32 networking core to protect CLASSIC LED rendering.
- Added automated coverage for polling gates, retry timing, low-heap safety,
  minute-transition deferral, timer wraparound, parsing, and default-off
  behavior.
- Completed a 24-hour manually enabled live soak: 5,028 successful relay
  checks out of 5,050 (99.56%), stable clock/Web UI behavior, saved zones,
  and no LED glitches.

See [docs/RED_ALERT.md](docs/RED_ALERT.md) for operating limits and the
official-alert disclaimer.

## [1.0.0] — 2026-07-20

HALO CST 1.0.0 is the first public release of the Connected Smart Timepiece: a local-first ESP32 clock with a custom LED display engine, responsive embedded WebUI, persistent settings, weather, diagnostics, and OTA support.

### New features

- Added a responsive, device-hosted WebUI with a live overview, display controls, network configuration, weather, diagnostics, and trusted-LAN safety guidance.
- Added runtime LED-ring calibration: configurable physical 12 o’clock offset, clockwise/counterclockwise direction, a four-color cardinal test, validation, persistence, and WebUI/API controls.
- Added persisted weather-location configuration with validated WGS84 coordinates and Open-Meteo refresh integration.
- Added the CUSTOM display mode with six persisted LED color controls.

### Improvements

- Established the HALO CST visual identity with octagonal marks, favicon, social card, brand guide, and a WebUI that reflects the physical prototype.
- Reworked the README as a product-facing guide with real, privacy-redacted WebUI captures, quick start, hardware overview, configuration, testing, and security guidance.
- Improved API validation and persisted settings handling for calibration, custom colors, and weather location.
- Added architecture and WebUI-design documentation, while preserving existing wiring, network-recovery, partition-migration, contribution, and security references.

### Testing and release engineering

- Added WebUI source checks and public-release metadata checks.
- Validated the release with 39 WebUI source checks, 46 public-release checks, and 38 embedded test cases across weather, diagnostics, and network configuration.
- Confirmed a successful ESP32 firmware build using the dual-OTA 4 MiB partition layout.

### Repository consolidation

- Consolidated the branded WebUI, calibration, weather-location, and CUSTOM-mode work through the integration branch.
- Removed superseded WebUI, calibration, and branding feature branches after merge, while preserving all milestone tags.
- Removed the unused `faceplates/arabic.svg` asset and kept release imagery free of private network addresses and personal coordinates.

### Known limitations

- HALO CST remains a trusted-LAN device: keep the WebUI local, and never expose its ports publicly. ArduinoOTA is disabled by default and requires its administrator password for each enabled session.
- Hardware enclosure photographs are not yet part of this release; the documented enclosure remains a maker prototype.
- The supplied WebUI captures are representative desktop views; mobile and physical-prototype imagery can be added in a later release.

## [1.0.0-rc1] — 2026-07-15

- Renamed the public product to HALO CST — Connected Smart Timepiece.
- Centralized product identity, version, hostname, and setup AP metadata.
- Changed the local/OTA hostname to `halo-cst` and setup AP to `HALO-CST-Setup`.
- Replaced installation-specific weather coordinates with documented public example values.
- Added public build, wiring, security, contribution, migration, and release documentation.
- Preserved the legacy `halo-clock` Preferences namespace for upgrade compatibility.

## [0.6.0-diagnostics] — 2026-07-15

- Added RAM-only health counters, `/api/diagnostics`, and Web UI copy/download actions.

## [0.5.1-expanded-ota] — 2026-07-15

- Replaced unused SPIFFS with two equal 2,031,616-byte OTA slots while retaining NVS, OTA metadata, and coredump.

## [0.5.0-weather] — 2026-07-15

- Added keyless Open-Meteo current conditions, validated cache/stale behavior, Web UI weather, and a temporary OLED view.

## [0.4.0-web-ui] — 2026-07-14

- Added a responsive trusted-LAN dashboard and validated local control/status API.

## [0.3.0-ota] — 2026-07-14

- Added ArduinoOTA with non-blocking OLED and 60-LED progress feedback.

## [0.2.0-display-modes] — 2026-07-14

- Added persistent CLASSIC, MINIMAL, and NIGHT modes with correct normal-brightness restoration.

## [0.1.0-working-clock] — 2026-07-13

- Established the synchronized OLED/LED working clock baseline on ESP32-WROOM, SH1106, and 60 WS2812B pixels.
