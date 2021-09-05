#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libxml2
PKG_VERSION:=2.9.12
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://xmlsoft.org/sources/
PKG_HASH:=c8d6681e38c56f172892c85ddc0852e1fd4b53b4209e7f4ebf17f7e2eae71d92

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:xmlsoft:libxml2

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libxml2
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Gnome XML library
  URL:=http://xmlsoft.org/
  DEPENDS:=+libpthread +zlib $(ICONV_DEPENDS)
endef

define Package/libxml2/description
  A library for manipulating XML and HTML resources.
endef

define Package/libxml2-dev
  SECTION:=devel
  CATEGORY:=Development
  SUBMENU:=Libraries
  TITLE:=Development files for libxml2
  URL:=http://xmlsoft.org/
  DEPENDS:=+libxml2
endef

define Package/libxml2-dev/description
  A library for manipulating XML and HTML resources.

  This package contains the headers and xml2-config binary.
endef

define Package/libxml2-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=XML command line utilities (xmllint...)
  URL:=http://xmlsoft.org/
  DEPENDS:=+libxml2
endef

define Package/libxml2-utils/description
  This package contains the binaries xmllint and xmlcatalog
  from libxml2, a library for manipulating XML and HTML resources.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--with-c14n \
	--without-catalog \
	--with-debug \
	--without-docbook \
	--with-html \
	--without-ftp \
	--without-http \
	--without-iso8859x \
	--without-legacy \
	--with-output \
	--without-pattern \
	--without-push \
	--without-python \
	--with-reader \
	--without-readline \
	--without-regexps \
	--with-sax1 \
	--with-schemas \
	--with-threads \
	--with-tree \
	--with-valid \
	--with-writer \
	--with-xinclude \
	--with-xpath \
	--with-xptr \
	--with-zlib=$(STAGING_DIR)/usr \
	--without-lzma

HOST_CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--with-c14n \
	--without-catalog \
	--with-debug \
	--without-docbook \
	--with-html \
	--without-ftp \
	--without-http \
	--without-iconv \
	--without-iso8859x \
	--without-legacy \
	--with-output \
	--without-pattern \
	--without-push \
	--without-python \
	--with-reader \
	--without-readline \
	--without-regexps \
	--with-sax1 \
	--with-schemas \
	--with-threads \
	--with-tree \
	--with-valid \
	--with-writer \
	--with-xinclude \
	--with-xpath \
	--with-xptr \
	--with-zlib \
	--without-lzma

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/xml2-config \
		$(2)/bin/$(GNU_TARGET_NAME)-xml2-config
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' \
		$(2)/bin/$(GNU_TARGET_NAME)-xml2-config
	$(LN) $(GNU_TARGET_NAME)-xml2-config $(2)/bin/xml2-config

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libxml2 $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxml2.{la,a,so*} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/cmake/libxml2
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cmake/libxml2/libxml2-config.cmake \
		$(1)/usr/lib/cmake/libxml2

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libxml-2.0.pc $(1)/usr/lib/pkgconfig/

	$(INSTALL_DIR) $(2)/share/aclocal/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/* $(2)/share/aclocal
endef

define Host/Install
	$(call Host/Install/Default)
	mv $(1)/bin/xml2-config $(1)/bin/$(GNU_HOST_NAME)-xml2-config
	$(LN) $(GNU_HOST_NAME)-xml2-config $(1)/bin/xml2-config
endef

define Package/libxml2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxml2.so.* $(1)/usr/lib/
endef

define Package/libxml2-dev/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/xml2-config $(1)/usr/bin/
	$(SED) "s,$(STAGING_DIR),,g" $(1)/usr/bin/xml2-config

	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxml2.so $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/{cmake,pkgconfig}
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/{cmake,pkgconfig} $(1)/usr/lib/
	$(SED) "s,$(STAGING_DIR),,g" $(1)/usr/lib/pkgconfig/*.pc

	$(INSTALL_DIR) $(1)/usr/share/aclocal
	$(CP) $(PKG_INSTALL_DIR)/usr/share/aclocal/* $(1)/usr/share/aclocal
endef

define Package/libxml2-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/xmllint $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/xmlcatalog $(1)/usr/bin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libxml2))
$(eval $(call BuildPackage,libxml2-dev))
$(eval $(call BuildPackage,libxml2-utils))
