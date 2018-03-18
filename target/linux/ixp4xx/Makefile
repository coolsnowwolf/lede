#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=armeb
BOARD:=ixp4xx
BOARDNAME:=Intel IXP4xx
FEATURES:=squashfs
MAINTAINER:=Ted Hess <thess@kitschensync.net>, \
	    Imre Kaloz <kaloz@openwrt.org>
SUBTARGETS:=generic harddisk

KERNEL_PATCHVER:=4.9

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += ixp4xx-microcode fconfig
KERNELNAME:=zImage

$(eval $(call BuildTarget))
