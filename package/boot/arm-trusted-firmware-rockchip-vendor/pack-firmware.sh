#!/bin/bash
# Copyright (C) 2021 ImmortalWrt.org

ACTION="$1"
VARIANT="$2"
PKG_BUILD_DIR="$3"
STAGING_DIR_IMAGE="$4"

case "$VARIANT" in
"rk3328")
	ATF="rk33/rk322xh_bl31_v1.46.elf"
	DDR="rk33/rk3328_ddr_333MHz_v1.17.bin"
	LOADER="rk33/rk322xh_miniloader_v2.50.bin"
	;;
"rk3399")
	ATF="rk33/rk3399_bl31_v1.35.elf"
	DDR="rk33/rk3399_ddr_800MHz_v1.25.bin"
	LOADER="rk33/rk3399_miniloader_v1.26.bin"
	;;
"rk3566")
	ATF="rk35/rk3568_bl31_v1.24.elf"
	DDR="rk35/rk3566_ddr_1056MHz_v1.08.bin"
	LOADER="rk35/rk356x_spl_v1.11.bin"
	;;
"rk3568")
	ATF="rk35/rk3568_bl31_v1.24.elf"
	DDR="rk35/rk3568_ddr_1560MHz_v1.08.bin"
	LOADER="rk35/rk356x_spl_v1.11.bin"
	;;
*)	
	echo -e "Not compatible with your platform: $VARIANT."
	exit 1
	;;
esac

set -x
if [ "$ACTION" == "build" ]; then
	mkimage -n "$VARIANT" -T "rksd" -d "$PKG_BUILD_DIR/bin/$DDR" "$PKG_BUILD_DIR/$VARIANT-idbloader.bin"
	cat "$PKG_BUILD_DIR/bin/$LOADER" >> "$PKG_BUILD_DIR/$VARIANT-idbloader.bin"
	"$PKG_BUILD_DIR/tools/trust_merger" --replace "bl31.elf" "$PKG_BUILD_DIR/bin/$ATF" "$PKG_BUILD_DIR/trust.ini"
elif [ "$ACTION" == "install" ]; then
	mkdir -p "$STAGING_DIR_IMAGE"
	cp -fp "$PKG_BUILD_DIR/bin/$ATF" "$STAGING_DIR_IMAGE"/
	cp -fp "$PKG_BUILD_DIR/tools/loaderimage" "$STAGING_DIR_IMAGE"/
	cp -fp "$PKG_BUILD_DIR/$VARIANT-idbloader.bin" "$STAGING_DIR_IMAGE"/
	cp -fp "$PKG_BUILD_DIR/$VARIANT-trust.bin" "$STAGING_DIR_IMAGE"/
else
	echo -e "Unknown operation: $ACTION."
	exit 1
fi
set +x
