# HALO CST Israeli Red Alert v1.1.1

This stable patch release improves the local ArduinoOTA password-management
experience for the dedicated Israeli firmware branch. The optional Israeli Red
Alert companion remains a local visual aid only, not an official warning
system. Always follow the official Home Front Command app, sirens, and
instructions first.

## Highlights

- Adds a dedicated Change OTA admin password panel in the local Web UI.
- Password changes require the current password, a new password, and matching
  confirmation before a replacement hash is saved.
- Clearly explains that OTA itself is disabled at each reboot, while the saved
  password hash is remembered for future manual activations.
- Supports normal passwords containing letters, numbers, and symbols.
- Disables the current OTA session after a password change, requiring an
  explicit re-enable with the replacement password before any upgrade.

## Validation

- Web UI source checks: 50/50.
- The tested GPIO16/GPIO17 ESP32 firmware profile compiled successfully.
- A real OTA update using a changed, mixed-character password authenticated,
  uploaded, rebooted, and served the updated password-management interface.
- The prototype completed a further 48-hour stability run with normal clock,
  Web UI, LED, and Red Alert behavior.

## Known limits

- OTA is a trusted-LAN maintenance service and is disabled by default after
  every reboot.
- A password is stored as a hash, never as plaintext. It cannot be recovered;
  use Change OTA admin password when it needs to be replaced.
- The Red Alert companion is optional, disabled by default, and never replaces
  official alerts or instructions.

## Release assets

GitHub builds the exact tagged source and publishes both the raw `.bin` and a
ready-to-flash ZIP containing that firmware image.

See [the Red Alert operating guide](../RED_ALERT.md) for configuration,
runtime safeguards, and credits.
