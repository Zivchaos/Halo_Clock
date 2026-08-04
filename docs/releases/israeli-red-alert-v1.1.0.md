# HALO CST Israeli Red Alert v1.1.0

This stable release packages HALO CST's optional Israeli Red Alert companion on
the dedicated Israeli firmware branch. It is a local visual aid only, not an
official warning system. Always follow the official Home Front Command app,
sirens, and instructions first.

## Highlights

- Red Alert live polling is disabled by default. It must be enabled manually
  only after selecting every applicable official alert area and reviewing the
  configured HTTPS relay URL.
- Live HTTPS work is kept separate from display rendering, pauses around minute
  transitions, and suspends safely before beginning a request under low heap.
- The local Web UI reports relay health, request counts, duration, and the
  recovered and lowest request heap.
- The refreshed glass-style Web UI uses a one-second local Live Clock repaint
  while device-status requests remain at three seconds.
- Supported LED, OLED, and button pins can be configured from the Web UI with
  conflict checks and an explicit reboot confirmation.

## Validation

- Web UI and public-release checks passed, and the tagged firmware was built
  with the standard ESP32 production profile.
- A live soak exceeded 48 hours with Red Alert manually enabled: 10,623 of
  10,749 successful relay checks, typical 2.2-second responses, a 158.3 KiB
  observed heap floor, stable Web UI, retained zones, correct mode changes,
  and no LED glitches.

## Known limits

- The default testing relay is community operated; its availability and
  timeliness are not guaranteed by HALO CST.
- A relay error means this optional visual aid is unavailable. It must never be
  treated as an official alert, an early-warning guarantee, or a substitute for
  official instructions.
- The supplied release firmware uses the standard GPIO21/GPIO22 OLED defaults.
  Use the local Hardware Setup controls after first connection when a supported
  alternate wiring map is required.

## Release assets

GitHub builds the exact tagged source and publishes both the raw `.bin` and a
ready-to-flash ZIP containing that firmware image.

See [the Red Alert operating guide](../RED_ALERT.md) for configuration,
runtime safeguards, and credits.
