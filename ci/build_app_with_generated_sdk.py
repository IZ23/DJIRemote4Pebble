#!/usr/bin/env python3
from pathlib import Path
import argparse, os, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument("--sdk-root", required=True)
ap.add_argument("--project-root", required=True)
args = ap.parse_args()

sdk = Path(args.sdk_root).resolve()
project = Path(args.project_root).resolve()

candidates = [sdk / "common" / "waf", sdk / "waf"]
waf = next((p for p in candidates if p.exists()), None)

if waf is None:
    found = list(sdk.rglob("waf"))
    waf = found[0] if found else None

if waf is None:
    raise SystemExit(
        "Generated SDK contains no waf builder. "
        "Firmware build succeeded, but app-packaging path needs adaptation."
    )

env = os.environ.copy()
env["PEBBLE_SDK_ROOT"] = str(sdk)

subprocess.run([sys.executable, str(waf), "configure"], cwd=project, env=env, check=True)
subprocess.run([sys.executable, str(waf), "build"], cwd=project, env=env, check=True)

pbws = list((project / "build").rglob("*.pbw"))
if not pbws:
    raise SystemExit("App build finished without a PBW output.")

print("PBW:", pbws[0])
