# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for IPSec VPN Server (IKEv1 with PSK and Xauth)
LUCI_DEPENDS:=+strongswan-minimal +strongswan-mod-xauth-generic
LUCI_PKGARCH:=all
PKG_VERSION:=1.0
PKG_RELEASE:=7

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature


