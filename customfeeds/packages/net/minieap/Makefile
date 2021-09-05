#
# Copyright (C) 2016 Openwrt.org
# Copyright (C) 2021 ImmortalWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minieap
PKG_VERSION:=0.93
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/updateing/minieap/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=752cec83f453ab2b44503156e0a84a82c475b9a231f45d407b6b7eb75de60f68

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/minieap
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Campus Network
  TITLE:=Extendable 802.1x client
  URL:=https://github.com/updateing/minieap
endef

define Package/minieap/description
  Extendable 802.1x client with Ruijie v3 (v4) plugin.
endef

define Package/minieap/conffiles
/etc/minieap.conf
endef

define Package/minieap/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/minieap.conf $(1)/etc/minieap.conf
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/sbin/minieap $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,minieap))
