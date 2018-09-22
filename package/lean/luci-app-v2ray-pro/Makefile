# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for V2Ray (Please select lua-cjson package manually)
LUCI_DEPENDS:=+iptables-mod-tproxy +kmod-ipt-tproxy +ip +ipset-lists +pdnsd-alt +coreutils +coreutils-base64 +coreutils-nohup +dnsmasq-full +v2ray +ca-certificates
LUCI_PKGARCH:=all
PKG_VERSION:=1.0
PKG_RELEASE:=9

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
