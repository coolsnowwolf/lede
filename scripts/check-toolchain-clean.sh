#!/bin/sh
eval "$(grep CONFIG_GCC_VERSION .config)"
CONFIG_TOOLCHAIN_BUILD_VER="$CONFIG_GCC_VERSION-$(cat toolchain/build_version)"
touch .toolchain_build_ver
CURRENT_TOOLCHAIN_BUILD_VER="$(cat .toolchain_build_ver)"
[ -z "$CURRENT_TOOLCHAIN_BUILD_VER" ] && {
	echo "$CONFIG_TOOLCHAIN_BUILD_VER" > .toolchain_build_ver
	exit 0
}
[ "$CONFIG_TOOLCHAIN_BUILD_VER" = "$CURRENT_TOOLCHAIN_BUILD_VER" ] && exit 0
echo "Toolchain build version changed ($CONFIG_TOOLCHAIN_BUILD_VER != $CURRENT_TOOLCHAIN_BUILD_VER), running make targetclean"
make targetclean
echo "$CONFIG_TOOLCHAIN_BUILD_VER" > .toolchain_build_ver
exit 0
