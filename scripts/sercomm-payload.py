#!/usr/bin/env python3

import argparse
import hashlib
import os

def create_output(args):
	in_st = os.stat(args.input_file)
	in_size = in_st.st_size

	in_f = open(args.input_file, 'r+b')
	in_bytes = in_f.read(in_size)
	in_f.close()

	sha256 = hashlib.sha256()
	sha256.update(in_bytes)

	out_f = open(args.output_file, 'w+b')
	out_f.write(bytes.fromhex(args.pid))
	out_f.write(sha256.digest())
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

	parser.add_argument('--pid',
		dest='pid',
		action='store',
		type=str,
		help='Sercomm PID')

	args = parser.parse_args()

	if ((not args.input_file) or
	    (not args.output_file) or
	    (not args.pid)):
		parser.print_help()

	create_output(args)

main()
