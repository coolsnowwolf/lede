#
# Copyright (c) 2016-2021 Dirk Brenken (dev@brenken.org)
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=travelmate
PKG_VERSION:=2.0.3
PKG_RELEASE:=1
PKG_LICENSE:=GPL-3.0-or-later
PKG_MAINTAINER:=Dirk Brenken <dev@brenken.org>

include $(INCLUDE_DIR)/package.mk

define Package/travelmate
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A wlan connection manager for travel router
	DEPENDS:=+iwinfo +jshn +jsonfilter +curl +ca-bundle
	PKGARCH:=all
endef

define Package/travelmate/description
A wlan connection manager for travel router.
Please see https://github.com/openwrt/packages/blob/master/net/travelmate/files/README.md for further information.

endef

define Package/travelmate/conffiles
/etc/config/travelmate
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/travelmate/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/travelmate.sh $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/travelmate.init $(1)/etc/init.d/travelmate

	$(INSTALL_DIR) $(1)/etc/hotplug.d/ntp
	$(INSTALL_BIN) ./files/travelmate_ntp.hotplug $(1)/etc/hotplug.d/ntp/25-travelmate_ntp

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/travelmate.conf $(1)/etc/config/travelmate

	$(INSTALL_DIR) $(1)/etc/travelmate
	$(INSTALL_BIN) ./files/travelmate.vpn $(1)/etc/travelmate
	$(INSTALL_BIN) ./files/travelmate.mail $(1)/etc/travelmate
	$(INSTALL_BIN) ./files/*.login $(1)/etc/travelmate
endef

$(eval $(call BuildPackage,travelmate))
