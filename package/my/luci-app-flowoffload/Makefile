# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for Flow Offload
LUCI_DEPENDS:=+kmod-ipt-offload +pdnsd-alt +kmod-bbr +iptables-mod-fullconenat @LINUX_4_14
LUCI_PKGARCH:=all
PKG_VERSION:=1.5
PKG_RELEASE:=1

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

