#
# Copyright (C) 2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openpgm
PKG_VERSION:=5.3.128
PKG_RELEASE:=1

PKG_SOURCE_URL_FILE:=release-5-3-128
PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_URL_FILE).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/steve-o/openpgm/tar.gz
PKG_HASH:=8d707ef8dda45f4a7bc91016d7f2fed6a418637185d76c7ab30b306499c6d393
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_URL_FILE)

PKG_FIXUP:=autoreconf
MAKE_PATH:=openpgm/pgm
PKG_AUTOMAKE_PATHS:=$(MAKE_PATH)

PKG_MAINTAINER:=Ye Holmes <yeholmes@outlook.com>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=$(MAKE_PATH)/LICENSE
PKG_CPE_ID:=cpe:/a:openpgm:openpgm

include $(INCLUDE_DIR)/package.mk

define Package/openpgm
  TITLE:=OpenPGM, an implementation of the PGM protocol
  URL:=http://openpgm.googlecode.com/
  SECTION:=libs
  CATEGORY:=Libraries
endef

define Package/openpgm/description
  OpenPGM is a library implementing the PGM reliable multicast
  network protocol. For more information about OpenPGM, see:
  http://openpgm.googlecode.com/
endef

CONFIGURE_VARS += ac_cv_file__proc_cpuinfo=yes \
    ac_cv_file__dev_rtc=no ac_cv_file__dev_hpet=no
CONFIGURE_ARGS += --enable-static=no --enable-shared=yes

define Build/Configure
	$(call Build/Configure/Default,,,$(MAKE_PATH))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/pgm
	$(CP) $(PKG_BUILD_DIR)/$(MAKE_PATH)/include/pgm/* $(1)/usr/include/pgm/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/$(MAKE_PATH)/.libs/libpgm*.so* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(MAKE_PATH)/openpgm-5.3.pc $(1)/usr/lib/pkgconfig/
endef

define Package/openpgm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/$(MAKE_PATH)/.libs/libpgm*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,openpgm))
