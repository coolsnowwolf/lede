#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=xburst
BOARDNAME:=Ingenic XBurst
FEATURES:=targz nand ubifs audio
SUBTARGETS:=qi_lb60

KERNEL_PATCHVER:=3.18

DEVICE_TYPE:=other

define Target/Description
	Build firmware images for XBurst JZ47x0 based boards.
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
