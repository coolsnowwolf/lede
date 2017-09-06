#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ipip
PKG_VERSION:=1
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/ipip
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
  TITLE:=IP in IP Tunnel config support
  DEPENDS:= +kmod-ipip +resolveip
endef

define Package/ipip/description
 IP in IP Tunnel config support in /etc/config/network.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/ipip/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/ipip.sh $(1)/lib/netifd/proto/ipip.sh
endef

$(eval $(call BuildPackage,ipip))
