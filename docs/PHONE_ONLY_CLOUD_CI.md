# Phone-only Cloud CI

This proof of concept can be built from GitHub Actions using only a phone or
browser.

> ## Important safety/status notice
>
> **The hardware test described below has not been performed by this project.**
>
> The current application build depends on an internally generated PebbleOS SDK
> and a matching modified PebbleOS firmware because generic BLE Central/GATT
> APIs are not yet exposed as supported public third-party app APIs.
>
> The generated firmware is an **experimental development artifact**. Do not
> install it unless you fully understand the exact Pebble Time 2 hardware target
> and have a verified recovery/rollback procedure.
>
> The project's intended deployment target is stock PebbleOS once a public BLE
> Central/GATT Client API becomes available.

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

## Proposed first hardware test — not yet validated

The following is a **future test plan**, not a verified installation guide.

Only after the matching firmware/PBW, hardware revision and recovery path have
been independently reviewed:

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

## Build interpretation

A green GitHub Actions run means only that the protocol tests, matching firmware
and application artifacts were successfully built and packaged.

It does **not** prove:

- that the experimental firmware is safe to install,
- that the physical watch boots with it,
- that the DJI camera accepts the pairing/session exchange,
- or that recording control works on hardware.

The archived PoC should therefore be treated as a reproducible development
reference until a stock-firmware public BLE Central/GATT API is available or a
separate hardware-validation effort is deliberately undertaken.
