#
# Copyright (C) 2011-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnatpmp
PKG_VERSION:=20150609
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://miniupnp.tuxfamily.org/files
PKG_HASH:=e1aa9c4c4219bc06943d6b2130f664daee213fb262fcb94dd355815b8f4536b0

PKG_MAINTAINER:=
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libnatpmp/Default
  TITLE:=NAT Port Mapping Protocol (NAT-PMP)
  URL:=https://miniupnp.tuxfamily.org/libnatpmp.html
endef

define Package/libnatpmp/Default/description
  libnatpmp is an attempt to make a portable and fully compliant implementation
  of the protocol for the client side. It is based on non blocking sockets and
  all calls of the API are asynchronous. It is therefore very easy to integrate
  the NAT-PMP code to any event driven code.
endef

define Package/libnatpmp
  $(call Package/libnatpmp/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= library
  ABI_VERSION:=1
endef

define Package/libnatpmp/description
  $(call Package/libnatpmp/Default/description)
  This package contains the shared library.
endef

define Package/natpmpc
  $(call Package/libnatpmp/Default)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE+= client
  DEPENDS:=+libnatpmp
endef

define Package/natpmpc/description
  $(call Package/libnatpmp/Default/description)
  This package contains the natpmp client.
endef

define Package/libnatpmp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnatpmp.so.* $(1)/usr/lib/
endef

define Package/natpmpc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/natpmpc $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libnatpmp))
$(eval $(call BuildPackage,natpmpc))
