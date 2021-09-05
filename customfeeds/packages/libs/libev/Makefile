#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libev
PKG_VERSION:=4.31
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://dist.schmorp.de/libev/Attic/
PKG_HASH:=ed855d2b52118e32c0c1a6a32bd18c97f9e6711ca511f5ee12de3b9eccc66e5a
PKG_LICENSE:=BSD-2-Clause
PKG_MAINTAINER:=Karl Palsson <karlp@tweak.net.au>

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/libev
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=High-performance event loop
  URL:=http://software.schmorp.de/pkg/libev.html
endef

define Package/libev/description
 A full-featured and high-performance event loop that is loosely modelled after
 libevent, but without its limitations and bugs.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/event.h $(1)/usr/include/ev_event_compat.h
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ev.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ev++.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libev.{a,so*} $(1)/usr/lib/
endef

define Package/libev/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libev.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libev))
