#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libradcli
PKG_VERSION:=1.2.11
PKG_RELEASE:=1

PKG_SOURCE:=radcli-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/radcli/radcli/releases/download/$(PKG_VERSION)
PKG_HASH:=b6210e4c7deae235bb8c4e9df20c4f82c8bc2bda6d6214d50c5667660ab38548
PKG_BUILD_DIR:=$(BUILD_DIR)/radcli-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libradcli
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library for radius clients
  URL:=https://radcli.github.io/radcli
  MAINTAINER:=Nikos Mavrogiannopoulos <nmav@gnutls.org>
  DEPENDS:= +RADCLI_TLS:libgnutls +libnettle
endef

define Package/libradcli/decription
  The radcli library is a library for writing RADIUS Clients. The library's
  approach is to allow writing RADIUS-aware application in less than 50 lines
  of C code. It was based originally on freeradius-client and is source 
  compatible with it.
endef

CONFIGURE_ARGS+= \
	--enable-legacy-compat

ifneq ($(CONFIG_RADCLI_TLS),y)
CONFIGURE_ARGS += --without-tls
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/radcli
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/radcli/radcli.h \
		$(PKG_INSTALL_DIR)/usr/include/radcli/version.h \
		$(1)/usr/include/radcli
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/freeradius-client.h \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libradcli.so* \
		$(PKG_INSTALL_DIR)/usr/lib/libfreeradius-client.so \
		$(1)/usr/lib/
endef

define Package/libradcli/conffiles
/etc/radcli/radiusclient.conf
endef

define Package/libradcli/install
	$(INSTALL_DIR) $(1)/etc/radcli
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/etc/radiusclient.conf $(1)/etc/radcli/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libradcli.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libradcli))
