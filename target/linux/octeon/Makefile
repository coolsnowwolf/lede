# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=mips64
BOARD:=octeon
BOARDNAME:=Cavium Networks Octeon
FEATURES:=squashfs ramdisk pci usb
CPU_TYPE:=octeonplus

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

define Target/Description
	Build firmware images for Cavium Networks Octeon-based boards.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += mkf2fs e2fsprogs

$(eval $(call BuildTarget))
