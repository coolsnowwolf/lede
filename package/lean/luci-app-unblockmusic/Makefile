# Copyright (C) 2020 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-unblockmusic
PKG_VERSION:=2.3.1
PKG_RELEASE:=8

PKG_LICENSE:=Apache-2.0

LUCI_TITLE:=LuCI support for Unblock NeteaseCloudMusic
LUCI_DEPENDS:=+UnblockNeteaseMusic +dnsmasq-full +ipset +wget
LUCI_PKGARCH:=all

PKG_MAINTAINER:=lean

include $(TOPDIR)/feeds/luci/luci.mk

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/unblockmusic ) && rm -f /etc/uci-defaults/unblockmusic
	rm -f /tmp/luci-indexcache
	/bin/sh /etc/ppp/ip-up.d/unblockmusic.sh &
	/etc/init.d/unblockmusic disable >/dev/null 2>&1
fi
exit 0
endef

# call BuildPackage - OpenWrt buildroot signature

