# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for Turbo ACC (SFE)
LUCI_DEPENDS:=+kmod-fast-classifier +pdnsd-alt +kmod-tcp-bbr +iptables-mod-fullconenat @LINUX_4_9
LUCI_PKGARCH:=all
PKG_VERSION:=1.5
PKG_RELEASE:=1

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

