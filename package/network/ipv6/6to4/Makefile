#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=6to4
PKG_VERSION:=12
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/6to4
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@IPV6 +kmod-sit
  TITLE:=IPv6-to-IPv4 configuration support
  MAINTAINER:=Jo-Philipp Wich <xm@subsignal.org>
  PKGARCH:=all
endef

define Package/6to4/description
Provides support for 6to4 tunnels in /etc/config/network.
Refer to http://wiki.openwrt.org/doc/uci/network for
configuration details.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/6to4/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/6to4.sh $(1)/lib/netifd/proto/6to4.sh
endef

$(eval $(call BuildPackage,6to4))
