# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
include $(TOPDIR)/rules.mk

LUCI_TITLE:=Custom message for serverchan
LUCI_DEPENDS:=+iputils-arping +curl +ccal
LUCI_PKGARCH:=all
PKG_VERSION:=2.0
PKG_RELEASE:=1

include $(TOPDIR)/feeds/luci/luci.mk
# call BuildPackage - OpenWrt buildroot signature
