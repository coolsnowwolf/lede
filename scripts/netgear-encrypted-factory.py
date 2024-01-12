#!/usr/bin/env python3

import argparse
import re
import struct
import subprocess
import zlib


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input-file', type=str, required=True)
    parser.add_argument('--output-file', type=str, required=True)
    parser.add_argument('--model', type=str, required=True)
    parser.add_argument('--region', type=str, required=True)
    parser.add_argument('--version', type=str, required=True)
    parser.add_argument('--encryption-block-size', type=str, required=True)
    parser.add_argument('--openssl-bin', type=str, required=True)
    parser.add_argument('--key', type=str, required=True)
    parser.add_argument('--iv', type=str, required=True)
    args = parser.parse_args()

    assert re.match(r'V[0-9]\.[0-9]\.[0-9]\.[0-9]',
                    args.version), 'Version must start with Vx.x.x.x'
    encryption_block_size = int(args.encryption_block_size, 0)
    assert (encryption_block_size > 0 and encryption_block_size % 16 ==
            0), 'Encryption block size must be a multiple of the AES block size (16)'

    image = open(args.input_file, 'rb').read()
    image_enc = []
    for i in range(0, len(image), encryption_block_size):
        chunk = image[i:i + encryption_block_size]
        chunk += b'\x00' * ((-len(chunk)) % 16)  # pad to AES block size (16)
        res = subprocess.run([
            args.openssl_bin,
            'enc',
            '-aes-256-cbc',
            '-nosalt',
            '-nopad',
            '-K', args.key,
            '-iv', args.iv
        ],
            check=True, input=chunk, stdout=subprocess.PIPE)
        image_enc.append(res.stdout)
    image_enc = b''.join(image_enc)

    image_with_header = struct.pack(
        '>32s32s64s64s64s256s12sII',
        args.model.encode('ascii'),
        args.region.encode('ascii'),
        args.version.encode('ascii'),
        b'Thu Jan 1 00:00:00 1970',  # static date for reproducibility
        b'',  # reserved
        b'',  # RSA signature - omitted for now
        b'encrpted_img',
        len(image_enc),
        encryption_block_size,
    ) + image_enc

    checksum = zlib.crc32(image_with_header, 0xffffffff) ^ 0xffffffff

    with open(args.output_file, 'wb') as outfile:
        outfile.write(image_with_header)
        outfile.write(struct.pack('>I', checksum))


if __name__ == "__main__":
    main()
