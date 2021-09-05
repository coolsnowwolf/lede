#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libesmtp
PKG_VERSION:=1.0.6
PKG_RELEASE:=5

PKG_MAINTAINER:=Othmar Truniger <github@truniger.ch>
PKG_LICENSE:=LGPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://ftp.osuosl.org/pub/blfs/conglomeration/libesmtp
PKG_HASH:=d0a61a5c52d99fa7ce7d00ed0a07e341dbda67101dbed1ab0cdae3f37db4eb0b

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libesmtp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A Library for Posting Electronic Mail
  URL:=https://libesmtp.github.io/
  DEPENDS:=+libpthread +libopenssl
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/libesmtp-config $(1)/usr/bin/
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/libesmtp-config

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libesmtp.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/libesmtp-config $(2)/bin/
endef

define Package/libesmtp/install
	$(INSTALL_DIR)  $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libesmtp.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libesmtp))
