#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Update the saved environment in a copy of the Phytium boot firmware."""

import struct
import sys
import zlib
from pathlib import Path

ENV_OFFSET = 0x170000
ENV_SIZE = 0x1000


def update_environment(source, destination):
    firmware = bytearray(Path(source).read_bytes())
    environment = firmware[ENV_OFFSET:ENV_OFFSET + ENV_SIZE]
    if len(environment) != ENV_SIZE:
        raise ValueError("boot firmware does not contain the saved environment")
    checksum, = struct.unpack_from("<I", environment)
    if checksum != zlib.crc32(environment[4:]):
        raise ValueError("saved U-Boot environment CRC does not match")

    entries = bytes(environment[4:]).split(b"\0\0", 1)[0].split(b"\0")
    updated = []
    found = False
    for entry in entries:
        if entry.startswith(b"bootargs="):
            if b",6M(kernel)," not in entry:
                raise ValueError("unexpected kernel partition in bootargs")
            entry = entry.replace(b",6M(kernel),", b",12M(kernel),")
            found = True
        elif entry.startswith(b"load_kernel=cp.b "):
            if not entry.endswith(b" 0x600000"):
                raise ValueError("unexpected kernel copy length")
            entry = entry[:-len(b"0x600000")] + b"0xc00000"
        updated.append(entry)
    if not found:
        raise ValueError("saved environment has no kernel partition bootargs")

    payload = b"\0".join(updated) + b"\0\0"
    if len(payload) > ENV_SIZE - 4:
        raise ValueError("updated environment is too large")
    payload = payload.ljust(ENV_SIZE - 4, b"\xff")
    firmware[ENV_OFFSET:ENV_OFFSET + ENV_SIZE] = (
        struct.pack("<I", zlib.crc32(payload)) + payload
    )
    Path(destination).write_bytes(firmware)


if __name__ == "__main__":
    update_environment(*sys.argv[1:])
