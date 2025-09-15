#!/usr/bin/env python3
"""
# SPDX-License-Identifier: GPL-2.0-or-later
#
# sercomm-kernel-header.py: Creates Sercomm kernel header
#
# Copyright © 2022 Mikhail Zhilkin
"""

import argparse
import binascii
import os
import struct

KERNEL_HEADER_SIZE = 0x100
PADDING = 0xff
ROOTFS_FAKE_HEADER = "UBI#"

def auto_int(x):
	return int(x, 0)

def create_kernel_header(args):
	out_file = open(args.header_file, "wb")
	header = get_kernel_header(args)
	out_file.write(header)
	out_file.close()

def get_kernel_header(args):
	header = bytearray([PADDING] * KERNEL_HEADER_SIZE)

	struct.pack_into('<L', header, 0xc, 0xffffff02)
	struct.pack_into('<L', header, 0x1c, 0x0)
	struct.pack_into('<L', header, 0x34, 0x0)
	struct.pack_into('<L', header, 0x10, args.kernel_offset)
	struct.pack_into('<L', header, 0x28, args.rootfs_offset)

	if (args.rootfs_file):
		if (args.rootfs_checking_size):
			rootfs_size = args.rootfs_checking_size
		else:
			rootfs_size = os.path.getsize(args.rootfs_file)
		buf = open(args.rootfs_file,'rb').read(rootfs_size)
		crc = binascii.crc32(buf) & 0xffffffff
	else:
		rootfs_size = len(ROOTFS_FAKE_HEADER)
		crc = binascii.crc32(str.encode(ROOTFS_FAKE_HEADER)) & \
		      0xffffffff
	struct.pack_into('<L', header, 0x2c, rootfs_size)
	struct.pack_into('<L', header, 0x30, crc)

	rootfs_end_offset = args.rootfs_offset + rootfs_size
	struct.pack_into('<L', header, 0x4, rootfs_end_offset)

	kernel_size = os.path.getsize(args.kernel_file)
	struct.pack_into('<L', header, 0x14, kernel_size)

	buf = open(args.kernel_file,'rb').read()
	crc = binascii.crc32(buf) & 0xffffffff
	struct.pack_into('<L', header, 0x18, crc)

	crc = binascii.crc32(header) & 0xffffffff
	struct.pack_into('<L', header, 0x8, crc)

	struct.pack_into('<L', header, 0x0, 0x726553)

	return header

def main():
	global args

	parser = argparse.ArgumentParser(description='This script generates \
		a kernel header for the Sercomm mt7621 devices')

	parser.add_argument('--kernel-image',
		dest='kernel_file',
		action='store',
		type=str,
		help='Path to a Kernel binary image')

	parser.add_argument('--kernel-offset',
		dest='kernel_offset',
		action='store',
		type=auto_int,
		help='Kernel offset')

	parser.add_argument('--rootfs-offset',
		dest='rootfs_offset',
		action='store',
		type=auto_int,
		help='RootFS offset')

	parser.add_argument('--output-header',
		dest='header_file',
		action='store',
		type=str,
		help='Output kernel header file')

	parser.add_argument('--rootfs-image',
		dest='rootfs_file',
		action='store',
		type=str,
		help='Path to RootFS binary image (optional)')

	parser.add_argument('--rootfs-checking-size',
		dest='rootfs_checking_size',
		action='store',
		type=auto_int,
		help='Bytes count for CRC calculation (optional)')

	args = parser.parse_args()

	if ((not args.kernel_file) or
	    (not args.kernel_offset) or
	    (not args.rootfs_offset) or
	    (not args.header_file)):
		parser.print_help()
		exit()

	create_kernel_header(args)

main()
