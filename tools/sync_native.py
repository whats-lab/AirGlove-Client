#!/usr/bin/env python3
import argparse
import filecmp
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
NATIVE = ROOT / "native"
COPIES = {
    "linux-x64/libairglove_client.so": [
        "python/airglove_client/_lib/linux-x64/libairglove_client.so",
        "unity/com.whatslab.airgloveclient/Plugins/Linux/x86_64/libairglove_client.so",
        "unreal/AirGloveClient/Binaries/ThirdParty/AirGloveClient/Linux/libairglove_client.so",
    ],
    "linux-arm64/libairglove_client.so": [
        "python/airglove_client/_lib/linux-arm64/libairglove_client.so",
        "unity/com.whatslab.airgloveclient/Plugins/Linux/ARM64/libairglove_client.so",
        "unreal/AirGloveClient/Binaries/ThirdParty/AirGloveClient/LinuxArm64/libairglove_client.so",
    ],
    "windows-x64/airglove_client.dll": [
        "python/airglove_client/_lib/windows-x64/airglove_client.dll",
        "unity/com.whatslab.airgloveclient/Plugins/Windows/x86_64/airglove_client.dll",
        "unreal/AirGloveClient/Binaries/ThirdParty/AirGloveClient/Win64/airglove_client.dll",
    ],
    "windows-arm64/airglove_client.dll": [
        "python/airglove_client/_lib/windows-arm64/airglove_client.dll",
        "unity/com.whatslab.airgloveclient/Plugins/Windows/ARM64/airglove_client.dll",
    ],
    "macos-universal/libairglove_client.dylib": [
        "python/airglove_client/_lib/macos-universal/libairglove_client.dylib",
        "unity/com.whatslab.airgloveclient/Plugins/macOS/libairglove_client.dylib",
        "unreal/AirGloveClient/Binaries/ThirdParty/AirGloveClient/Mac/libairglove_client.dylib",
    ],
}


def main():
    parser = argparse.ArgumentParser(description="Copy native/ binaries into the Python, Unity and Unreal packages")
    parser.add_argument("--check", action="store_true", help="only verify that every copy matches native/")
    args = parser.parse_args()
    mismatched = []
    for source, targets in COPIES.items():
        src = NATIVE / source
        if not src.is_file():
            sys.exit(f"missing {src}")
        for target in targets:
            dst = ROOT / target
            if dst.is_file() and filecmp.cmp(src, dst, shallow=False):
                continue
            if args.check:
                mismatched.append(target)
            else:
                dst.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(src, dst)
                print(f"updated {target}")
    if mismatched:
        sys.exit("out of sync with native/:\n  " + "\n  ".join(mismatched))
    print("all package binaries match native/")


if __name__ == "__main__":
    main()
