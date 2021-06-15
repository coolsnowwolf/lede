# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2007-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

BOARD:=lantiq
BOARDNAME:=Lantiq
FEATURES:=squashfs
SUBTARGETS:=xrx200 xway xway_legacy falcon ase

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

define Target/Description
	Build firmware images for Lantiq SoC
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
