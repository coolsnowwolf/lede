# Copyright (C) 2016 Openwrt.org
#
# This is a free software, use it under GNU General Public License v3.0.
#
# Created By [CTCGFW]Project-OpenWrt
# https://github.com/project-openwrt

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-unblockneteasemusic-mini
PKG_VERSION:=1.2
PKG_RELEASE:=4
LUCI_TITLE:=LuCI support for UnblockNeteaseMusic(-Go)
LUCI_DEPENDS:=+busybox +dnsmasq-full +ipset +iptables +wget
LUCI_PKGARCH:=all

PKG_MAINTAINER:=[CTCGFW]Project-OpenWrt

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
