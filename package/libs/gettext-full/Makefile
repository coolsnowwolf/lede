#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gettext-full
PKG_VERSION:=0.19.8.1
PKG_RELEASE:=1

PKG_SOURCE:=gettext-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gettext
PKG_HASH:=105556dbc5c3fbbc2aa0edb46d22d055748b6f5c7cd7a8d99f8e7eb84e938be4
PKG_BUILD_DIR:=$(BUILD_DIR)/gettext-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/gettext-$(PKG_VERSION)

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-3.0+

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0
PKG_BUILD_DEPENDS:=gettext-full/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/libintl-full
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU Internationalization library
  URL:=http://www.gnu.org/software/gettext/
endef

TARGET_CFLAGS += $(FPIC)
ifneq ($(HOST_OS),Linux)
  TARGET_CFLAGS += -I$(STAGING_DIR_HOSTPKG)/include
endif
ifdef CONFIG_USE_MUSL
  TARGET_CFLAGS += -D__UCLIBC__
endif

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-libasprintf \
	--disable-rpath \
	--enable-nls \
	--disable-java \
	--disable-native-java \
	--disable-openmp \
	--disable-curses \
	--with-included-gettext \
	--without-libintl-prefix \
	--without-libexpat-prefix \
	--without-emacs

HOST_CONFIGURE_ARGS += \
	--disable-shared \
	--enable-static \
	--disable-libasprintf \
	--disable-rpath \
	--disable-java \
	--disable-native-java \
	--disable-openmp \
	--without-emacs \
	--without-libxml2-prefix


HOST_CONFIGURE_VARS += \
	EMACS="no" \
	am_cv_lib_iconv=no \
	am_cv_func_iconv=no \
	ac_cv_header_iconv_h=no \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/libintl-full/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libintl.h $(1)/usr/lib/libintl-full/include/

	$(INSTALL_DIR) $(1)/usr/lib/libintl-full/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libintl.{a,so*} $(1)/usr/lib/libintl-full/lib/

	$(INSTALL_DIR) $(1)/usr/share/aclocal
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/* $(1)/usr/share/aclocal/

	$(SED) '/read dummy/d' $(STAGING_DIR_HOSTPKG)/bin/gettextize
endef

define Package/libintl-full/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libintl.so* $(1)/usr/lib/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libintl-full))
