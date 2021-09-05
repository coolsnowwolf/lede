#
# Copyright (C) 2017-2018 Daniel Engberg <daniel.engberg.lists@pyret.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libidn2
PKG_VERSION:=2.3.0
PKG_RELEASE:=1

PKG_SOURCE_URL:=@GNU/libidn
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=e1cb1db3d2e249a6a3eb6f0946777c2e892d5c5dc7bd91c74394fc3a01cab8b5

PKG_MAINTAINER:=
PKG_CPE_ID:=cpe:/a:libidn2_project:libidn2

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/idn2/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://www.gnu.org/software/libidn/
endef

define Package/idn2/Default/description
  Libidn2 is a free software implementation of IDNA2008,
  Punycode and TR46 in library form. It contains
  functionality to convert internationalized domain
  names to and from ASCII Compatible Encoding (ACE),
  following the IDNA2008 and TR46 standards.
endef

define Package/idn2
  $(call Package/idn2/Default)
  SUBMENU:=IP Addresses and Names
  TITLE:=GNU IDN2 (Internationalized Domain Name) tool
  DEPENDS:=+libidn2
  LICENSE:=GPL-2.0-or-later
  LICENSE_FILES:=COPYINGv2
endef

define Package/idn2/description
$(call Package/idn2/Default/description)

  Command line tool using libidn2

endef

define Package/libidn2
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libunistring $(ICONV_DEPENDS) $(INTL_DEPENDS)
  TITLE:=International domain name library (IDNA2008, Punycode and TR46)
  URL:=https://www.gnu.org/software/libidn/#libidn2
  LICENSE:=LGPL-3.0-or-later
  LICENSE_FILES:=COPYING.LESSERv3
endef

define Package/libidn2/description
$(call Package/idn2/Default/description)

  Library only package

endef

CONFIGURE_ARGS += \
	--disable-rpath \
	--disable-doc

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/idn2.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{la,so}* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libidn2.pc $(1)/usr/lib/pkgconfig/
endef

define Package/idn2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Package/libidn2/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,idn2))
$(eval $(call BuildPackage,libidn2))
