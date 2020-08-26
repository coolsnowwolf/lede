#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zram-swap
PKG_VERSION:=1.1
PKG_RELEASE:=6

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/zram-swap
  SECTION:=utils
  CATEGORY:=Base system
  DEPENDS:=+kmod-zram +!(BUSYBOX_CONFIG_MKSWAP&&BUSYBOX_CONFIG_SWAPON&&BUSYBOX_CONFIG_SWAPOFF):swap-utils
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
