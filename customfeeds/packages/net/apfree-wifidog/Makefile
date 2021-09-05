#
# Copyright (C) 2018 Dengfeng Liu
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=apfree-wifidog
PKG_VERSION:=4.08.1771
PKG_RELEASE:=6

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/liudf0716/apfree_wifidog.git
PKG_SOURCE_VERSION:=$(PKG_VERSION)
PKG_MIRROR_HASH:=0fadb7a0e139bf4a5c6daffdd0f200ee2a4c903aea71e46bc19212b054ca71a7

PKG_MAINTAINER:=Dengfeng Liu <liudf0716@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/apfree-wifidog
  SUBMENU:=Captive Portals
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+zlib +iptables-mod-extra +iptables-mod-ipopt +kmod-ipt-nat +iptables-mod-nat-extra \
           +libjson-c +ipset +libip4tc +libevent2 +libevent2-openssl +libuci +px5g
  TITLE:=Apfree's wireless captive portal solution
  URL:=https://github.com/liudf0716/apfree_wifidog
endef

define Package/apfree-wifidog/description
  The ApFree Wifidog project is a complete and embeddable captive portal
  solution for wireless community groups or individuals who wish to open a free
  Hotspot while still preventing abuse of their Internet connection.
  It's enhanced wifidog
endef

define Package/apfree-wifidog/conffiles
/etc/config/wifidogx
endef

define Package/apfree-wifidog/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifidogx $(1)/usr/bin/wifidogx
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wdctlx $(1)/usr/bin/wdctlx
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/wdping $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-msg.html $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-redir.html $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-redir.html.front $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-redir.html.rear $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/authserver-offline.html $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/internet-offline.html $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wifidogx.init $(1)/etc/init.d/wifidogx
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) ./files/wifidogx.conf $(1)/etc/config/wifidogx
endef

$(eval $(call BuildPackage,apfree-wifidog))
