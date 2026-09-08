# DJIRemote4Pebble

Experimental proof of concept for controlling a **DJI Osmo Action 4 directly
from a Pebble Time 2 over Bluetooth LE**, without a phone bridge during normal
operation.

> **Build-only PoC / hardware not tested**
>
> The current PoC successfully builds only against an **internally generated
> PebbleOS SDK** exposing BLE Central/GATT APIs that are not yet part of the
> supported public third-party Pebble app API.
>
> The matching experimental PebbleOS firmware has **not been installed or
> hardware-tested** by this project. Do not flash the generated firmware unless
> you fully understand the target hardware and recovery procedure.
>
> The intended final target is:
>
> **Stock PebbleOS + normal third-party .pbw + direct BLE peripheral communication**

## Why this project exists

PebbleOS already contains substantial BLE Central/GATT Client functionality,
but the required app-facing exports are currently internal. This project is a
practical test case for a future public BLE Central/GATT API, as discussed in
Core Devices PebbleOS issue #853.

The proof of concept demonstrates that the application can be built against
those existing internal interfaces. It does **not** demonstrate successful
communication with a physical DJI camera yet.

## PoC 0.4 scope

Current target:

- Pebble Time 2 / Emery
- DJI Osmo Action 4
- direct BLE Central connection
- DJI service FFF0
- FFF4 notifications
- FFF5 writes
- DJI 0019 verification/pairing
- camera status, battery and recording time
- SELECT = REC toggle
- UP = connect/reconnect
- DOWN = reserved for future zoom

## Build architecture

The Cloud CI currently:

1. runs DJI protocol preflight tests,
2. checks out current Core Devices PebbleOS,
3. generates an internal SDK exposing existing BLE Central/GATT symbols,
4. builds matching Pebble Time 2 firmware,
5. builds DJIRemote4Pebble against that exact SDK,
6. produces PBW/PBZ artifacts for development and archival purposes.

A successful build means the code compiles and packages correctly. It does
**not** mean the firmware or camera-control path has been hardware-validated.

See `docs/PHONE_ONLY_CLOUD_CI.md` for details.

## Archived PoC

The successful internal-API build is archived as:

**`poc-0.4-internal-ble`**

This release is for reproducibility and reference. Its firmware artifact is
experimental and is **not intended for installation on stock PebbleOS**.

## License and third-party notice

This project's source code is licensed under the MIT License. See `LICENSE`.

The MIT License applies only to the code in this repository. It does not grant
rights to third-party software, protocols, firmware, trademarks, or other
intellectual property.

DJI, Osmo and related names are trademarks of their respective owners.
Pebble and PebbleOS are associated with their respective owners/projects.
This is an independent community proof of concept and is not affiliated with,
endorsed by, or sponsored by DJI or Core Devices.
