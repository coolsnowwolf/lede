# Copyright 2017-2018 Stan Grishin (stangri@melmac.net)
# This is free software, licensed under the GNU General Public License v3.

include $(TOPDIR)/rules.mk

PKG_LICENSE:=GPL-3.0+
PKG_MAINTAINER:=Stan Grishin <stangri@melmac.net>

LUCI_TITLE:=VPN Policy-Based Routing Service Web UI
LUCI_DESCRIPTION:=Provides Web UI for vpn-policy-routing service.
LUCI_DEPENDS:=+luci-mod-admin-full +vpn-policy-routing
LUCI_PKGARCH:=all
PKG_RELEASE:=32

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
