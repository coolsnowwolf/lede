#!/usr/bin/python3
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2022 OpenWrt.org
#
# ./cameo-tag.py <uImageFileName> <OffsetOfRootFS>
#
# CAMEO tag generator used for the D-Link DGS-1210 switches. Their U-Boot
# loader checks for the string CAMEOTAG and a checksum in the kernel and
# rootfs partitions. If not found it complains about the boot image.
# Nevertheless it will boot if the tags are available in the secondary
# boot partitions. If some day we want to overwrite the original vendor
# partition we must have the tags in place. To solve this we insert the
# tag two times into the kernel image.
#
# To understand what we do here it is helpful to explain how the original
# CAMEO tag generation/checking works. The firmware consists of two parts.
# A kernel uImage (<1.5MB) and a rootfs image (<12MB) that are written to
# their respective mtd partitions. The default generator simply checksums
# both parts and appends 16 bytes [<CAMEOTAG><0001><checksum>] to each part.
# The checksum is only an addition of all preceding bytes (b0+b1+b2+...).
# A tag does not interfere with any data in the images itself. During boot
# the loader will scan all primary/secondary partitions (2*kernel, 2*rootfs)
# until it finds the CAMEO tag. If checksums match everything is fine.
# If all 4 fail we are lost. Luckily the loader does not care about where
# the tags are located and ignores any data beyond a tag.
#
# The OpenWrt image consists of a kernel (>1.5MB) and a rootfs. There is
# no chance to add CAMEO tags at the default locations, since the kernel spans
# both the original kernel partition and the start of the rootfs partition.
# This would leave the kernel partition without a tag. So we must find suitable
# space.
#
# Location for original kernel partition is at the end of the uImage header.
# We will reuse the last bytes of the IH_NAME field. This is the tricky part
# because we have the header CRC and the CAMEO checksum that must match the
# whole header. uImage header CRC checksums all data except the CRC itself. The
# for CAMEO checksum in turn, checksums all preceding data except itself.
# Changing one of both results in a change of the other, but data trailing the
# CAMEO checksum only influences the CRC.
#
# Location for original rootfs partition is very simple. It is behind the
# OpenWrt compressed kernel image file that spans into the rootfs. So
# the tag will be written somewhere to the following rootfs partition and
# can be found by U-Boot. The CAMEO checksum calculation must start at the
# offset of the original rootfs partition and includes the "second" half of the
# "split" kernel uImage.

import argparse
import os
import zlib

READ_UNTIL_EOF = -1
UIMAGE_HEADER_SIZE = 64
UIMAGE_CRC_OFF = 4
UIMAGE_CRC_END = 8
UIMAGE_NAME_OFF = 32
UIMAGE_NAME_END = 56
UIMAGE_SUM_OFF = 56
UIMAGE_SUM_END = 60
UIMAGE_INV_OFF = 60
UIMAGE_INV_END = 64
CAMEO_TAG = bytes([0x43, 0x41, 0x4d, 0x45, 0x4f, 0x54, 0x41, 0x47, 0x00, 0x00, 0x00, 0x01])
IMAGE_NAME = bytes([0x4f, 0x70, 0x65, 0x6e, 0x57, 0x72, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00])
CRC_00 = bytes([0x00] * 4)
CRC_FF = bytes([0xff] * 4)

def read_buffer(offset, count):
    args.uimage_file.seek(offset)
    return bytearray(args.uimage_file.read(count))

def write_buffer(whence, buf):
    args.uimage_file.seek(0, whence)
    args.uimage_file.write(buf)

def cameosum(buf):
    return (sum(buf) & 0xffffffff).to_bytes(4, 'big')

def invertcrc(buf):
    return (zlib.crc32(buf) ^ 0xffffffff).to_bytes(4, 'little')

def checksum_header(buf):
    # To efficently get a combination, we will make use of the following fact:
    #     crc32(data + littleendian(crc32(data) ^ 0xffffffff)) = 0xffffffff
    #
    # After manipulation the uImage header looks like this:
    #     [...<ffffffff>...<OpenWrt><000000><CAMEOTAG><0001><checksum><InvCRC>]
    buf[UIMAGE_NAME_OFF:UIMAGE_NAME_END] = IMAGE_NAME + CAMEO_TAG
    buf[UIMAGE_CRC_OFF:UIMAGE_CRC_END] = CRC_FF
    buf[UIMAGE_SUM_OFF:UIMAGE_SUM_END] = cameosum(buf[0:UIMAGE_NAME_END])
    buf[UIMAGE_CRC_OFF:UIMAGE_CRC_END] = CRC_00
    buf[UIMAGE_INV_OFF:UIMAGE_INV_END] = invertcrc(buf[0:UIMAGE_SUM_END])
    buf[UIMAGE_CRC_OFF:UIMAGE_CRC_END] = CRC_FF
    return buf

parser = argparse.ArgumentParser(description='Insert CAMEO firmware tags.')
parser.add_argument('uimage_file', type=argparse.FileType('r+b'))
parser.add_argument('rootfs_start', type=int)
args = parser.parse_args()

args.uimage_file.seek(0, os.SEEK_END)
if args.uimage_file.tell() <= args.rootfs_start:
    raise ValueError(f"uImage must be larger than {args.rootfs_start} bytes")

# tag for the uImage Header of 64 bytes inside the kernel
# partition. Read and mangle it so it contains a valid CAMEO tag
# and checksum that matches perfectly to the uImage header CRC.

buf = checksum_header(read_buffer(0, UIMAGE_HEADER_SIZE))
write_buffer(os.SEEK_SET, buf)

# tag for the second part of the kernel that resides in the
# vendor rootfs partition. For this we will add the CAMEO tag
# and the checksum to the end of the image.

buf = read_buffer(args.rootfs_start, READ_UNTIL_EOF)
write_buffer(os.SEEK_END, CAMEO_TAG + cameosum(buf + CAMEO_TAG))
