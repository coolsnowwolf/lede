#!/bin/sh
eval `grep CONFIG_GCC_VERSION .config`
CONFIG_TOOLCHAIN_BUILD_VER="$CONFIG_GCC_VERSION-$(cat toolchain/build_version)"
touch .toolchain_build_ver
[ "$CONFIG_TOOLCHAIN_BUILD_VER" = "$(cat .toolchain_build_ver)" ] && exit 0
echo "Toolchain build version changed, running make targetclean"
make targetclean
echo "$CONFIG_TOOLCHAIN_BUILD_VER" > .toolchain_build_ver
exit 0
