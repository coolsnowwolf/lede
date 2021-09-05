#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vpnc-scripts
PKG_VERSION:=20151220
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/vpnc-scripts
  SECTION:=net
  CATEGORY:=Network
  TITLE:=VPN configuration script for vpnc and OpenConnect
  MAINTAINER:=Nikos Mavrogiannopoulos <n.mavrogiannopoulos@gmail.com>
  SUBMENU:=VPN
  PKGARCH:=all
endef

define Package/vpnc-scripts/description
	This package contains the vpnc-script which is used by vpnc
	and OpenConnect to configure the tunnel interface.
endef

define Build/Compile
endef

define Package/vpnc-scripts/install
	$(INSTALL_DIR) $(1)/lib/netifd
	$(INSTALL_BIN) ./files/vpnc-script $(1)/lib/netifd/
endef

$(eval $(call BuildPackage,vpnc-scripts))
