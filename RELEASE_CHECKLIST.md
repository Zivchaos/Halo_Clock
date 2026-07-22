# HALO CST release checklist

## Scope and hygiene

- [ ] Version, product metadata, README, and changelog agree.
- [ ] Diff contains only intended release work.
- [ ] No credentials, tokens, API keys, private coordinates, device IPs, generated logs, binaries, or machine-specific paths.
- [ ] `git diff --check` succeeds and the working tree is understood.
- [ ] Security and migration notes match actual firmware behavior.

## Build and automated validation

- [ ] `pio run` succeeds with zero project-code warnings.
- [ ] `pio run -e esp32dev_ota` succeeds.
- [ ] Weather parser embedded tests pass.
- [ ] Diagnostics embedded tests pass.
- [ ] Full Web UI/API suite passes.
- [ ] Weather hardware/API regression suite passes.
- [ ] Public-release metadata/hygiene audit passes.
- [ ] RAM, flash, and remaining OTA capacity are recorded.

## Device validation

- [ ] Normal USB upload succeeds.
- [ ] Device boots, synchronizes time, and renders OLED/LED clock correctly.
- [ ] Button short/long presses retain expected behavior.
- [ ] Web UI is reachable at `halo-cst.local` and controls stay synchronized.
- [ ] Weather fetch/cache behavior works.
- [ ] Automatic NIGHT and brightness restoration work.
- [ ] Real ArduinoOTA upload succeeds with OLED/LED progress and normal post-reboot rendering.
- [ ] Existing Wi-Fi credentials and Preferences survive a compatible upgrade.
- [ ] Fresh setup path is validated on an erased test device or explicitly documented as a safe simulation.

## Publication

- [ ] Draft pull request records exact validation and unresolved limitations.
- [ ] Review feedback and CI are complete.
- [ ] Release notes and artifacts are prepared without secrets.
- [ ] Use a `v*` tag for the standard firmware, or an
  `israeli-red-alert-v*` tag for the Israeli Red Alert branch.
- [ ] Publish the GitHub Release from that tag; the release workflow builds
  the tagged source and attaches the correctly named `.bin` automatically.
- [ ] Final release tag is created only after explicit approval.
- [ ] Repository visibility is changed only by an authorized maintainer.
