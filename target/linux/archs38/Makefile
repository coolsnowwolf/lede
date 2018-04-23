#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arc
CPU_TYPE:=archs
BOARD:=archs38
BOARDNAME:=Synopsys DesignWare ARC HS38
MAINTAINER:=Alexey Brodkin <abrodkin@synopsys.com>
SUBTARGETS:=generic

KERNEL_PATCHVER:=4.14

DEVICE_TYPE:=developerboard

define Target/Description
	Synopsys DesignWare boards
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
