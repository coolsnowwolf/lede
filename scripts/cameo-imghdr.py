#!/usr/bin/python3
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2022 Luiz Angelo Daros de Luca <luizluca@gmail.com>
#
# Cameo Image header geneator, used by some D-Link DGS-1210 switches
# and APRESIA ApresiaLightGS series
#
import argparse
import pathlib
import socket
import struct

MODEL_LEN = 20
SIGNATURE_LEN = 16
LINUXLOAD_LEN = 10
BUFSIZE = 4096

parser = argparse.ArgumentParser(description='Generate Cameo firmware header.')
parser.add_argument('source_file', type=argparse.FileType('rb'))
parser.add_argument('dest_file', type=argparse.FileType('wb'))
parser.add_argument('model')
parser.add_argument('signature')
parser.add_argument('partition', type=int, choices=range(0,10),
                    metavar="partition=[0-9]",help="partition id")
parser.add_argument('customer_signature', type=int, choices=range(0,10),
                    metavar="customer_signature=[0-9]",
                    help="customer signature")
parser.add_argument('board_version', type=int, choices=range(0,2**32),
                    metavar="board_version=[0-4294967295]",
                    help="board version")
parser.add_argument('linux_loadaddr', nargs='?',
                    help="Kernel start address in 0xFFFFFFFF format")
args = parser.parse_args()

if len(args.model) > MODEL_LEN:
    raise ValueError(f"Model '{args.model}' is greater than {MODEL_LEN} bytes")

if len(args.signature) > SIGNATURE_LEN:
    raise ValueError(f"Signature '{args.signature}' is greater than"
                     f"{SIGNATURE_LEN} bytes")

if args.signature == "os":
    if args.linux_loadaddr:
        if len(args.linux_loadaddr) > LINUXLOAD_LEN:
            raise ValueError(f"linux_loadaddr '{args.linux_loadaddr}' is greater"
                             f"than {LINUXLOAD_LEN} bytes")
        if (args.linux_loadaddr[0:2] != "0x"):
            raise ValueError(f"linux_loadaddr '{args.linux_loadaddr}' must use"
                             f"the 0x789ABCDE format")
        int(args.linux_loadaddr[2:],16)
    else:
        raise ValueError(f"linux_loadaddr is required for signature 'os'")
else:
    args.linux_loadaddr = ""

checksum = 0
size = 0
while True:
    buf = args.source_file.read(BUFSIZE)
    if not buf:
        break
    checksum = sum(iter(buf),checksum) % (1<<32)
    size += len(buf)

args.dest_file.write(struct.pack('!I', checksum))
args.dest_file.write(struct.pack(f'{MODEL_LEN}s',
                                 args.model.encode("ascii")))
args.dest_file.write(struct.pack(f'{SIGNATURE_LEN}s',
                                 args.signature.encode("ascii")))
args.dest_file.write(struct.pack('!B', args.partition))
args.dest_file.write(struct.pack('!B', 0x40)) # ??? This header size?
args.dest_file.write(struct.pack('!B', 0x00)) # ??? Encrypted?
args.dest_file.write(struct.pack('!B', args.customer_signature))
args.dest_file.write(struct.pack('!I', args.board_version))
args.dest_file.write(struct.pack('!I', size))
args.dest_file.write(struct.pack(f'{LINUXLOAD_LEN}s',
                                 args.linux_loadaddr.encode("ascii")))
args.dest_file.write(struct.pack('!2x'))

args.source_file.seek(0)
while True:
    buf = args.source_file.read(BUFSIZE)
    if not buf:
        break
    args.dest_file.write(buf)
