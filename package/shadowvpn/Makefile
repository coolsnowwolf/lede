#
# Copyright (C) 2015 OpenWrt-dist
# Copyright (C) 2015 Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ShadowVPN
PKG_VERSION:=0.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/aa65535/openwrt-shadowvpn/releases/download/v$(PKG_VERSION)
PKG_MD5SUM:=b722b0e72608e00bca00bd4061e37209

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=clowwindy <clowwindy42@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ShadowVPN
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A fast, safe VPN based on libsodium
	URL:=https://github.com/clowwindy/ShadowVPN
	DEPENDS:=+kmod-tun +ip
endef

define Package/ShadowVPN/description
A fast, safe VPN based on libsodium
endef

define Package/ShadowVPN/conffiles
/etc/config/shadowvpn
endef

define Package/ShadowVPN/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/shadowvpn.init $(1)/etc/init.d/shadowvpn
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/shadowvpn.config $(1)/etc/config/shadowvpn
	$(INSTALL_DIR) $(1)/etc/shadowvpn
	$(INSTALL_DATA) ./files/client_up.sh $(1)/etc/shadowvpn/client_up.sh
	$(INSTALL_DATA) ./files/client_down.sh $(1)/etc/shadowvpn/client_down.sh
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/shadowvpn $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/shadowvpn.hotplug $(1)/etc/hotplug.d/iface/30-shadowvpn
endef

$(eval $(call BuildPackage,ShadowVPN))
