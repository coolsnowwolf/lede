#!/usr/bin/env python3

"""
CFE Partition Tag

{
	u32 part_id;
	u32 part_size;
	u16 flags;
	char part_name[33];
	char part_version[21];
	u32 part_crc32;
}

"""

import argparse
import os
import struct
import binascii


PART_NAME_SIZE = 33
PART_VERSION_SIZE = 21


def auto_int(x):
    return int(x, 0)


def str_to_bytes_pad(string, size):
    str_bytes = string.encode()
    num_bytes = len(str_bytes)
    if num_bytes >= size:
        str_bytes = str_bytes[: size - 1] + "\0".encode()
    else:
        str_bytes += "\0".encode() * (size - num_bytes)
    return str_bytes


def create_tag(args, in_bytes, size):
    # JAM CRC32 is bitwise not and unsigned
    crc = ~binascii.crc32(in_bytes) & 0xFFFFFFFF

    tag = bytearray()
    tag += struct.pack(">I", args.part_id)
    tag += struct.pack(">I", size)
    tag += struct.pack(">H", args.part_flags)
    tag += str_to_bytes_pad(args.part_name, PART_NAME_SIZE)
    tag += str_to_bytes_pad(args.part_version, PART_VERSION_SIZE)
    tag += struct.pack(">I", crc)

    return tag


def create_output(args):
    in_st = os.stat(args.input_file)
    in_size = in_st.st_size

    in_f = open(args.input_file, "r+b")
    in_bytes = in_f.read(in_size)
    in_f.close()

    tag = create_tag(args, in_bytes, in_size)

    out_f = open(args.output_file, "w+b")
    out_f.write(tag)
    out_f.close()


def main():
    global args

    parser = argparse.ArgumentParser(description="")

    parser.add_argument(
        "--flags",
        dest="part_flags",
        action="store",
        type=auto_int,
        help="Partition Flags",
    )

    parser.add_argument(
        "--id",
        dest="part_id",
        action="store",
        type=auto_int,
        help="Partition ID",
    )

    parser.add_argument(
        "--input-file",
        dest="input_file",
        action="store",
        type=str,
        help="Input file",
    )

    parser.add_argument(
        "--output-file",
        dest="output_file",
        action="store",
        type=str,
        help="Output file",
    )

    parser.add_argument(
        "--name",
        dest="part_name",
        action="store",
        type=str,
        help="Partition Name",
    )

    parser.add_argument(
        "--version",
        dest="part_version",
        action="store",
        type=str,
        help="Partition Version",
    )

    args = parser.parse_args()

    if (
        (not args.part_flags)
        or (not args.part_id)
        or (not args.input_file)
        or (not args.output_file)
        or (not args.part_name)
        or (not args.part_version)
    ):
        parser.print_help()
    else:
        create_output(args)


main()
