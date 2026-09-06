#!/usr/bin/env python3
from pathlib import Path
import sys

repo = Path(sys.argv[1])
p = repo / "cmake/modules/native_sdk.cmake"
s = p.read_text()

needle = "${PBL_PLATFORM_NAME}\n"
replacement = "${PBL_PLATFORM_NAME} --internal-sdk-build\n"

if "--internal-sdk-build" not in s:
    if needle not in s:
        raise SystemExit("native_sdk.cmake layout changed; refusing blind patch")
    s = s.replace(needle, replacement, 1)
    p.write_text(s)

print("PebbleOS native SDK generation configured for internal symbols.")
