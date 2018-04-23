#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mips64
BOARD:=octeon
BOARDNAME:=Cavium Networks Octeon
FEATURES:=squashfs ramdisk pci usb
CPU_TYPE:=octeon
MAINTAINER:=John Crispin <john@phrozen.org>

KERNEL_PATCHVER:=4.14

define Target/Description
	Build firmware images for Cavium Networks Octeon-based boards.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += mkf2fs e2fsprogs

$(eval $(call BuildTarget))
