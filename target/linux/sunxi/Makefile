# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2016 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=sunxi
BOARDNAME:=Allwinner ARM SoCs
FEATURES:=usb ext4 display rootfs-part rtc squashfs
SUBTARGETS:=cortexa8 cortexa7 cortexa53 arm926ejs

KERNEL_PATCHVER:=6.1
KERNEL_TESTING_PATCHVER:=6.6

KERNELNAME:=zImage dtbs

# A10: Cortex-A8
# A13: Cortex-A8
# A20: dual Cortex-A7
# A31: quad Cortex-A7
# A80: octa Cortex-A15/A7
# H3: quad Cortex-A7
# R40: quad Cortex-A7
# A64/H5/H6/H616: quad Cortex-A53

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += uboot-envtools
DEFAULT_PACKAGES += partx-utils mkf2fs e2fsprogs

$(eval $(call BuildTarget))
