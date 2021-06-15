# Copyright (C) 2016 LEDE project

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-keyring
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/keyring.git
PKG_SOURCE_DATE:=2021-02-20
PKG_SOURCE_VERSION:=49283916005d7868923d34ab34f14188cf74812d
PKG_MIRROR_HASH:=7b58592bb49e4b37c8e80904c8f457ce3f0f2e6b1d2c473ccfe9204a8b7be831

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/openwrt-keyring
  SECTION:=base
  CATEGORY:=Base system
  PROVIDES:=lede-keyring
  TITLE:=OpenWrt Developer Keyring
  URL:=https://openwrt.org/docs/guide-user/security/signatures
endef

define Package/openwrt-keyring/description
  The keyring of with the developer using and gpg public keys.
endef

Build/Compile=

define Package/openwrt-keyring/install
	$(INSTALL_DIR) $(1)/etc/opkg/keys/
	# Public usign key for unattended snapshot builds
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/b5043e70f9a75cde $(1)/etc/opkg/keys/
endef

$(eval $(call BuildPackage,openwrt-keyring))
