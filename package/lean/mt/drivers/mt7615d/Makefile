#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kmod-mt7615d
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	CATEGORY:=Kernel modules
	SUBMENU:=Wireless Drivers
	TITLE:=MTK wifi AP driver (DBDC)
	DEPENDS:=
endef

define Package/$(PKG_NAME)/config
config 7615_DBDC_MODE
	bool "MTK wifi AP driver (DBDC)"
	default y if TARGET_ramips_mt7621_DEVICE_phicomm_k2p
	default n
endef

define Package/$(PKG_NAME)/description
MTK wifi AP driver (DBDC) for K2P
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
