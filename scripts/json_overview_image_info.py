#!/usr/bin/env python3

from os import getenv, environ
from pathlib import Path
from subprocess import run
from sys import argv
import json

if len(argv) != 2:
    print("JSON info files script requires ouput file as argument")
    exit(1)

output_path = Path(argv[1])

assert getenv("WORK_DIR"), "$WORK_DIR required"

work_dir = Path(getenv("WORK_DIR"))

output = {}

for json_file in work_dir.glob("*.json"):
    image_info = json.loads(json_file.read_text())
    if not output:
        output.update(image_info)
    else:
        # get first (and only) profile in json file
        device_id = next(iter(image_info["profiles"].keys()))
        if device_id not in output["profiles"]:
            output["profiles"].update(image_info["profiles"])
        else:
            output["profiles"][device_id]["images"].append(
                image_info["profiles"][device_id]["images"][0]
            )


output["default_packages"] = run(
    [
        "make",
        "--no-print-directory",
        "-C",
        f"target/linux/{output['target'].split('/')[0]}",
        "val.DEFAULT_PACKAGES",
    ],
    capture_output=True,
    check=True,
    env=environ.copy().update({"TOPDIR": Path().cwd()}),
    text=True,
).stdout.split()

if output:
    output_path.write_text(json.dumps(output, sort_keys=True, separators=(",", ":")))
else:
    print("JSON info file script could not find any JSON files for target")
