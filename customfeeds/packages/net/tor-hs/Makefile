#
# Copyright (C) 2020-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tor-hs
PKG_VERSION:=0.0.1
PKG_RELEASE:=3

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=GPL-3.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/tor-hs
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=Tor hidden service configurator
  DEPENDS:=+tor +rpcd
endef

define Package/tor-hs/description
  Tor Hidden Service configurator
endef

define Package/tor-hs/conffiles
/etc/config/tor-hs
endef

define Build/Compile
endef

define Build/Install
endef

define Package/tor-hs/postinst
#!/bin/sh
[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/rpcd restart
exit 0
endef

define Package/tor-hs/postrm
#!/bin/sh
[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/rpcd restart
exit 0
endef

define Package/tor-hs/install
	$(INSTALL_DIR) $(1)/etc/config/
	$(CP) ./files/tor-hs.conf $(1)/etc/config/tor-hs
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/tor-hs.init $(1)/etc/init.d/tor-hs
	$(INSTALL_DIR) $(1)/etc/tor/
	$(INSTALL_BIN) ./files/nextcloud-update.sh $(1)/etc/tor/
	$(INSTALL_DIR) $(1)/usr/libexec/rpcd
	$(INSTALL_BIN) ./files/tor_rpcd.sh $(1)/usr/libexec/rpcd/tor-hs-rpc
endef

$(eval $(call BuildPackage,tor-hs))
