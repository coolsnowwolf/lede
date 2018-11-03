#
# Copyright (C) 2013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mxs
BOARDNAME:=Freescale i.MX23/i.MX28
FEATURES:=ext4 rtc usb gpio
CPU_TYPE:=arm926ej-s

MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>
KERNEL_PATCHVER:=4.14
KERNELNAME:=zImage dtbs

define Target/Description
	Support for Freescale i.MX23/i.MX28 boards
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
