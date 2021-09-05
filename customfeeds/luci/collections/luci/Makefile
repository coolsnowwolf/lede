#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TYPE:=col
LUCI_BASENAME:=luci

LUCI_TITLE:=Standard OpenWrt set including full admin with ppp support and the default Bootstrap theme
LUCI_DEPENDS:= \
	+uhttpd +uhttpd-mod-ubus +luci-mod-admin-full +luci-theme-bootstrap \
	+luci-app-firewall +luci-proto-ppp +libiwinfo-lua \
	+rpcd-mod-rrdns

PKG_LICENSE:=Apache-2.0

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
