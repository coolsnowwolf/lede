#
# Copyright (C) 2017 Dan Luedtke <mail@danrl.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for the MWAN3 multiwan hotplug script
LUCI_DEPENDS:=+mwan3 +libuci-lua +luci-mod-admin-full +luci-app-firewall +luci-lib-nixio
LUCI_PKGARCH:=all
PKG_LICENSE:=GPLv2

PKG_MAINTAINER:=Aedan Renner <chipdankly@gmail.com> \
		Florian Eckert <fe@dev.tdt.de>

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
