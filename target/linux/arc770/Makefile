# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arc
BOARD:=arc770
BOARDNAME:=Synopsys DesignWare ARC 770D
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.4

DEVICE_TYPE:=basic

define Target/Description
	Synopsys DesignWare boards
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
