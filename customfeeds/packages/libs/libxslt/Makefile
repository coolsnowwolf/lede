#
# Copyright (C) 2014 - 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libxslt
PKG_VERSION:=1.1.34
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
	http://xmlsoft.org/sources/ \
	ftp://fr.rpmfind.net/pub/libxml/
PKG_HASH:=98b1bd46d6792925ad2dfe9a87452ea2adebf69dcb9919ffd55bf926a7f93f7f

PKG_MAINTAINER:=Jiri Slachta <jiri@slachta.eu>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:xmlsoft:libxslt

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
HOST_BUILD_DEPENDS:=libxml2/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libxslt
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libxml2 $(ICONV_DEPENDS)
  TITLE:=Gnome XSLT library
  URL:=http://xmlsoft.org/XSLT/
endef

define Package/libxslt/description
 A library for XML transformation using XSLT.
endef

define Package/libexslt
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libxslt
  TITLE:=Gnome XSLT library Extension
  URL:=http://xmlsoft.org/XSLT/EXSLT/
endef

define Package/libexslt/description
 An extension for XSLT.
endef

define Package/xsltproc
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libxslt +PACKAGE_xsltproc:libexslt
  TITLE:=Gnome XSLT xsltproc Utility
  URL:=http://xmlsoft.org/XSLT/
endef

define Package/xsltproc/description
 XSLT XML transformation utility.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--without-python \
	--without-crypto \
	--without-debug \
	--without-mem-debug \
	--without-debugger \
	--without-plugins

HOST_CONFIGURE_ARGS += \
	--with-libxml-prefix=$(STAGING_DIR_HOSTPKG) \
	--without-python \
	--without-crypto \
	--without-debug \
	--without-mem-debug \
	--without-debugger \
	--without-profiler \
	--without-plugins

define Build/InstallDev/Xslt
	$(INSTALL_DIR) $(1)/usr/bin $(2)/bin $(1)/usr/include/libxslt \
		$(1)/usr/include/libexslt $(1)/usr/lib \
		$(1)/usr/lib/pkgconfig $(2)/share/aclocal

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/xslt-config \
		$(2)/bin/

	ln -sf $(STAGING_DIR)/host/bin/xslt-config $(1)/usr/bin/xslt-config

	$(SED) \
		's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' \
		$(2)/bin/xslt-config

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/libxslt/* \
		$(1)/usr/include/libxslt/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libxslt.{la,a,so*} \
		$(1)/usr/lib/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libxslt.pc \
		$(1)/usr/lib/pkgconfig/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/aclocal/* \
		$(2)/share/aclocal
endef

define Build/InstallDev/Exslt
	$(INSTALL_DIR) $(1)/usr/include/libexslt $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/libexslt/* \
		$(1)/usr/include/libexslt/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libexslt.{la,a,so*} \
		$(1)/usr/lib/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libexslt.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Build/InstallDev
	 $(if $(CONFIG_PACKAGE_libxslt),$(call Build/InstallDev/Xslt,$(1),$(2)))
	 $(if $(CONFIG_PACKAGE_libexslt),$(call Build/InstallDev/Exslt,$(1),$(2)))
endef

define Package/libxslt/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libxslt.so* \
		$(1)/usr/lib/
endef

define Package/libexslt/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libexslt.so* \
		$(1)/usr/lib/
endef

define Package/xsltproc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/xsltproc \
		$(1)/usr/bin/
endef

$(eval $(call BuildPackage,libxslt))
$(eval $(call BuildPackage,libexslt))
$(eval $(call BuildPackage,xsltproc))
$(eval $(call HostBuild))
