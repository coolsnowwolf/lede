#
# Copyright (C) 2017-2019 Tomasz Maciej Nowak <tomek_n@o2.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH := arm
BOARD := tegra
BOARDNAME := NVIDIA Tegra
FEATURES := audio boot-part display ext4 fpu gpio pci pcie rootfs-part rtc squashfs usb
CPU_TYPE := cortex-a9
CPU_SUBTYPE := vfpv3
MAINTAINER := Tomasz Maciej Nowak <tomek_n@o2.pl>

KERNEL_PATCHVER := 4.19

include $(INCLUDE_DIR)/target.mk

KERNELNAME := zImage dtbs

DEFAULT_PACKAGES += e2fsprogs mkf2fs partx-utils

define Target/Description
	Build firmware image for NVIDIA Tegra SoC devices.
endef

$(eval $(call BuildTarget))
