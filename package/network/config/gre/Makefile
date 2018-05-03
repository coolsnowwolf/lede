#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gre
PKG_VERSION:=1
PKG_RELEASE:=8
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/gre/Default
endef

define Package/gre
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
  TITLE:=Generic Routing Encapsulation config support
  DEPENDS:=+kmod-gre +IPV6:kmod-gre6 +resolveip
  PROVIDES:=grev4 grev6
endef

define Package/gre/description
 Generic Routing Encapsulation config support (IPv4 and IPv6) in /etc/config/network.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/gre/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/gre.sh $(1)/lib/netifd/proto/gre.sh
endef

$(eval $(call BuildPackage,gre))
