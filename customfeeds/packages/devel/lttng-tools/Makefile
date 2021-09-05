#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lttng-tools
PKG_VERSION:=2.12.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://lttng.org/files/$(PKG_NAME)/
PKG_HASH:=0de7afc1f40a5acbede933cdfd6cf47b32ff84d02e170a1321f7fc86141585b8

PKG_MAINTAINER:=
PKG_LICENSE:=LGPL-2.1 GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/lttng-tools
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Linux Trace Toolkit: next generation (tools)
  URL:=https://lttng.org/
  DEPENDS:= +lttng-ust +libpopt +libxml2 $(ICONV_DEPENDS)
endef

CONFIGURE_ARGS += \
	--enable-epoll \
	--disable-man-pages \
	--disable-python-bindings \
	--disable-test-python-agent-all \
	--disable-test-sdt-uprobe \
	--disable-bin-lttng-consumerd \
	--disable-extras \
	--without-kmod \
	--with-pic

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lttng $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblttng-ctl*.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lttng-ctl.pc $(1)/usr/lib/pkgconfig/
endef

define Package/lttng-tools/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblttng-ctl*.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lttng* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,lttng-tools))
