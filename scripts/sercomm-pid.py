#!/usr/bin/env python3
"""
# SPDX-License-Identifier: GPL-2.0-or-later
#
# sercomm-pid.py: Creates Sercomm device PID
#
# Copyright © 2022 Mikhail Zhilkin
"""

import argparse
import binascii
import struct

PID_SIZE	= 0x70
PADDING		= 0x30
PADDING_TAIL	= 0x0

def auto_int(x):
	return int(x, 0)

def create_pid_file(args):
	pid_file = open(args.pid_file, "wb")
	buf = get_pid(args)
	pid_file.write(buf)
	pid_file.close()

def get_pid(args):
	buf = bytearray([PADDING] * PID_SIZE)

	enc = args.hw_version.rjust(8, '0').encode('ascii')
	struct.pack_into('>8s', buf, 0x0, enc)

	enc = binascii.hexlify(args.hw_id.encode())
	struct.pack_into('>6s', buf, 0x8, enc)

	enc = args.sw_version.rjust(4, '0').encode('ascii')
	struct.pack_into('>4s', buf, 0x64, enc)

	if (args.extra_padd_size):
		tail = bytearray([PADDING_TAIL] * args.extra_padd_size)
		if (args.extra_padd_byte):
			struct.pack_into ('<i', tail, 0x0,
					  args.extra_padd_byte)
		buf += tail

	return buf

def main():
	global args

	parser = argparse.ArgumentParser(description='This script \
		generates firmware PID for the Sercomm-based devices')

	parser.add_argument('--hw-version',
		dest='hw_version',
		action='store',
		type=str,
		help='Sercomm hardware version')

	parser.add_argument('--hw-id',
		dest='hw_id',
		action='store',
		type=str,
		help='Sercomm hardware ID')

	parser.add_argument('--sw-version',
		dest='sw_version',
		action='store',
		type=str,
		help='Sercomm software version')

	parser.add_argument('--pid-file',
		dest='pid_file',
		action='store',
		type=str,
		help='Output PID file')

	parser.add_argument('--extra-padding-size',
		dest='extra_padd_size',
		action='store',
		type=auto_int,
		help='Size of extra NULL padding at the end of the file \
			(optional)')

	parser.add_argument('--extra-padding-first-byte',
		dest='extra_padd_byte',
		action='store',
		type=auto_int,
		help='First byte of extra padding (optional)')

	args = parser.parse_args()

	if ((not args.hw_version) or
	    (not args.hw_id) or
	    (not args.sw_version) or 
	    (not args.pid_file)):
		parser.print_help()
		exit()

	create_pid_file(args)

main()
