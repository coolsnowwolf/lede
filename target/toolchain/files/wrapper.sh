#!/bin/sh

# 2009 (C) Copyright Industrie Dial Face S.p.A.
#          Luigi 'Comio' Mantellini <luigi.mantellini@idf-hit.com>
#
# Based on original idea from WindRiver
#
# Toolchain wrapper script.
#
# This script allows us to use a small number of GCC / binutils cross-tools
# (one toolchain per instruction set architecture) to implement a larger
# number of processor- or board-specific tools.  The wrapper script is
# configured at install time with information covering basic CFLAGS,
# LD options and the toolchain triplet name.
#

PROGNAME=$0
REALNAME=$(readlink -f $0)

REALNAME_BASE=$(basename $REALNAME)
REALNAME_DIR=$(dirname $REALNAME)

TARGET_FUNDAMENTAL_ASFLAGS=''
TARGET_FUNDAMENTAL_CFLAGS=''
TARGET_ROOTFS_CFLAGS=''
TARGET_FUNDAMENTAL_LDFLAGS=''
TARGET_TOOLCHAIN_TRIPLET=${REALNAME_BASE%-*}

# Parse our tool name, splitting it at '-' characters.
BINARY=${PROGNAME##*-}

# Parse our tool name, splitting it at '-' characters.
IFS=- read TOOLCHAIN_ARCH TOOLCHAIN_BUILDROOT TOOLCHAIN_OS TOOLCHAIN_PLATFORM PROGNAME << EOF
$REALNAME_BASE
EOF

#
# We add the directory this was executed from to the PATH
# The toolchains (links) should be in this directory or in the users
# PATH.
#
TOOLCHAIN_BIN_DIR="$REALNAME_DIR/"

# Set the PATH so that our run-time location is first
# (get_feature is run from the path, so this has to be set)
export PATH="$TOOLCHAIN_BIN_DIR":$PATH
export GCC_HONOUR_COPTS

TOOLCHAIN_SYSROOT="$TOOLCHAIN_BIN_DIR/../.."
if [ ! -d "$TOOLCHAIN_SYSROOT" ]; then
	echo "Error: Unable to determine sysroot (looking for $TOOLCHAIN_SYSROOT)!" >&2
	exit 1
fi

# -Wl,--dynamic-linker=$TOOLCHAIN_SYSROOT/lib/ld-uClibc.so.0 
# --dynamic-linker=$TOOLCHAIN_SYSROOT/lib/ld-uClibc.so.0 

case $TOOLCHAIN_PLATFORM in
	gnu|glibc|uclibc|musl)
		GCC_SYSROOT_FLAGS="--sysroot=$TOOLCHAIN_SYSROOT -Wl,-rpath=$TOOLCHAIN_SYSROOT/lib:$TOOLCHAIN_SYSROOT/usr/lib"
		LD_SYSROOT_FLAGS="-rpath=$TOOLCHAIN_SYSROOT/lib:$TOOLCHAIN_SYSROOT/usr/lib"
		;;
	*)
		GCC_SYSROOT_FLAGS=""
		LD_SYSROOT_FLAGS=""
		;;
esac

#
# Run the cross-tool.
#
case $BINARY in
	cc|gcc|g++|c++|cpp)
		exec $TARGET_TOOLCHAIN_TRIPLET-$BINARY.bin $GCC_SYSROOT_FLAGS $TARGET_FUNDAMENTAL_CFLAGS $TARGET_ROOTFS_CFLAGS "$@"
		;;
	ld)
		exec $TARGET_TOOLCHAIN_TRIPLET-$BINARY.bin $LD_SYSROOT_FLAGS $TARGET_FUNDAMENTAL_LDFLAGS "$@"
		;;
	as)
		exec $TARGET_TOOLCHAIN_TRIPLET-$BINARY.bin $TARGET_FUNDAMENTAL_ASFLAGS "$@"
		;;
	*)
		exec $TARGET_TOOLCHAIN_TRIPLET-$BINARY.bin "$@"
		;;
esac

exit 0
