#
# Copyright (C) 2016 Ben Rosser <rosser.bjr@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=quasselc
PKG_SOURCE_DATE:=2017-01-11
PKG_SOURCE_VERSION:=a0a1e6bd87d3eac68b5369972d1c2035cfe47e94
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_DATE).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/phhusson/QuasselC/tar.gz/$(PKG_SOURCE_VERSION)?
PKG_HASH:=fe7b48a13c0e6dad81cdae18069d4f5607af64d73a3201f42d79548170dde510
PKG_BUILD_DIR:=$(BUILD_DIR)/QuasselC-$(PKG_SOURCE_VERSION)

PKG_MAINTAINER:=Ben Rosser <rosser.bjr@gmail.com>
PKG_LICENSE:=LGPL-3.0
PKG_LICENSE_FILES:=COPYING.LESSER

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

MAKE_FLAGS += prefix=$(STAGING_DIR)/usr libdir=$(STAGING_DIR)/usr/lib includedir=$(STAGING_DIR)/usr/include
MAKE_INSTALL_FLAGS += prefix=/usr libdir=/usr/lib includedir=/usr/include

define Package/quasselc
    SECTION:=libs
    CATEGORY:=Libraries
    DEPENDS:=+glib2
    SUBMENU:=Instant Messaging
    URL:=https://github.com/phhusson/QuasselC
    TITLE:=API to access a Quassel Core in pure C
endef

define Package/quasselc/description
  An implementation of the Quassel protocol in pure C.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/quasselc
	$(CP) $(PKG_INSTALL_DIR)/usr/include/quasselc/* $(1)/usr/include/quasselc/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libquasselc.so* $(1)/usr/lib/
	$(LN) libquasselc.so.0 $(1)/usr/lib/libquasselc.so

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/quasselc.pc $(1)/usr/lib/pkgconfig/
endef

define Package/quasselc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libquasselc.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,quasselc))
