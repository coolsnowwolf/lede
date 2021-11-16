# Copyright (C) 2018-2019 Lienol
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for SoftEtherVPN
LUCI_DEPENDS:=+zlib +libpthread +librt +libreadline +libncurses +libiconv-full +kmod-tun +libopenssl +softethervpn5-bridge +softethervpn5-client +softethervpn5-server
LUCI_PKGARCH:=all
PKG_VERSION:=1.0
PKG_RELEASE:=4

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
