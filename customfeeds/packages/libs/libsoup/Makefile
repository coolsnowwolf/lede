#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsoup
PKG_VERSION:=2.72.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNOME/$(PKG_NAME)/2.72
PKG_HASH:=170c3f8446b0f65f8e4b93603349172b1085fb8917c181d10962f02bb85f5387

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnome:libsoup

PKG_BUILD_DEPENDS:=meson/host glib2/host
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

define Package/libsoup
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libsoup
  URL:=https://wiki.gnome.org/Projects/libsoup
  DEPENDS:=+glib2 +libxml2 +libgnutls +libsqlite3 +libpsl $(ICONV_DEPENDS) $(INTL_DEPENDS)
endef

MESON_ARGS += \
	-Dgssapi=disabled \
	-Dntlm=disabled \
	-Dbrotli=disabled \
	-Dtls_check=false \
	-Dgnome=false \
	-Dintrospection=disabled \
	-Dvapi=disabled \
	-Dgtk_doc=false \
	-Dtests=false \
	-Dinstalled_tests=false \
	-Dsysprof=disabled

define package/libsoup/decription
Libsoup is an HTTP library implementation in C
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{lib/pkgconfig,include/libsoup-2.4/libsoup}

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libsoup-2.4.so* \
		$(1)/usr/lib/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* \
		$(1)/usr/lib/pkgconfig/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/libsoup-2.4/libsoup/*.h \
		$(1)/usr/include/libsoup-2.4/libsoup/
endef

define Package/libsoup/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libsoup-2.4.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsoup))
