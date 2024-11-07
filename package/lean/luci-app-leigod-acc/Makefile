#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Luci for Leigod Game Accelerater
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+libpcap +iptables +kmod-ipt-nat +iptables-mod-tproxy +kmod-ipt-tproxy +kmod-tun +kmod-netem +tc-full +kmod-ipt-ipset +ipset +curl +miniupnpd +conntrack +conntrackd +leigod-acc +jq
PKG_VERSION:=1
PKG_RELEASE:=5

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

