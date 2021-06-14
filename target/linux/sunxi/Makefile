# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2016 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=sunxi
BOARDNAME:=Allwinner A1x/A20/A3x/H3/H5/R40
FEATURES:=fpu usb ext4 display rtc squashfs
SUBTARGETS:=cortexa8 cortexa7 cortexa53

KERNEL_PATCHVER:=5.4
KERNELNAME:=zImage dtbs

# A10: Cortex-A8
# A13: Cortex-A8
# A20: dual Cortex-A7
# A31: quad Cortex-A7
# A80: octa Cortex-A15/A7
# H3: quad Cortex-A7
# R40: quad Cortex-A7

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += autocore-arm uboot-envtools
DEFAULT_PACKAGES += partx-utils mkf2fs e2fsprogs

$(eval $(call BuildTarget))
