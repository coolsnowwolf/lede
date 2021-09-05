#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libgee
PKG_VERSION:=0.20.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNOME/libgee/0.20
PKG_HASH:=524c1bf390f9cdda4fbd9a47b269980dc64ab5280f0801b53bc69d782c72de0e

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=vala/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libgee
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2
  TITLE:=libgee
  URL:=https://wiki.gnome.org/Libgee
endef

define Package/libgee/decription
  Libgee is an utility library providing GObject-based interfaces and classes
endef

CONFIGURE_ARGS += \
	--disable-benchmark \
	--disable-coverage \
	--disable-doc \
	--disable-internal-asserts \
	--disable-introspection \
	--disable-vala-fatal-warnings

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/gee-0.8/ \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig/
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/share/vala-`$(STAGING_DIR_HOSTPKG)/bin/valac --api-version`/vapi/
	$(INSTALL_DATA) \
                $(PKG_INSTALL_DIR)/usr/share/vala/vapi/* \
                $(STAGING_DIR_HOSTPKG)/share/vala-`$(STAGING_DIR_HOSTPKG)/bin/valac --api-version`/vapi
endef

define Package/libgee/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libgee))
