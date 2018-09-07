# 
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

NAND_BLOCKSIZE=2048-128k

define Image/BuildKernel
	cat $(KDIR)/zImage $(DTS_DIR)/omap2420-n810.dtb > $(BIN_DIR)/$(IMG_PREFIX)-zImage
	chmod 0644 $(BIN_DIR)/$(IMG_PREFIX)-zImage
endef

define Image/Build/squashfs
    $(call prepare_generic_squashfs,$(BIN_DIR)/$(IMG_PREFIX)-root.$(1))
endef

define Image/Build
	$(CP) $(KDIR)/root.$(1) $(BIN_DIR)/$(IMG_PREFIX)-root.$(1)
	$(call Image/Build/$(1),$(1))
endef


$(eval $(call BuildImage))
