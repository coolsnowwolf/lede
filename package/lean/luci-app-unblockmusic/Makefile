# Copyright (C) 2020 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-unblockmusic
PKG_VERSION:=2.3.1
PKG_RELEASE:=22

PKG_LICENSE:=Apache-2.0

LUCI_TITLE:=LuCI support for Unblock NeteaseCloudMusic
LUCI_DEPENDS:=+UnblockNeteaseMusic +dnsmasq-full +ipset +wget +coreutils +coreutils-nohup
LUCI_PKGARCH:=all

PKG_MAINTAINER:=lean

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

