#!/bin/sh
make ./scripts/config/conf >/dev/null || { make ./scripts/config/conf; exit 1; }
grep \^CONFIG_TARGET_ .config | head -n3 > tmp/.diffconfig.head
grep \^CONFIG_TARGET_DEVICE_ .config >> tmp/.diffconfig.head
grep '^CONFIG_ALL=y' .config >> tmp/.diffconfig.head
grep '^CONFIG_ALL_KMODS=y' .config >> tmp/.diffconfig.head
grep '^CONFIG_ALL_NONSHARED=y' .config >> tmp/.diffconfig.head
grep '^CONFIG_DEVEL=y' .config >> tmp/.diffconfig.head
grep '^CONFIG_TOOLCHAINOPTS=y' .config >> tmp/.diffconfig.head
grep '^CONFIG_BUSYBOX_CUSTOM=y' .config >> tmp/.diffconfig.head
grep '^CONFIG_TARGET_PER_DEVICE_ROOTFS=y' .config >> tmp/.diffconfig.head
./scripts/config/conf --defconfig=tmp/.diffconfig.head -w tmp/.diffconfig.stage1 Config.in >/dev/null
./scripts/kconfig.pl '>+' tmp/.diffconfig.stage1 .config >> tmp/.diffconfig.head
./scripts/config/conf --defconfig=tmp/.diffconfig.head -w tmp/.diffconfig.stage2 Config.in >/dev/null
./scripts/kconfig.pl '>' tmp/.diffconfig.stage2 .config >> tmp/.diffconfig.head
cat tmp/.diffconfig.head
rm -f tmp/.diffconfig tmp/.diffconfig.head
