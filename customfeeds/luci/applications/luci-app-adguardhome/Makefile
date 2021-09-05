# Copyright (C) 2018-2019 Lienol
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-adguardhome
PKG_VERSION:=1.8
PKG_RELEASE:=12
PKG_MAINTAINER:=<https://github.com/rufengsuixing/luci-app-adguardhome>

LUCI_TITLE:=LuCI app for AdGuardHome
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+ca-certs +curl +wget-ssl +PACKAGE_$(PKG_NAME)_INCLUDE_binary:adguardhome
LUCI_DESCRIPTION:=LuCI support for AdGuardHome

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_binary
	bool "Include Binary File"
	default y
endef

PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_binary

define Package/luci-app-adguardhome/conffiles
/usr/share/AdGuardHome/links.txt
/etc/config/AdGuardHome
endef

define Package/luci-app-adguardhome/postinst
#!/bin/sh
	/etc/init.d/AdGuardHome enable >/dev/null 2>&1
	enable=$(uci get AdGuardHome.AdGuardHome.enabled 2>/dev/null)
	if [ "$enable" == "1" ]; then
		/etc/init.d/AdGuardHome reload
	fi
	rm -f /tmp/luci-indexcache
	rm -f /tmp/luci-modulecache/*
exit 0
endef

define Package/luci-app-adguardhome/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/AdGuardHome disable
     /etc/init.d/AdGuardHome stop
uci -q batch <<-EOF >/dev/null 2>&1
	delete ucitrack.@AdGuardHome[-1]
	commit ucitrack
EOF
fi
exit 0
endef

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
