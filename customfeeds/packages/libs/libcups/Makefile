#
# Copyright (C) 2006-2012 OpenWrt.org
# Copyright (C) 2017-2018 Luiz Angelo Daros de Luca <luizluca@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cups
PKG_VERSION:=2.2.12
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-source.tar.gz
PKG_SOURCE_URL:=https://github.com/apple/cups/releases/download/v$(PKG_VERSION)/
PKG_HASH:=0f61ab449e4748a24c6ab355b481ff7691247a140d327b2b7526fce34b7f9aa8
PKG_MAINTAINER:=Luiz Angelo Daros de Luca <luizluca@gmail.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE.txt
PKG_CPE_ID:=cpe:/a:apple:cups

include $(INCLUDE_DIR)/package.mk

define Package/libcups/Default
  URL:=http://www.cups.org/
  SUBMENU:=Printing
endef

define Package/libcups
$(call Package/cups/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+zlib +libpthread +libpng +libjpeg +libusb-1.0
  TITLE:=Common UNIX Printing System - Core library
  BUILDONLY:=1
endef

define Package/libcups/description
	Common UNIX Printing System - Core library
endef

TARGET_LDFLAGS+=-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

CONFIGURE_ARGS+=--with-cups-user="nobody" \
		--with-cups-group="nogroup" \
		--with-components="core" \
		--with-pdftops="none" \
		--without-perl \
		--without-python \
		--without-php \
		--enable-shared \
		--enable-image \
		--enable-libusb \
		--disable-acl \
		--disable-dbus \
		--disable-dnssd \
		--disable-launchd \
		--disable-ldap \
		--disable-pam \
		--disable-slp \
		--disable-gnutls \
		--disable-openssl \
		--disable-cdsassl \
		--disable-ssl \
		--disable-gssapi \
		--disable-tiff \
		UNAME="Linux" \
		LIBS="$(TARGET_LDFLAGS) -lz -lpng -ljpeg"

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/cups \
		$(TARGET_CONFIGURE_OPTS) \
		DSTROOT="$(PKG_INSTALL_DIR)" \
		STRIP="/bin/true" \
		libcups.so.2 install-libs install-headers
	$(MAKE) -C $(PKG_BUILD_DIR)/filter \
		$(TARGET_CONFIGURE_OPTS) \
		DSTROOT="$(PKG_INSTALL_DIR)" \
		STRIP="/bin/true" \
		libcupsimage.so.2 install-libs install-headers
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cups-config $(PKG_INSTALL_DIR)/usr/bin
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cups-config $(1)/usr/bin/
	$(INSTALL_DIR) $(2)/bin
	$(LN) $(STAGING_DIR)/usr/bin/cups-config $(2)/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cups $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib*/libcups*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcups))
