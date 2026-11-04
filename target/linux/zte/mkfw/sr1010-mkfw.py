#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Pack an OpenWrt kernel + rootfs into the ZXSLC SR1010 upgrade format.

The output byte-for-byte matches the vendor upgrade package layout, so the
same file installs from the vendor web UI or from OpenWrt sysupgrade, letting
the two systems swap in place. Only this device's own firmware is produced.

  0x000  outer magic (16 bytes)
  0x010  header_offset, 0 = unsigned package (vendor VerifySign is skipped)
  0x014  512-byte version header
  0x214  body: vendor U-Boot (0x180000)
                | kernel magic (32) + AES-128-ECB(FIT)
                | pad to erase block
                | squashfs rootfs (plain)

The vendor U-Boot decrypts the kernel before bootm and CRC-checks the U-Boot,
kernel and rootfs regions. It does not interpret the rootfs, so a plain
squashfs is stored and read directly by the mainline kernel. CRC-32 here is
the standard polynomial, which matches the vendor csp_crc32.
"""

import argparse
from pathlib import Path
import struct
import subprocess
import sys
import zlib

OUTER_MAGIC = bytes.fromhex("999999994444444455555555aaaaaaaa")
INNER_MAGIC = bytes.fromhex("333333336666666699999999cccccccc")
KERNEL_MAGIC = bytes.fromhex("33333333cccccccc88888888dddddddd") + b"\xff" * 16
KERNEL_KEY = "38c09ffb98".join(("SR1010", ""))  # SR101038c09ffb98

VER_HDR_SIZE = 0x200
UBOOT_SIZE = 0x180000
BLOCK = 0x20000

# Each firmware slot is 41 MiB. The version header takes one more block after the
# body, and a few blocks may be bad, so keep room for both.
SLOT_SIZE = 0x2900000
SLOT_SPARE_BLOCKS = 8

BODY_OFS = 0x214          # body start in an unsigned package

H_VERSION = 0x10          # 32-byte version string
H_FW_SIZE = 0x30          # total body size
H_KERNEL_SIZE = 0x34
H_KERNEL_FILEOFS = 0x38   # absolute file offset of the kernel ciphertext
H_KERNEL_CRC = 0x3C
H_FS_SIZE = 0x40
H_FS_FILEOFS = 0x44       # absolute file offset of the rootfs
H_FS_CRC = 0x48
H_PRODUCT = 0x6C          # 52-byte product string
H_BOOT_INCLUDED = 0x8C    # non-zero asks the flasher to update U-Boot too
H_UBOOT_CRC = 0x98
H_CRC = 0xA4              # CRC over header[0:0xA4]
H_BUILD = 0xA8            # 16-byte build time
H_INNER_MAGIC = 0xF4
H_CRC2 = 0x1FC            # CRC over header[0:0x1FC], computed after H_CRC


def crc32(data):
    return zlib.crc32(data) & 0xFFFFFFFF


def put_u32(buf, ofs, value):
    struct.pack_into("<I", buf, ofs, value)


def put_str(buf, ofs, size, text):
    raw = text.encode("ascii")
    if len(raw) >= size:
        sys.exit("value too long for %#x-byte field: %r" % (size, text))
    buf[ofs:ofs + size] = raw + b"\0" * (size - len(raw))


def aes_ecb(plaintext, openssl):
    """AES-128-ECB with 0xff tail padding to a 16-byte multiple."""
    pad = (-len(plaintext)) % 16
    padded = plaintext + b"\xff" * pad
    out = subprocess.run(
        [openssl, "enc", "-aes-128-ecb", "-nopad", "-nosalt",
         "-K", KERNEL_KEY.encode().hex()],
        input=padded, stdout=subprocess.PIPE, check=True).stdout
    if len(out) != len(padded):
        sys.exit("openssl produced an unexpected ciphertext length")
    return out


# Patches for the vendor U-Boot of SR1010 FV1002B7.8000 (build Aug 17 2026).
# Every entry names the bytes it expects, so a different U-Boot is refused.
#
# pdt_start_kernel copies the whole kernel region into a malloc'ed buffer before
# decrypting it. U-Boot's heap is only 8 MiB, so a kernel FIT larger than about
# 8.4 MiB fails with "malloc tmp_kernel failed". The buffer is only scratch space
# and the vendor code never frees it, so use a fixed address in unused RAM
# instead: movz x0, #0x9000, lsl #16 in place of "bl malloc". RAM spans
# 0x80000000-0xa0000000 and U-Boot itself sits at the top.
UBOOT_PATCHES = (
    (0x32B58, "9476ff97", "0000b2d2", "tmp_kernel scratch buffer at 0x90000000"),
)


def patch_uboot(uboot):
    uboot = bytearray(uboot)
    for ofs, old, new, why in UBOOT_PATCHES:
        old, new = bytes.fromhex(old), bytes.fromhex(new)
        if uboot[ofs:ofs + len(old)] != old:
            sys.exit("U-Boot differs from the expected build at %#x (found %s, "
                     "expected %s): refusing to patch"
                     % (ofs, uboot[ofs:ofs + len(old)].hex(), old.hex()))
        uboot[ofs:ofs + len(new)] = new
        print("patched U-Boot %#x: %s -> %s (%s)" % (ofs, old.hex(), new.hex(), why),
              file=sys.stderr)
    return bytes(uboot)


def load_components(header_path, uboot_path):
    hdr = bytearray(Path(header_path).read_bytes())
    uboot = Path(uboot_path).read_bytes()
    if len(hdr) != VER_HDR_SIZE or hdr[H_INNER_MAGIC:H_INNER_MAGIC + 16] != INNER_MAGIC:
        sys.exit("invalid SR1010 header component")
    if (crc32(hdr[:H_CRC]) != struct.unpack_from("<I", hdr, H_CRC)[0] or
            crc32(hdr[:H_CRC2]) != struct.unpack_from("<I", hdr, H_CRC2)[0]):
        sys.exit("SR1010 header component CRC mismatch")
    if len(uboot) != UBOOT_SIZE or crc32(uboot) != struct.unpack_from("<I", hdr, H_UBOOT_CRC)[0]:
        sys.exit("SR1010 U-Boot component size/CRC mismatch")
    return hdr, uboot


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    components = Path(__file__).resolve().parents[4] / "package/firmware/zte/flies"
    ap.add_argument("--header", default=str(components / "sr1010-header.bin"))
    ap.add_argument("--uboot", default=str(components / "sr1010-uboot.bin"))
    ap.add_argument("kernel", help="OpenWrt FIT image (.itb)")
    ap.add_argument("rootfs", help="squashfs root filesystem")
    ap.add_argument("output", help="output package path")
    ap.add_argument("-v", "--version", help="version string (default: template)")
    ap.add_argument("-p", "--product", help="product string (default: template)")
    ap.add_argument("-b", "--build", help="build time string (default: template)")
    ap.add_argument("--openssl", default="openssl", help="openssl binary")
    args = ap.parse_args()

    hdr, uboot = load_components(args.header, args.uboot)
    uboot = patch_uboot(uboot)
    fit = open(args.kernel, "rb").read()
    rootfs = open(args.rootfs, "rb").read()

    enc_kernel = aes_ecb(fit, args.openssl)
    kernel_region = KERNEL_MAGIC + enc_kernel

    # Body: U-Boot | kernel region | pad to block | rootfs
    body = bytearray(uboot)
    body += kernel_region
    pad = (-len(body)) % BLOCK
    body += b"\xff" * pad
    fs_body_ofs = len(body)
    body += rootfs

    kernel_file_ofs = BODY_OFS + UBOOT_SIZE + len(KERNEL_MAGIC)
    fs_file_ofs = BODY_OFS + fs_body_ofs

    if args.version:
        put_str(hdr, H_VERSION, 32, args.version)
    if args.product:
        put_str(hdr, H_PRODUCT, 52, args.product)
    if args.build:
        put_str(hdr, H_BUILD, 16, args.build)

    if len(body) > SLOT_SIZE - (1 + SLOT_SPARE_BLOCKS) * BLOCK:
        sys.exit("the image does not fit a firmware slot: body %#x bytes, at most %#x "
                 "(rootfs is %#x bytes: drop some packages)"
                 % (len(body), SLOT_SIZE - (1 + SLOT_SPARE_BLOCKS) * BLOCK, len(rootfs)))


    put_u32(hdr, H_FW_SIZE, len(body))
    put_u32(hdr, H_KERNEL_SIZE, len(enc_kernel))
    put_u32(hdr, H_KERNEL_FILEOFS, kernel_file_ofs)
    put_u32(hdr, H_KERNEL_CRC, crc32(enc_kernel))
    put_u32(hdr, H_FS_SIZE, len(rootfs))
    put_u32(hdr, H_FS_FILEOFS, fs_file_ofs)
    put_u32(hdr, H_FS_CRC, crc32(rootfs))
    put_u32(hdr, H_UBOOT_CRC, crc32(uboot))
    put_u32(hdr, H_BOOT_INCLUDED, 0)      # never update the dedicated bootloader partition

    put_u32(hdr, H_CRC, crc32(bytes(hdr[:H_CRC])))
    put_u32(hdr, H_CRC2, crc32(bytes(hdr[:H_CRC2])))

    out = bytearray(OUTER_MAGIC)
    out += struct.pack("<I", 0)           # header_offset = 0, unsigned
    out += hdr
    out += body

    with open(args.output, "wb") as f:
        f.write(out)

    # Self-check: re-verify every CRC the bootloader validates.
    assert crc32(bytes(hdr[:H_CRC])) == struct.unpack_from("<I", hdr, H_CRC)[0]
    assert crc32(bytes(hdr[:H_CRC2])) == struct.unpack_from("<I", hdr, H_CRC2)[0]
    assert crc32(out[fs_file_ofs:fs_file_ofs + len(rootfs)]) == crc32(rootfs)
    assert crc32(out[kernel_file_ofs:kernel_file_ofs + len(enc_kernel)]) == crc32(enc_kernel)

    print("wrote %s: %d bytes (body %#x, kernel %#x, rootfs %#x @ %#x)"
          % (args.output, len(out), len(body), len(enc_kernel),
             len(rootfs), fs_file_ofs), file=sys.stderr)


if __name__ == "__main__":
    main()
