#
# Copyright (c) 2018-2021 Dirk Brenken (dev@brenken.org)
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=banip
PKG_VERSION:=0.7.9
PKG_RELEASE:=1
PKG_LICENSE:=GPL-3.0-or-later
PKG_MAINTAINER:=Dirk Brenken <dev@brenken.org>

include $(INCLUDE_DIR)/package.mk

define Package/banip
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Ban incoming and outgoing ip adresses via ipsets
	DEPENDS:=+jshn +jsonfilter +ip +ipset +iptables +ca-bundle
	PKGARCH:=all
endef

define Package/banip/description
Powerful banIP script to block ip addresses via ipsets.
The script supports many ip blacklist sites plus manual black- and whitelist overrides.
Please see https://github.com/openwrt/packages/blob/master/net/banip/files/README.md for further information.

endef

define Package/banip/conffiles
/etc/config/banip
/etc/banip/banip.maclist
/etc/banip/banip.blacklist
/etc/banip/banip.whitelist
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/banip/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/banip.sh $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/banip.init $(1)/etc/init.d/banip

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/banip.conf $(1)/etc/config/banip

	$(INSTALL_DIR) $(1)/etc/banip
	$(INSTALL_BIN) ./files/banip.dns $(1)/etc/banip
	$(INSTALL_BIN) ./files/banip.mail $(1)/etc/banip
	$(INSTALL_BIN) ./files/banip.service $(1)/etc/banip
	$(INSTALL_CONF) ./files/banip.maclist $(1)/etc/banip
	$(INSTALL_CONF) ./files/banip.blacklist $(1)/etc/banip
	$(INSTALL_CONF) ./files/banip.whitelist $(1)/etc/banip
	$(INSTALL_CONF) ./files/banip.countries $(1)/etc/banip
	$(INSTALL_CONF) ./files/banip.sources $(1)/etc/banip
	gzip -9 $(1)/etc/banip/banip.sources

	$(INSTALL_DIR) $(1)/etc/hotplug.d/firewall
	$(INSTALL_DATA) ./files/banip.hotplug $(1)/etc/hotplug.d/firewall/30-banip
endef

$(eval $(call BuildPackage,banip))
