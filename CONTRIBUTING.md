# Contributing to HALO CST

Thank you for helping improve HALO CST — Connected Smart Timepiece.

## Before starting

1. Read the README, architecture notes, security policy, and Code of Conduct.
2. Search existing issues and pull requests.
3. Open an issue before a large feature or hardware change so scope can be agreed first.
4. Keep each pull request focused; avoid unrelated refactors.

## Development setup

Fork and clone the repository, create a descriptive branch, then build from the repository root:

```sh
pio run
pio run -e esp32dev_ota
```

Run the embedded suites on a test ESP32 and the local API scripts against test firmware. Do not erase or overwrite a production device merely to demonstrate a test.

## Coding expectations

- Preserve the documented GPIO assignments unless a change is explicitly proposed.
- Keep runtime interaction non-blocking; avoid `delay()` in service/update paths.
- Centralize hardware, behavior, and product settings in the existing headers.
- Keep settings migrations backward compatible where practical.
- Do not commit credentials, tokens, personal coordinates, device IPs, generated logs, binaries, or local paths.
- Maintain the trusted-LAN security assumptions explicitly when adding network features.
- Add focused tests for parsing, validation, state transitions, and failure behavior.

## Pull requests

Complete the pull-request template. Include exact build results, memory usage, tests performed, hardware used, and anything not manually validated. Firmware claims must distinguish code review/build results from physical hardware validation.

By contributing, you agree that your contribution is licensed under the repository’s MIT License and that participation follows the Code of Conduct.
