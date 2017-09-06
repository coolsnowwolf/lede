#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zlib
PKG_VERSION:=1.2.11
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SF/libpng http://www.zlib.net
PKG_HASH:=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066

PKG_LICENSE:=Zlib
PKG_LICENSE_FILES:=README

include $(INCLUDE_DIR)/package.mk

define Package/zlib
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library implementing the deflate compression method
  URL:=http://www.zlib.net/
endef

define Package/zlib-dev
  SECTION:=devel
  CATEGORY:=Development
  SUBMENU:=Libraries
  DEPENDS:=zlib
  TITLE:=Development files for the zlib library
endef

define Package/zlib/description
 zlib is a lossless data-compression library.
 This package includes the shared library.
endef

define Package/zlib-dev/description
 zlib is a lossless data-compression library.
 This package includes the development support files.
endef

CONFIGURE_VARS := \
	$(TARGET_CONFIGURE_OPTS) \
	LDSHARED="$(TARGET_CC) -shared -Wl,-soname,libz.so.1" \
	CFLAGS="$(TARGET_CFLAGS) $(FPIC)" \

CONFIGURE_ARGS := \
	--prefix=/usr \
	--shared \
	--uname=Linux \

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		libz.a libz.so.$(PKG_VERSION)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		install
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP)	$(PKG_INSTALL_DIR)/usr/include/z{conf,lib}.h \
		$(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP)	$(PKG_INSTALL_DIR)/usr/lib/libz.{a,so*} \
		$(1)/usr/lib/
	mkdir -p $(1)/usr/lib/pkgconfig
	$(CP)	$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/zlib.pc \
		$(1)/usr/lib/pkgconfig/
endef

# libz.so is needed for openssl (zlib-dynamic)
define Package/zlib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libz.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libz.so.* $(1)/usr/lib/
endef

define Package/zlib-dev/install
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/zconf.h \
	  $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/zlib.h \
	  $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libz.a $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/zlib.pc \
	  $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,zlib))
$(eval $(call BuildPackage,zlib-dev))
