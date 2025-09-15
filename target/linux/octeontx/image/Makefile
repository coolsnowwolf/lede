# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2018 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Image/Build/Initramfs
	$(CP) $(KERNEL_BUILD_DIR)/vmlinux $(BIN_DIR)/$(IMG_PREFIX)-vmlinux
	$(CP) $(KERNEL_BUILD_DIR)/vmlinux-initramfs $(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs
endef

define Image/Build
	$(call Image/Build/$(1))
	cp $(KDIR)/root.$(1) $(BIN_DIR)/$(IMG_PREFIX)-$(1).img
	cp $(KDIR)/vmlinux $(BIN_DIR)/$(IMG_PREFIX)-vmlinux
endef

$(eval $(call BuildImage))
