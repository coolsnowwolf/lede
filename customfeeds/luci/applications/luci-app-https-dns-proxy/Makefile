# Copyright 2017-2018 Stan Grishin (stangri@melmac.net)
# This is free software, licensed under the GNU General Public License v3.

include $(TOPDIR)/rules.mk

PKG_LICENSE:=GPL-3.0-or-later
PKG_MAINTAINER:=Stan Grishin <stangri@melmac.net>

LUCI_TITLE:=DNS Over HTTPS Proxy Web UI
LUCI_DESCRIPTION:=Provides Web UI for DNS Over HTTPS Proxy
LUCI_DEPENDS:=+luci-compat +luci-mod-admin-full +https-dns-proxy
LUCI_PKGARCH:=all
PKG_RELEASE:=12

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
