# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2007-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

BOARD:=lantiq
BOARDNAME:=Lantiq
FEATURES:=squashfs
# ase and xway_legacy only provide firmware partitions below 4 MiB.
SUBTARGETS:=xrx200 xrx200_legacy xway falcon

KERNEL_PATCHVER:=6.18
KERNEL_TESTING_PATCHVER:=5.4

define Target/Description
	Build firmware images for Lantiq SoC
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
