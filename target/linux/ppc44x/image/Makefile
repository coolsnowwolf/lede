#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

JFFS2_BLOCKSIZE=256k

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk


define Image/Prepare
	$(LINUX_DIR)/scripts/dtc/dtc -O dtb -R 4 -S 0x20000 $(DTS_DIR)/canyonlands.dts > $(KDIR)/openwrt-canyonlands.dtb
endef

define Image/BuildKernel
	cp $(KDIR)/uImage $(BIN_DIR)/$(IMG_PREFIX)-uImage
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

define Image/Build/jffs2-256k
	( \
		dd if=$(KDIR)/uImage bs=2048k conv=sync; \
		dd if=$(KDIR)/root.$(1) bs=256k conv=sync; \
	) > $(BIN_DIR)/$(IMG_PREFIX)-jffs2.img
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	( \
		dd if=$(KDIR)/cuImage.taishan bs=256k conv=sync; \
		dd if=$(KDIR)/root.$(1) bs=256k conv=sync; \
	) > $(BIN_DIR)/$(IMG_PREFIX)-taishan-$(1).img
	( \
		dd if=$(KDIR)/uImage bs=1920k conv=sync; \
		dd if=$(KDIR)/openwrt-canyonlands.dtb bs=128k conv=sync; \
		dd if=$(KDIR)/root.$(1) bs=256k conv=sync; \
	) > $(BIN_DIR)/$(IMG_PREFIX)-canyonlands-$(1).img
endef

$(eval $(call BuildImage))
