#!/usr/bin/env python3

import json
import os
import hashlib


def e(variable, default=None):
    return os.environ.get(variable, default)


json_path = "{}{}{}.json".format(e("BIN_DIR"), os.sep, e("IMAGE_PREFIX"))

with open(os.path.join(e("BIN_DIR"), e("IMAGE_NAME")), "rb") as image_file:
    image_hash = hashlib.sha256(image_file.read()).hexdigest()


def get_titles():
    titles = []
    for prefix in ["", "ALT0_", "ALT1_", "ALT2_"]:
        title = {}
        for var in ["vendor", "model", "variant"]:
            if e("DEVICE_{}{}".format(prefix, var.upper())):
                title[var] = e("DEVICE_{}{}".format(prefix, var.upper()))

        if title:
            titles.append(title)

    if not titles:
        titles.append({"title": e("DEVICE_TITLE")})

    return titles


if not os.path.exists(json_path):
    device_info = {
        "id": e("DEVICE_ID"),
        "image_prefix": e("IMAGE_PREFIX"),
        "images": [],
        "metadata_version": 1,
        "supported_devices": e("SUPPORTED_DEVICES").split(),
        "target": "{}/{}".format(e("TARGET"), e("SUBTARGET", "generic")),
        "titles": get_titles(),
        "version_commit": e("VERSION_CODE"),
        "version_number": e("VERSION_NUMBER"),
    }
else:
    with open(json_path, "r") as json_file:
        device_info = json.load(json_file)

image_info = {"type": e("IMAGE_TYPE"), "name": e("IMAGE_NAME"), "sha256": image_hash}
device_info["images"].append(image_info)

with open(json_path, "w") as json_file:
    json.dump(device_info, json_file, sort_keys=True, indent="  ")
