# Red Alert companion

HALO CST's Israeli Red Alert companion is an optional visual aid for selected
Home Front Command alert areas. It is not an official warning system. Always
follow the official Home Front Command app, sirens, and instructions first.

## Configuration and defaults

- Live polling is disabled on a fresh install.
- Enable it manually from the local Web UI only after selecting every official
  alert area that applies to you and reviewing the relay URL.
- The default testing relay is the community Tzeva Adom endpoint. It is not
  operated or guaranteed by HALO CST.
- Area selections and the relay URL persist across compatible firmware
  upgrades. The Web UI keeps saved areas visible even when they are outside
  the initial catalog list.

## Runtime behavior

- The companion normally polls the configured HTTPS relay every 15 seconds.
- Failed checks use a 30-second retry delay. A source failure means only that
  HALO's visual aid is unavailable; it must never replace official channels.
- Alert HTTPS work runs asynchronously on the ESP32 networking core, separate
  from the clock-rendering loop. Checks are also deferred during the few
  seconds around a minute transition.
- The service records request duration and recovered/lowest request heap in
  the Web UI. It suspends live polling before starting a new request when free
  heap is below its safety threshold.
- The amber **Run visual test** control is local simulation only. It neither
  contacts a provider nor represents a real alert.

## Validated prototype soak

The current hardening checkpoint was validated on the HALO CST prototype with
Red Alert manually enabled, saved zones, the default community relay, CLASSIC
rendering, weather, and the Web UI active:

| Measure | Observed result |
| --- | --- |
| Live soak duration | Nearly 8 hours |
| Successful relay checks | 1,594 of 1,597 (99.8%) |
| Typical check duration | 2.1 seconds |
| Recovered heap after a check | 201.9 KiB |
| Lowest observed request heap | 160.5 KiB |
| Visual/network result | No LED glitches; clock and Web UI remained healthy |

The three missed provider checks were transient community-source retries and
did not affect clock or Web UI operation. This is prototype validation, not a
guarantee of alert delivery, timeliness, provider availability, or suitability
for life-safety use.

## When to disable it

Disable Red Alert immediately if the Web UI reports low-heap suspension,
repeated provider failures, or any unexpected clock/LED behavior. The saved
zones remain available for a later controlled retest.

## Credits and area data

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for the alert-area catalog
license and community research credits.
