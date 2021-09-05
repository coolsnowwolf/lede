# Copyright 2017-2018 Stan Grishin (stangri@melmac.net)
# This is free software, licensed under the GNU General Public License v3.

include $(TOPDIR)/rules.mk

PKG_LICENSE:=GPL-3.0-or-later
PKG_MAINTAINER:=Stan Grishin <stangri@melmac.net>

LUCI_TITLE:=Advanced Linksys Reboot Web UI
LUCI_DESCRIPTION:=Provides Web UI (found under System/Advanced Reboot) to reboot supported Linksys and ZyXEL routers to\
	an alternative partition. Also provides Web UI to shut down (power off) your device. 	Supported dual-partition\
	routers are listed at https://github.com/openwrt/luci/blob/master/applications/luci-app-advanced-reboot/README.md

LUCI_DEPENDS:=+luci-mod-admin-full
LUCI_PKGARCH:=all
PKG_RELEASE:=54

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
