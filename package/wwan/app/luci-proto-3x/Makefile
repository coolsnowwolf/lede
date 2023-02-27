#
# Copyright (C) 2007-2013 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-proto-3x
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_MAINTAINER:=Dairyman
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define Package/luci-proto-3x/Default
  VERSION:=$(PKG_VERSION)-$(PKG_RELEASE)
  URL:=http://openwrt.org/
  MAINTAINER:=Dairyman
endef

define Package/luci-proto-3x
$(call Package/luci-proto-3x/Default)
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=5. Protocols
  TITLE:=Support for 3x
endef

define Package/luci-proto-3x/description
 This package contains LuCI support for 3x
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile/Default
endef

Build/Compile = $(Build/Compile/Default)

define Package/luci-proto-3x/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,luci-proto-3x))
