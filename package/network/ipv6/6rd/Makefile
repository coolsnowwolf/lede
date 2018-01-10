#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=6rd
PKG_VERSION:=9
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/6rd
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@IPV6 +kmod-sit
  TITLE:=6rd configuration support
  MAINTAINER:=Steven Barth <cyrus@openwrt.org>
  PKGARCH:=all
endef

define Package/6rd/description
Provides support for 6rd tunnels in /etc/config/network.
Refer to http://wiki.openwrt.org/doc/uci/network for
configuration details.
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/6rd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/6rdcalc $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/6rd.sh $(1)/lib/netifd/proto/6rd.sh
endef

$(eval $(call BuildPackage,6rd))
