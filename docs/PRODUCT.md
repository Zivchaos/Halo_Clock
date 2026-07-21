# HALO CST product definition

**Repository-facing name:** HALO CST — Connected Smart Timepiece

“CST” is deliberately dual-purpose: it expands to Connected Smart Timepiece and evokes the language of time standards.

## Promise

Time should be beautiful, readable, calm, and locally controlled. The LED ring communicates at a glance; the OLED provides exact context.

## Principles

1. The clock continues to function when optional network services fail.
2. Normal interaction is non-blocking and available from both the button and local Web UI.
3. Saved choices survive reboot and compatible firmware upgrades.
4. Selected intent remains distinct from temporary effective behavior.
5. Network-facing features are explicit about their trusted-LAN security model.
6. Hardware, behavior, and product identity are centralized and documented.

## Release identity

- Product: HALO CST
- Expanded name: Connected Smart Timepiece
- Firmware: 1.0.0
- Hostname and OTA target: `halo-cst`
- Setup access point: `HALO-CST-Setup`
- Local dashboard: `http://halo-cst.local`

These values are centralized in `include/Version.h`. Internal class names such as `Halo` remain stable to avoid cosmetic refactors and regression risk.

## Current scope

The 1.0 release includes synchronized time, four display modes, automatic NIGHT, persistent settings, ring calibration, WiFiManager provisioning, ArduinoOTA, a local Web UI/API, cached current weather, and RAM-only diagnostics.

It deliberately excludes cloud telemetry, forecasts, alarms, browser firmware upload, authentication, filesystem storage, and extra display modes.
