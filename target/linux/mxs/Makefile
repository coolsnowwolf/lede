# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2014 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mxs
BOARDNAME:=Freescale i.MX23/i.MX28
FEATURES:=ext4 rtc usb gpio
CPU_TYPE:=arm926ej-s

MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>
KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10
KERNELNAME:=zImage dtbs

define Target/Description
	Support for Freescale i.MX23/i.MX28 boards
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
