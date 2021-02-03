#
# Copyright (C) 2010-2011 OpenWrt.org
# 注意取消accesscontrol
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=miwifi_mini
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/miwifi_mini
  TITLE:=miwifi_mini commonly used packages
  MAINTAINER:=Twinzo
  SECTION:=Twinzo devices
  CATEGORY:=Twinzo devices
  DEPENDS:=+automount +autosamba +ipv6helper +luci-app-CE \
		+luci-app-devcom +RNDIS +luci-app-timecontrol +luci-app-control-timewol \
		+luci-app-idns +luci-app-hd-idle +luci-app-minidlna \
		+luci-app-netdata +luci-app-sccustom +luci-app-serverchan +luci-app-smartdns \
		+luci-app-uhttpd +luci-app-usb-printer +luci-app-watchpig +luci-app-wrtbwmon
		
endef

define Package/miwifi_mini/description
add package for miwifi_mini.
endef

define Build/Compile
endef

define Package/miwifi_mini/install
endef

$(eval $(call BuildPackage,miwifi_mini))
