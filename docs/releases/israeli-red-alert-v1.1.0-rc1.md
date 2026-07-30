# HALO CST Israeli Red Alert v1.1.0-rc1

This prerelease packages HALO CST's optional Israeli Red Alert companion on
the dedicated Israeli Red Alert branch. It is a local visual aid only, not an
official warning system. Always follow the official Home Front Command app,
sirens, and instructions first.

## Highlights

- Red Alert live polling is disabled by default and must be enabled manually
  after selecting every applicable official alert area and reviewing the
  configured relay URL.
- Saved alert areas and a configurable HTTPS relay persist across compatible
  upgrades.
- Live HTTPS work runs separately from display rendering, defers briefly at
  minute transitions, and suspends before beginning a request under low heap.
- The Web UI reports provider health, request counts, duration, and recovered
  and lowest request heap.

## Validation

- Production and OTA firmware builds passed.
- Web UI source checks: 39/39.
- Public-release audit: 46/46.
- All embedded test targets compiled without upload.
- The prototype completed more than 24 hours with Red Alert manually enabled:
  5,028 of 5,050 successful relay checks (99.56%), a typical two-second
  response, stable Web UI, retained zones, and no LED glitches.

## Known limits

- The default testing relay is community operated; its availability and
  timeliness are not guaranteed by HALO CST.
- A relay error means this optional visual aid is unavailable. It must never
  be treated as an official alert, an early warning guarantee, or a substitute
  for official instructions.

## Release asset

After publication, GitHub attaches the build artifact as
`halo-cst-israeli-red-alert-israeli-red-alert-v1.1.0-rc1.bin`.

See [the Red Alert operating guide](../RED_ALERT.md) for configuration,
runtime safeguards, and credits.
