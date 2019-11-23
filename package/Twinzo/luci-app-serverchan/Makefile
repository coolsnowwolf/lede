# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for serverchan
LUCI_DEPENDS:=+iputils-arping +curl 
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-serverchan
PKG_VERSION:=1.0
PKG_RELEASE:=40

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

