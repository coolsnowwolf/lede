#!/usr/bin/env python3

import argparse
import binascii
import hashlib
import os
import struct

def create_header(key, version, iv, random, size):
	header = struct.pack('32s32s32s32s32s', key, version, iv, random, size)

	return header

def create_output(args):
	in_st = os.stat(args.input_file)
	in_size = in_st.st_size

	key = "".encode('ascii')
	version = args.version.encode('ascii')
	iv = "".encode('ascii')
	random = "".encode('ascii')
	size = str(in_size).encode('ascii')
	header = create_header(key, version, iv, random, size)

	out_f = open(args.output_file, 'w+b')
	out_f.write(header)
	out_f.close()

	md5 = hashlib.md5()
	md5.update(header[0x60:0x80])
	md5.update(header[0x20:0x40])
	md5_1 = md5.digest()

	md5 = hashlib.md5()
	md5.update(header[0x80:0xA0])
	md5.update(header[0x20:0x40])
	md5_2 = md5.digest()

	key = md5_1 + md5_2

	key_f = open(args.key_file, 'w+b')
	key_f.write(binascii.hexlify(bytearray(key)))
	key_f.close()

	print("AES 256 CBC Key:", binascii.hexlify(bytearray(key)))

def main():
	global args

	parser = argparse.ArgumentParser(description='')

	parser.add_argument('--input-file',
		dest='input_file',
		action='store',
		type=str,
		help='Input file')

	parser.add_argument('--key-file',
		dest='key_file',
		action='store',
		type=str,
		help='AES 256 CBC Key File')

	parser.add_argument('--output-file',
		dest='output_file',
		action='store',
		type=str,
		help='Output file')

	parser.add_argument('--version',
		dest='version',
		action='store',
		type=str,
		help='Version')

	args = parser.parse_args()

	if ((not args.input_file) or
	    (not args.key_file) or
	    (not args.output_file) or
	    (not args.version)):
		parser.print_help()

	create_output(args)

main()
