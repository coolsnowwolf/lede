# Copyright (C) 2016 Openwrt.org
#
# This is a free software, use it under Apache License 2.0 & GNU General Public License v3.0.
#
# licheng
# www.maxlicheng.com
# 2019-06-08
#
# Second revision by [CTCGFW]Project-OpenWRT
# 2019.07.15 

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for UnblockNeteaseMusic
LUCI_DEPENDS:=+node +bash +dnsmasq-full +ipset
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-unblockmusic
PKG_VERSION:=1.4
PKG_RELEASE:=3

PKG_MAINTAINER:=<https://github.com/project-openwrt/luci-app-unblockmusic>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
