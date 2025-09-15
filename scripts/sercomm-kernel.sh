#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2021 Mikhail Zhilkin <csharper2005@gmail.com>
#
###
### sercomm-kernel.sh  - calculates and appends a special kernel header.
###                      Intended for some Sercomm devices (e.g., Beeline
###                      SmartBox GIGA, Beeline SmartBox Turbo+, Sercomm
###                      S3).
#
# Credits to @kar200 for the header description. More details are here:
# https://forum.openwrt.org/t/add-support-for-sercomm-s3-on-stock-uboot
#
if [ $# -ne 3 ]; then
	echo "SYNTAX: $0 <kernel> <kernel_offset> <rootfs_offset>"
	exit 1
fi

FILE_TMP=$1.shdr
KERNEL_IMG=$1
KERNEL_OFFSET=$2
ROOTFS_OFFSET=$3

# Sercomm HDR (0x53657200), 0xffffffff for hdr crc32 calc
hdr_sign_offs=0x0
hdr_sign_val=0x53657200
# Absoulte lenght for Sercomm footer
hdr_footer_size_offs=0x4
hdr_footer_size_val=
# Header checksum. 0xffffffff for hdr crc32 calc
hdr_head_chksum_offs=0x8
hdr_head_chksum_val=
# Magic constant (0x2ffffff)
hdr_int04_offs=0xc
hdr_int04_val=0x2ffffff
# Kernel flash offset
hdr_kern_offs_offs=0x10
hdr_kern_offs_val=$KERNEL_OFFSET
# Kernel lenght
hdr_kern_len_offs=0x14
hdr_kern_len_val=
# Kernel checksum
hdr_kern_chksum_offs=0x18
hdr_kern_chksum_val=
# Magic constant (0x0)
hdr_int08_offs=0x1c
hdr_int08_val=0x0
# Rootfs flash offset
hdr_rootfs_offs_offs=0x28
hdr_rootfs_offs_val=$ROOTFS_OFFSET
# Rootfs flash lenght. We're checking only first 4 bytes
hdr_rootfs_len_offs=0x2c
hdr_rootfs_len_val=0x4
# Rootfs checksum. Checksum is a constant for UBI (first 4 bytes)
hdr_rootfs_chksum_offs=0x30
hdr_rootfs_chksum_val=0x1cfc552d
# Magic constant (0x0)
hdr_int10_offs=0x34
hdr_int10_val=0x0

pad_zeros () {
	awk '{ printf "%8s\n", $0 }' | sed 's/ /0/g'
}

# Remove leading 0x
trim_hx () {
	printf "%x\n" $1 | pad_zeros
}

# Change endian
swap_hx () {
	pad_zeros | awk '{for (i=7;i>=1;i=i-2) printf "%s%s", \
		substr($1,i,2), (i>1?"":"\n")}'
}

# Check file size
fsize () {
	printf "%x\n" `stat -c "%s" $1`
}

# Calculate checksum
chksum () {
	dd if=$1 2>/dev/null | gzip -c | tail -c 8 | od -An -tx4 -N4 \
		--endian=big | tr -d ' \n' | pad_zeros
}

# Write 4 bytes in the header by offset
write_hdr () {
	echo -ne "$(echo $1 | sed 's/../\\x&/g')" | dd of=$FILE_TMP bs=1 \
	seek=$(($2)) count=4 conv=notrunc status=none 2>/dev/null
}

# Pad a new header with 0xff
dd if=/dev/zero ibs=1 count=256 status=none | tr "\000" "\377" > \
	$FILE_TMP 2>/dev/null

# Write constants
write_hdr $(trim_hx $hdr_int04_val) $hdr_int04_offs
write_hdr $(trim_hx $hdr_int08_val) $hdr_int08_offs
write_hdr $(trim_hx $hdr_int10_val) $hdr_int10_offs
# Write footer data
hdr_footer_size_val=$(($hdr_rootfs_offs_val + $hdr_rootfs_len_val))
write_hdr $(trim_hx $hdr_footer_size_val | swap_hx) $hdr_footer_size_offs
# Write kernel data
write_hdr $(trim_hx $hdr_kern_offs_val | swap_hx) $hdr_kern_offs_offs
hdr_kern_len_val=$(fsize $KERNEL_IMG | pad_zeros)
write_hdr $(echo $hdr_kern_len_val | swap_hx) $hdr_kern_len_offs
hdr_kern_chksum_val=$(chksum $KERNEL_IMG)
write_hdr $hdr_kern_chksum_val $hdr_kern_chksum_offs
# Write rootfs data
write_hdr $(trim_hx $hdr_rootfs_offs_val | swap_hx) $hdr_rootfs_offs_offs
write_hdr $(trim_hx $hdr_rootfs_len_val | swap_hx) $hdr_rootfs_len_offs
write_hdr $(trim_hx $hdr_rootfs_chksum_val) $hdr_rootfs_chksum_offs
# Write header checksum
hdr_head_chksum_val=$(chksum $FILE_TMP)
write_hdr $hdr_head_chksum_val $hdr_head_chksum_offs
# Place Sercomm signature
write_hdr $(trim_hx $hdr_sign_val) $hdr_sign_offs

dd if=$KERNEL_IMG >> $FILE_TMP
mv $FILE_TMP $KERNEL_IMG
