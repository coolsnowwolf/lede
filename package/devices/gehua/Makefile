#
# Copyright (C) 2010-2011 OpenWrt.org
# 注意取消accesscontrol
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gehua
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/gehua
  TITLE:=gehua commonly used packages
  MAINTAINER:=Twinzo
  SECTION:=Twinzo devices
  CATEGORY:=Twinzo devices
  DEPENDS:=+automount +autosamba +ipv6helper +luci-ssl-openssl \
		+DevicesComment +RNDIS +luci-app-timecontrol +luci-app-control-timewol \
		+luci-app-idns +luci-app-hd-idle +luci-app-minidlna \
		+luci-app-mwan3helper +luci-app-music-remote-center +luci-app-netdata \
		+luci-app-oafclog +luci-app-sccustom +luci-app-serverchan +luci-app-smartdns \
		+luci-app-uhttpd +luci-app-usb-printer +luci-app-watchpig +luci-app-wrtbwmon \
		+luci-app-xlnetacc +luci-app-zerotier
		
endef

define Package/gehua/description
add package for gehua.
endef

define Build/Compile
endef

define Package/gehua/install
endef

$(eval $(call BuildPackage,gehua))
