#!/usr/bin/env python3

from os import getenv
from pathlib import Path
from sys import argv
import hashlib
import json

if len(argv) != 2:
    print("ERROR: JSON info script requires output arg")
    exit(1)

json_path = Path(argv[1])
file_path = Path(getenv("FILE_DIR")) / getenv("FILE_NAME")


if not file_path.is_file():
    print("Skip JSON creation for non existing file", file_path)
    exit(0)


def get_titles():
    titles = []
    for prefix in ["", "ALT0_", "ALT1_", "ALT2_", "ALT3_", "ALT4_"]:
        title = {}
        for var in ["vendor", "model", "variant"]:
            if getenv("DEVICE_{}{}".format(prefix, var.upper())):
                title[var] = getenv("DEVICE_{}{}".format(prefix, var.upper()))

        if title:
            titles.append(title)

    if not titles:
        titles.append({"title": getenv("DEVICE_TITLE")})

    return titles


device_id = getenv("DEVICE_ID")
hash_file = hashlib.sha256(file_path.read_bytes()).hexdigest()

if file_path.with_suffix(file_path.suffix + ".sha256sum").exists():
    hash_unsigned = (
        file_path.with_suffix(file_path.suffix + ".sha256sum").read_text().strip()
    )
else:
    hash_unsigned = hash_file

file_info = {
    "metadata_version": 1,
    "target": "{}/{}".format(getenv("TARGET"), getenv("SUBTARGET")),
    "version_code": getenv("VERSION_CODE"),
    "version_number": getenv("VERSION_NUMBER"),
    "source_date_epoch": int(getenv("SOURCE_DATE_EPOCH")),
    "profiles": {
        device_id: {
            "image_prefix": getenv("DEVICE_IMG_PREFIX"),
            "images": [
                {
                    "type": getenv("FILE_TYPE"),
                    "name": getenv("FILE_NAME"),
                    "sha256": hash_file,
                    "sha256_unsigned": hash_unsigned,
                }
            ],
            "device_packages": getenv("DEVICE_PACKAGES").split(),
            "supported_devices": getenv("SUPPORTED_DEVICES").split(),
            "titles": get_titles(),
        }
    },
}

if getenv("FILE_FILESYSTEM"):
    file_info["profiles"][device_id]["images"][0]["filesystem"] = getenv(
        "FILE_FILESYSTEM"
    )

json_path.write_text(json.dumps(file_info, separators=(",", ":")))
