#
# Copyright 2020 lwz322 <lwz322@qq.com>
# Licensed to the public under the MIT License.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-frps
PKG_VERSION:=0.0.2
PKG_RELEASE:=4

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_MAINTAINER:=lwz322 <lwz322@qq.com>

LUCI_TITLE:=LuCI support for Frps
LUCI_DEPENDS:=+wget +frps
LUCI_PKGARCH:=all

define Package/$(PKG_NAME)/conffiles
/etc/config/frps
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
