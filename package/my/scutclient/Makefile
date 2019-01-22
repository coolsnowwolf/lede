#
# Copyright (C) 2016-2018 SCUT Router Term
#
# This is free software, licensed under the GNU Affero General Public License v3.
# See /COPYING for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=scutclient
#If you want to compile your local source code, fill the absolute source path below.
SRCDIR:=
ifneq (,$(SRCDIR))
PKG_VERSION:=$(shell cd $(SRCDIR) && git describe --tags || echo "debug")
PKG_RELEASE:=1
PKG_UNPACK=$(CP) $(SRCDIR)/. $(PKG_BUILD_DIR)
else
PKG_VERSION:=3.1.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/scutclient/scutclient.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
endif

PKG_MAINTAINER:=Scutclient Project

PKG_LICENSE:=AGPL-3.0

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/scutclient
  SECTION:=net
  CATEGORY:=Network
  TITLE:=SCUT Dr.com client
  DEPENDS:=
  URL:=
endef

define Package/scutclient/description
 Support SCUT private authentication protocol.
endef

define Package/scutclient/conffiles
/etc/config/scutclient
endef

define Package/scutclient/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/openwrt/files/scutclient.config $(1)/etc/config/scutclient
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt/files/scutclient.init $(1)/etc/init.d/scutclient
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt/files/scutclient.hotplug $(1)/etc/hotplug.d/iface/99-scutclient
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scutclient $(1)/usr/bin
endef

$(eval $(call BuildPackage,scutclient))
