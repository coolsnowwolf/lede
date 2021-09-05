#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libantlr3c
PKG_VERSION:=3.4
PKG_RELEASE:=1
PKG_HASH:=ca914a97f1a2d2f2c8e1fca12d3df65310ff0286d35c48b7ae5f11dcc8b2eb52

PKG_SOURCE_URL:=https://www.antlr3.org/download/C
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_MAINTAINER:=Espen Jürgensen <espenjurgensen+openwrt@gmail.com>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/libantlr3c
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libantlr3c
  URL:=https://www.antlr3.org/
endef

define Package/libantlr3c/description
  ANother Tool for Language Recognition, is a language tool that provides a framework for constructing recognizers,
  interpreters, compilers, and translators from grammatical descriptions containing actions in a variety of target languages.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libantlr3c.{a,so} $(1)/usr/lib/
endef

define Package/libantlr3c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libantlr3c.so  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libantlr3c))
