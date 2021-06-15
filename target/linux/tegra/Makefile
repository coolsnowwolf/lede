# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017-2019 Tomasz Maciej Nowak <tmn505@gmail.com>

include $(TOPDIR)/rules.mk

ARCH := arm
BOARD := tegra
BOARDNAME := NVIDIA Tegra
FEATURES := audio boot-part display ext4 fpu gpio pci pcie rootfs-part rtc squashfs usb
CPU_TYPE := cortex-a9
CPU_SUBTYPE := vfpv3-d16

KERNEL_PATCHVER := 5.4
KERNEL_TESTING_PATCHVER := 5.10

include $(INCLUDE_DIR)/target.mk

KERNELNAME := zImage dtbs

DEFAULT_PACKAGES += e2fsprogs mkf2fs partx-utils

define Target/Description
	Build firmware image for NVIDIA Tegra SoC devices.
endef

$(eval $(call BuildTarget))
