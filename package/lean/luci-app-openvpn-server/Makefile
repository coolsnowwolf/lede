# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-openvpn-server
PKG_VERSION:=2.0
PKG_RELEASE:=19

LUCI_TITLE:=LuCI support for OpenVPN Server
LUCI_DEPENDS:=+openvpn-openssl +openvpn-easy-rsa +kmod-tun
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
