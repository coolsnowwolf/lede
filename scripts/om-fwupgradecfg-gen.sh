#!/bin/sh
#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

usage() {
	echo "Usage: $0 <OM2P|OM5P|OM5PAC|MR600|MR900|MR1750|A60|A42|A62> <out file path> <kernel path> <rootfs path>"
	rm -f $CFG_OUT
	exit 1
}

[ "$#" -lt 4 ] && usage

CE_TYPE=$1
CFG_OUT=$2
KERNEL_PATH=$3
ROOTFS_PATH=$4

case $CE_TYPE in
	OM2P)
		MAX_PART_SIZE=7168
		KERNEL_FLASH_ADDR=0x1c0000
		FLASH_BS=262144
		MD5_SKIP_BLOCKS=1
		SIZE_FACTOR=1
		SIZE_FORMAT="%d"
		;;
	OM5P|OM5PAC|MR600|MR900|MR1750|A60)
		MAX_PART_SIZE=7808
		KERNEL_FLASH_ADDR=0xb0000
		FLASH_BS=65536
		MD5_SKIP_BLOCKS=4
		SIZE_FACTOR=1
		SIZE_FORMAT="%d"
		;;
	A42)
		MAX_PART_SIZE=15616
		KERNEL_FLASH_ADDR=0x180000
		FLASH_BS=65536
		MD5_SKIP_BLOCKS=4
		SIZE_FACTOR=1024
		SIZE_FORMAT="0x%08x"
		;;
	A62)
		MAX_PART_SIZE=15552
		KERNEL_FLASH_ADDR=0x1a0000
		FLASH_BS=65536
		MD5_SKIP_BLOCKS=4
		SIZE_FACTOR=1024
		SIZE_FORMAT="0x%08x"
		;;
	*)
		echo "Error - unsupported ce type: $CE_TYPE"
		exit 1
		;;
esac

CHECK_BS=65536

KERNEL_SIZE=$(stat -c%s "$KERNEL_PATH")
KERNEL_MD5=$(mkhash md5 $KERNEL_PATH)
KERNEL_SHA256=$(mkhash sha256 $KERNEL_PATH)
KERNEL_PART_SIZE_KB=$(size=$(($KERNEL_SIZE / $FLASH_BS)); [ $(($size * $FLASH_BS)) -lt $KERNEL_SIZE ] && size=$(($size + 1)); echo $(($size * $FLASH_BS / 1024)))
KERNEL_PART_SIZE=$(printf $SIZE_FORMAT $(($KERNEL_PART_SIZE_KB * $SIZE_FACTOR)))

ROOTFS_FLASH_ADDR=$(addr=$(($KERNEL_FLASH_ADDR + ($KERNEL_PART_SIZE_KB * 1024))); printf "0x%x" $addr)
ROOTFS_SIZE=$(stat -c%s "$ROOTFS_PATH")
ROOTFS_CHECK_BLOCKS=$((($ROOTFS_SIZE / $CHECK_BS) - $MD5_SKIP_BLOCKS))
ROOTFS_MD5=$(dd if=$ROOTFS_PATH bs=$CHECK_BS count=$ROOTFS_CHECK_BLOCKS 2>&- | mkhash md5)
ROOTFS_MD5_FULL=$(mkhash md5 $ROOTFS_PATH)
ROOTFS_SHA256_FULL=$(mkhash sha256 $ROOTFS_PATH)
ROOTFS_CHECK_SIZE=$(printf '0x%x' $(($ROOTFS_CHECK_BLOCKS * $CHECK_BS)))
ROOTFS_PART_SIZE_KB=$(($MAX_PART_SIZE - $KERNEL_PART_SIZE_KB))
ROOTFS_PART_SIZE=$(printf $SIZE_FORMAT $(($ROOTFS_PART_SIZE_KB * $SIZE_FACTOR)))

cat << EOF > $CFG_OUT
[vmlinux]
filename=kernel
md5sum=$KERNEL_MD5
filemd5sum=$KERNEL_MD5
filesha256sum=$KERNEL_SHA256
flashaddr=$KERNEL_FLASH_ADDR
checksize=0x0
cmd_success=setenv bootseq 1,2; setenv kernel_size_1 $KERNEL_PART_SIZE; saveenv
cmd_fail=reset

[rootfs]
filename=rootfs
md5sum=$ROOTFS_MD5
filemd5sum=$ROOTFS_MD5_FULL
filesha256sum=$ROOTFS_SHA256_FULL
flashaddr=$ROOTFS_FLASH_ADDR
checksize=$ROOTFS_CHECK_SIZE
cmd_success=setenv bootseq 1,2; setenv kernel_size_1 $KERNEL_PART_SIZE; setenv rootfs_size_1 $ROOTFS_PART_SIZE; saveenv
cmd_fail=reset
EOF
