#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ipip6
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/ipip6
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@IPV6 +kmod-ip6-tunnel +resolveip
  TITLE:=IPv4 over IPv6 (ipip6) configuration support
  MAINTAINER:=Missing <ms@missing233.com>
  PKGARCH:=all
endef

define Package/ipip6/description
Provides support for IPv4 over IPv6 (ipip6) in /etc/config/network.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/ipip6/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/ipip6.sh $(1)/lib/netifd/proto/ipip6.sh
endef

$(eval $(call BuildPackage,ipip6))
