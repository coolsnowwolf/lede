#
# Copyright (C) 2006,2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wifidog
PKG_VERSION:=1.3.0
PKG_RELEASE:=6

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/wifidog/wifidog-gateway
PKG_SOURCE_VERSION:=$(PKG_VERSION)
PKG_MIRROR_HASH:=9ffd9f3ae54baceb723abb7a04e27a9b6a3ff1479f8a3bfda9b8a496e8b4050f

PKG_MAINTAINER:=Michael Haas <haas@computerlinguist.org>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=0

include $(INCLUDE_DIR)/package.mk

define Package/wifidog/Default
  SUBMENU:=Captive Portals
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+iptables-mod-extra +iptables-mod-ipopt +iptables-mod-nat-extra +libpthread
  TITLE:=A wireless captive portal solution
  URL:=http://www.wifidog.org
endef

define Package/wifidog
$(call Package/wifidog/Default)
  VARIANT:=normal
endef

define Package/wifidog-tls
$(call Package/wifidog/Default)
  DEPENDS+= +PACKAGE_wifidog-tls:libwolfssl
  VARIANT:=tls
endef


define Package/wifidog/description
	The Wifidog project is a complete and embeddable captive
	portal solution for wireless community groups or individuals
	who wish to open a free Hotspot while still preventing abuse
	of their Internet connection.
endef

define Package/wifidog-tls/description
$(call Package/wifidog/description)

This variant can talk TLS to the auth server.
endef

define Package/wifidog/conffiles
/etc/wifidog.conf
endef

Package/wifidog-tls/conffiles = $(Package/wifidog/conffiles)

ifeq ($(BUILD_VARIANT),tls)

CONFIGURE_ARGS += \
  --enable-cyassl

endif

define Package/wifidog/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifidog $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wdctl $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhttpd.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog.conf $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-msg.html $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wifidog.init $(1)/etc/init.d/wifidog
endef

Package/wifidog-tls/install = $(Package/wifidog/install)

$(eval $(call BuildPackage,wifidog))
$(eval $(call BuildPackage,wifidog-tls))
