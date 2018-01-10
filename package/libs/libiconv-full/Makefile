#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libiconv-full
PKG_VERSION:=1.11.1
PKG_RELEASE:=3

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_SOURCE:=libiconv-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/libiconv
PKG_HASH:=e78c347a1a0cb15f2648519e9799151f4b4a934b61ad9ee7424478efe2b8257f
PKG_BUILD_DIR:=$(BUILD_DIR)/libiconv-$(PKG_VERSION)
PKG_LICENSE:=LGPL-2.0
PKG_LICENSE_FILES:=COPYING.LIB

PKG_FIXUP:=patch-libtool

include $(INCLUDE_DIR)/package.mk

define Package/libiconv-full/Default
  URL:=http://www.gnu.org/software/libiconv/
  TITLE:=Character set conversion
endef

define Package/libiconv-full
  $(call Package/libiconv-full/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= library
endef

define Package/libcharset
  $(call Package/libiconv-full/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= library
endef

define Package/iconv
  $(call Package/libiconv-full/Default)
  DEPENDS:=+libiconv-full +libcharset
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= utility
endef

TARGET_CFLAGS += $(FPIC) -DUSE_DOS

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-rpath \
	--enable-relocatable

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/libiconv-full/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/iconv.h $(1)/usr/lib/libiconv-full/include/

	$(INSTALL_DIR) $(1)/usr/lib/libiconv-full/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcharset.{a,so*} $(1)/usr/lib/libiconv-full/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libiconv.{a,so*} $(1)/usr/lib/libiconv-full/lib/
endef

define Package/libcharset/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcharset.so* $(1)/usr/lib/
endef

define Package/libiconv-full/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libiconv.so* $(1)/usr/lib/
endef

define Package/iconv/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/iconv $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libcharset))
$(eval $(call BuildPackage,libiconv-full))
$(eval $(call BuildPackage,iconv))
