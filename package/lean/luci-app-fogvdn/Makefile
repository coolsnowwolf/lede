#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk

LLUCI_TITLE:=Luci for OpenFog Pear P2P CDN
LUCI_PKGARCH:=all
PKG_VERSION:=7
PKG_RELEASE:=1
LUCI_DEPENDS:=+luci-compat +fogvdn +jq +lsblk

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
