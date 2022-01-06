# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for nfs
LUCI_DEPENDS:=+nfs-utils +nfs-kernel-server +nfs-kernel-server-utils +mount-utils
LUCI_PKGARCH:=all
PKG_VERSION:=1.1
PKG_RELEASE:=1

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
