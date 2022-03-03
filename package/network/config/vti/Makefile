#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vti
PKG_RELEASE:=5
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/vti/Default
endef

define Package/vti
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=Andre Valentin <avalentin@marcant.net>
  TITLE:=Virtual IPsec Tunnel Interface config support
  DEPENDS:=+kmod-ip-vti +IPV6:kmod-ip6-vti
  PROVIDES:=vtiv4 vtiv6
  PKGARCH:=all
endef

define Package/vti/description
 Virtual IPsec Tunnel Interface config support (IPv4 and IPv6) in /etc/config/network.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/vti/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/vti.sh $(1)/lib/netifd/proto/vti.sh
endef

$(eval $(call BuildPackage,vti))
