#
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libidn
PKG_VERSION:=1.36
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/libidn
PKG_HASH:=14b67108344d81ba844631640df77c9071d9fb0659b080326ff5424e86b14038

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-or-later GPL-3.0-or-later LGPL-2.1-or-later LGPL-3.0-or-later Apache-2.0
PKG_LICENSE_FILES:=COPYING COPYINGv2 COPYINGv3 COPYING.LESSERv2 COPYING.LESSERv3 java/LICENSE-2.0.txt

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/idn/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://www.gnu.org/software/libidn/
endef

define Package/idn/Default/description
  GNU Libidn is a fully documented implementation of the Stringprep,
  Punycode and IDNA specifications. Libidn's purpose is to encode and
  decode internationalized domain names.
endef

define Package/idn
  $(call Package/idn/Default)
  SUBMENU:=IP Addresses and Names
  TITLE:=GNU IDN (Internationalized Domain Name) tool
  DEPENDS:=+libidn
endef

define Package/idn/description
$(call Package/idn/Default/description)

  Command line tool using libidn

endef

define Package/libidn
  $(call Package/idn/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Stringprep, Punycode and IDNA implementation
endef

define Package/libidn/description
$(call Package/idn/Default/description)

  Library only package

endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
        --disable-rpath \
        --disable-doc

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libidn.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libidn.pc $(1)/usr/lib/pkgconfig/
endef

define Package/idn/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Package/libidn/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libidn.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,idn))
$(eval $(call BuildPackage,libidn))
