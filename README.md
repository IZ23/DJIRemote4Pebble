# DJIRemote4Pebble

Experimental proof of concept for controlling a DJI Osmo Action 4 directly from
a Pebble Time 2 over Bluetooth LE — without a phone or DJI Mimo during normal use.

## PoC 0.4 Cloud CI

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

The project uses an internal SDK generated from the same PebbleOS source as the
experimental test firmware. This is necessary because generic BLE Central/GATT
functions exist in PebbleOS but are not yet part of the ordinary public watch-app API.

## Cloud build

The GitHub Action builds protocol tests, dual-slot Pebble Time 2 firmware and the
matching DJIRemote PBW. It never installs or flashes anything automatically.

See `docs/PHONE_ONLY_CLOUD_CI.md`.

## Experimental status

Do not install a generated firmware PBZ until its hardware target and recovery path
have been checked. Zoom and a periodic preview image are future research items;
no undocumented DJI command is sent for either feature.
