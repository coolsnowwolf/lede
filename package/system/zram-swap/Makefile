# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2021 OpenWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=zram-swap
PKG_RELEASE:=$(AUTORELEASE)

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/zram-swap
  SECTION:=utils
  CATEGORY:=Base system
  DEPENDS:= \
	+@BUSYBOX_CONFIG_FEATURE_SWAPON_DISCARD \
	+@BUSYBOX_CONFIG_FEATURE_SWAPON_PRI \
	+@BUSYBOX_CONFIG_MKSWAP \
	+@BUSYBOX_CONFIG_SWAPOFF \
	+@BUSYBOX_CONFIG_SWAPON \
	+kmod-zram
  TITLE:=ZRAM swap scripts
  PKGARCH:=all
endef

define Package/zram-swap/description
 A script to activate swaping on a compressed zram partition. This
 could be used to increase the available memory, by using compressed
 memory.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/zram-swap/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/zram.init $(1)/etc/init.d/zram
endef

$(eval $(call BuildPackage,zram-swap))
