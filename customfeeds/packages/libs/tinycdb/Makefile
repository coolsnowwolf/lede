# 
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tinycdb
PKG_RELEASE:=2
PKG_SOURCE_URL:=http://www.corpit.ru/mjt/tinycdb/
PKG_VERSION:=0.78
PKG_HASH:=50678f432d8ada8d69f728ec11c3140e151813a7847cf30a62d86f3a720ed63c
PKG_MAINTAINER:=Denis Shulyaka <Shulyaka@gmail.com>
PKG_LICENSE:=NLPL

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

# Pass CPPFLAGS in the CFLAGS as otherwise the build system will
# ignore them.
TARGET_CFLAGS+=$(TARGET_CPPFLAGS)

MAKE_FLAGS+= \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

CDB_INST_STRING:=prefix=/usr install install-sharedlib install-piclib

define Package/tinycdb
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=a Constant DataBase
  URL:=http://www.corpit.ru/mjt/tinycdb.html
  ABI_VERSION=1
endef

define Package/tinycdb/description
 TinyCDB is a very fast and simple package for creating and reading constant data bases
endef

define Package/tinycdb/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libcdb.so.$(ABI_VERSION) $(1)/usr/lib
endef

define Build/Compile
	$(call Build/Compile/Default,shared staticlib piclib)
endef

define Build/Install
	$(call Build/Install/Default,$(CDB_INST_STRING))
endef

define Build/InstallDev
	cd $(PKG_BUILD_DIR); $(MAKE) DESTDIR=$(1) $(CDB_INST_STRING)
endef

$(eval $(call BuildPackage,tinycdb))
