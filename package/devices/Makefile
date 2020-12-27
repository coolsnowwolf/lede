#
# Copyright (C) 2010-2011 OpenWrt.org
# 注意取消accesscontrol
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=x86
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_ARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/x86
  TITLE:=x86 commonly used packages
  MAINTAINER:=Twinzo
  SECTION:=Twinzo devices
  CATEGORY:=Twinzo devices
  DEPENDS:=+automount +autosamba +ipv6helper +luci-app-CE +luci-ssl-openssl \
		+luci-app-devcom +RNDIS +luci-app-timecontrol +luci-app-control-timewol \
		+luci-app-idns +luci-app-hd-idle +luci-app-minidlna +luci-app-adguardhome \
		+luci-app-mwan3helper +luci-app-sqm +luci-app-netdata +luci-app-ttyd \
		+luci-app-oafclog +luci-app-sccustom +luci-app-serverchan +luci-app-smartdns \
		+luci-app-uhttpd +luci-app-usb-printer +luci-app-watchpig +luci-app-wrtbwmon \
		+luci-app-xlnetacc +luci-app-zerotier +luci-theme-argon +luci-app-cifs-mount \
		+luci-app-cifsd +youtube-dl +4G +luci-app-jd-dailybonus +luci-app-kodexplorer \
    +luci-app-kcptun +luci-app-music-remote-center +luci-app-nfs +luci-app-openvpn \
    +luci-app-passwall +luci-app-passwall_INCLUDE_simple-obfs +luci-app-privoxy \
    +luci-app-rclone +luci-app-unblockmusic +luci-app-wireguard
		
endef

define Package/x86/description
add package for x86.
endef

define Build/Compile
endef

define Package/x86/install
endef

$(eval $(call BuildPackage,x86))
