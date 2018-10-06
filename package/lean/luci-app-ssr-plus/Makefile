# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for SSR Plus
LUCI_DEPENDS:=+shadowsocksr-libev +shadowsocks-libev-ss-local +shadowsocks-libev-ss-redir +shadowsocks-libev-ss-tunnel +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +bash +pdnsd-alt +wget
LUCI_PKGARCH:=all
PKG_VERSION:=1
PKG_RELEASE:=11

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature


