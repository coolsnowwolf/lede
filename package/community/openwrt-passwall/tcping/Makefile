#
# Copyright (C) 2014 OpenWrt-dist
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk


PKG_NAME:=tcping
PKG_VERSION:=0.3
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=license.txt

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Lienol/tcping
PKG_MIRROR_HASH:=79414cd8e1d124422a36b8fe36a1f296b7d9bde99807b2c90ad81bbd65e200e0
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=db9101834732dac9aaa59dbb7fb9c74612dbf723
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/tcping
	SECTION:=net
	CATEGORY:=Network
	TITLE:=tcping measures the latency of a tcp-connection
	URL:=https://github.com/jlyo/tcping
endef

define Package/tcping/description
endef

define Package/tcping/conffiles
endef

define Package/tcping/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcping $(1)/usr/sbin
endef

$(eval $(call BuildPackage,tcping))
