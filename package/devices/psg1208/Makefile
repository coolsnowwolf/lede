#
# Copyright (C) 2010-2011 OpenWrt.org
# 注意取消accesscontrol
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=psg1208
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/psg1208
  TITLE:=psg1208 commonly used packages
  MAINTAINER:=Twinzo
  SECTION:=Twinzo devices
  CATEGORY:=Twinzo devices
  DEPENDS:=+ipv6helper +luci-app-CE +luci-ssl-openssl \
		+luci-app-devcom +luci-app-timecontrol \
		+luci-app-idns \
		+luci-app-sccustom +luci-app-serverchan \
		+luci-app-uhttpd +luci-app-mac +luci-app-wrtbwmon
endef

define Package/psg1208/description
add package for psg1208.
endef

define Build/Compile
endef

define Package/psg1208/install
endef

$(eval $(call BuildPackage,psg1208))
