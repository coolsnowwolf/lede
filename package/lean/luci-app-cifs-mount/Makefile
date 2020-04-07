
# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI for SMB/CIFS Mount
LUCI_DEPENDS:=+kmod-fs-cifs +kmod-nls-utf8
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-cifs-mount
PKG_VERSION:=1
PKG_RELEASE:=6

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
