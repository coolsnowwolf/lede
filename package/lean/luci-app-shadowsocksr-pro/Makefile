# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for Shadowsocksr
LUCI_DEPENDS:=+iptables-mod-tproxy +kmod-ipt-tproxy +ip +ipset-lists +shadowsocksr-libev-alt +pdnsd-alt +coreutils +coreutils-base64 +coreutils-nohup +dnsmasq-full
LUCI_PKGARCH:=all
PKG_VERSION:=2.0
PKG_RELEASE:=21

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature


