# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
# licheng
# www.maxlicheng.com
# 2019-06-08
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-unblockmusic
PKG_VERSION:=v2.1.0
PKG_RELEASE:=5

PKG_LICENSE:=Apache-2.0

LUCI_TITLE:=LuCI support for Unblock NeteaseCloudMusic
LUCI_DEPENDS:=+UnblockNeteaseMusic +bash +dnsmasq-full +ipset
LUCI_PKGARCH:=all

PKG_MAINTAINER:=<https://github.com/maxlicheng/luci-app-unblockmusic>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

