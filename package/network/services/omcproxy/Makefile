#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=omcproxy
PKG_RELEASE:=4

PKG_SOURCE_URL:=https://github.com/sbyx/omcproxy.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2017-02-14
PKG_SOURCE_VERSION:=1fe6f48f8a5067685cbcf6427baf1b3e8b889da1
PKG_MIRROR_HASH:=b5a5e09149bd08a945ada8a29a30c3e42834524747d7fd0911717f5a0ec59c72
PKG_MAINTAINER:=Steven Barth <cyrus@openwrt.org>
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/omcproxy
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libubox +libubus
  TITLE:=IGMPv3 and MLDv2 Multicast Proxy
endef

CMAKE_OPTIONS += -DWITH_LIBUBOX=1

define Package/omcproxy/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/omcproxy.config $(1)/etc/config/omcproxy
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/omcproxy.init $(1)/etc/init.d/omcproxy
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/omcproxy $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,omcproxy))
