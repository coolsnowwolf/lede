# Copyright (C) 2019 Openwrt.org
#
# This is a free software, use it under Apache Licene 2.0 & GNU General Public License v3.0.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for BaiduPCS-Go
LUCI_DEPENDS:=+baidupcs-web +wget +openssl-util
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-baidupcs-web
PKG_VERSION:=1.0
PKG_RELEASE:=1

# PKG_MAINTAINER:=< /luci-app-baidupcs-web>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
