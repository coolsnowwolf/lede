# Copyright (C) 2018-2021 Lienol <lawlienol@gmail.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for IPSec VPN Server
LUCI_DEPENDS:=+strongswan +strongswan-minimal +strongswan-mod-xauth-generic +xl2tpd
LUCI_PKGARCH:=all
PKG_VERSION:=1
PKG_RELEASE:=12-20210628

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
