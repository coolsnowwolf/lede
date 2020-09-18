#!/usr/bin/env python3

import argparse
import os
import struct

def create_header(args, size):
	header = struct.pack('32s32s32s32s32s',
		args.part_name.encode('ascii'),
		str(size).encode('ascii'),
		args.part_version.encode('ascii'),
		"".encode('ascii'),
		args.rootfs_version.encode('ascii'))

	return header

def create_output(args):
	in_st = os.stat(args.input_file)
	in_size = in_st.st_size

	header = create_header(args, in_size)
	print(header)

	in_f = open(args.input_file, 'r+b')
	in_bytes = in_f.read(in_size)
	in_f.close()

	out_f = open(args.output_file, 'w+b')
	out_f.write(header)
	out_f.write(in_bytes)
	out_f.close()

def main():
	global args

	parser = argparse.ArgumentParser(description='')

	parser.add_argument('--input-file',
		dest='input_file',
		action='store',
		type=str,
		help='Input file')

	parser.add_argument('--output-file',
		dest='output_file',
		action='store',
		type=str,
		help='Output file')

	parser.add_argument('--part-name',
		dest='part_name',
		action='store',
		type=str,
		help='Partition Name')

	parser.add_argument('--part-version',
		dest='part_version',
		action='store',
		type=str,
		help='Partition Version')

	parser.add_argument('--rootfs-version',
		dest='rootfs_version',
		action='store',
		type=str,
		help='RootFS lib version')

	args = parser.parse_args()

	if not args.rootfs_version:
		args.rootfs_version = ""

	if ((not args.input_file) or
	    (not args.output_file) or
	    (not args.part_name) or
	    (not args.part_version)):
		parser.print_help()

	create_output(args)

main()
