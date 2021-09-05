#
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=softflowd
PKG_VERSION:=1.0.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/irino/softflowd/tar.gz/softflowd-$(PKG_VERSION)?
PKG_HASH:=98aa66026d730211b45fe89670cd6ce50959846d536880b82f5afbca6281e108
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-softflowd-$(PKG_VERSION)

PKG_MAINTAINER:=Ross Vandegrift <ross@kallisti.us>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/softflowd
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpcap
  TITLE:=softflowd
  URL:=https://github.com/irino/softflowd
endef

define Package/softflowd/description
	Software netflow exporter
endef

define Package/softflowd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/softflowd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/softflowctl $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

define Package/softflowd/conffiles
/etc/config/softflowd
endef

$(eval $(call BuildPackage,softflowd))
