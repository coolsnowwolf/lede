# Copyright (C) 2017 Steven Hessing (steven.hessing@gmail.com)
# Based on initial implementation by Stan Grishin (stangri@melmac.net)
# This is free software, licensed under the GNU General Public License v3.

include $(TOPDIR)/rules.mk

PKG_LICENSE:=GPLv3
PKG_MAINTAINER:=Steven Hessing <steven.hessing@gmail.com>

LUCI_TITLE:=Noddos Service Web UI
LUCI_DESCRIPTION:=Provides Web UI for Noddos service.
LUCI_DEPENDS:=+luci +noddos
LUCI_PKGARCH:=all
PKG_RELEASE:=1

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
