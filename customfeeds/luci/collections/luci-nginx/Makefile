#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TYPE:=col
LUCI_BASENAME:=nginx

LUCI_TITLE:=LuCI interface with Nginx as Webserver
LUCI_DESCRIPTION:=Standard OpenWrt set including full admin with ppp support and the default Bootstrap theme
LUCI_DEPENDS:= \
	+nginx +nginx-mod-luci +luci-mod-admin-full +luci-theme-bootstrap \
	+luci-app-firewall +luci-proto-ppp +libiwinfo-lua +IPV6:luci-proto-ipv6 \
	+rpcd-mod-rrdns

PKG_LICENSE:=Apache-2.0

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
