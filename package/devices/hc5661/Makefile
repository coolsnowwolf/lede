#
# Copyright (C) 2010-2011 OpenWrt.org
# 注意取消accesscontrol
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hc5661
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/hc5661
  TITLE:=hc5661 commonly used packages
  MAINTAINER:=Twinzo
  SECTION:=Twinzo devices
  CATEGORY:=Twinzo devices
  DEPENDS:=+automount +autosamba +ipv6helper +luci-app-CE +luci-ssl-openssl \
		+luci-app-devcom +RNDIS +luci-app-timecontrol \
		+luci-app-idns +luci-app-hd-idle +luci-app-minidlna \
		+luci-app-netdata \
		+luci-app-sccustom +luci-app-serverchan +luci-app-smartdns \
		+luci-app-uhttpd +luci-app-usb-printer +luci-app-mac +luci-app-wrtbwmon
endef

define Package/hc5661/description
add package for hc5661.
endef

define Build/Compile
endef

define Package/hc5661/install
endef

$(eval $(call BuildPackage,hc5661))
