# Phone-only Cloud CI

This proof of concept can be built from GitHub Actions using only a phone.

## What CI builds
1. DJI protocol preflight tests.
2. Current upstream Core Devices PebbleOS.
3. An internal SDK exposing PebbleOS' already existing BLE Central/GATT symbols.
4. Pebble Time 2 firmware for both firmware slots.
5. A merged test-firmware PBZ.
6. DJIRemote4Pebble against the matching generated SDK.
7. A PBW app artifact.

Firmware and app must come from the same PebbleOS build because native Pebble
apps call firmware functions through the generated Pebble function table.

## First hardware test
After the matching firmware and PBW have been reviewed and installed:

1. Enable wireless/Bluetooth on the DJI Osmo Action 4.
2. Open DJI Remote on the Pebble Time 2.
3. Press UP.
4. Pebble scans for DJI advertising and connects.
5. FFF0 / FFF4 / FFF5 are discovered.
6. The app verifies a single GATT write of at least 51 bytes is possible.
7. DJI 0019 pairing begins and a four-digit code is shown.
8. Confirm the code on the camera.
9. The Pebble should show READY.
10. SELECT toggles recording.

DOWN is reserved for future zoom support.

## Safety
The CI never flashes the watch. A green build only means artifacts were created.
Before installing experimental firmware, verify the exact Time 2 hardware revision
and the available recovery/rollback path.
