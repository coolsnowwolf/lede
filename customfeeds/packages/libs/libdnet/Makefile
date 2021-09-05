#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdnet
PKG_VERSION:=1.14
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ofalk/libdnet/tar.gz/$(PKG_NAME)-$(PKG_VERSION)?
PKG_HASH:=592599c54a57102a177270f3a2caabda2c2ac7768b977d7458feba97da923dfe
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Luka Perkov <luka@openwrt.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libdnet
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Low-level network library
  URL:=https://github.com/ofalk/libdnet
endef

define Package/libdnet/description
 libdnet is a library of simplified, portable interface to several low-level
 networking routines.
endef

CONFIGURE_ARGS += \
	--without-check \
	--without-python \
	--without-wpdpack

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dnet-config $(1)/usr/bin/
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/dnet-config
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/dnet-config $(2)/bin/

	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA)\
		$(PKG_INSTALL_DIR)/usr/include/dnet.h \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/dnet \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libdnet.{la,a,so*} \
		$(1)/usr/lib/
endef

define Package/libdnet/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libdnet.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdnet))
