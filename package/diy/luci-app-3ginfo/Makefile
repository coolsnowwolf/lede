# Copyright 2020-2021 Rafał Wabik - IceG - From eko.one.pl forum
# Licensed to the public under the Apache License 2.0.

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-3ginfo
LUCI_TITLE:=LuCI panel for 3ginfo
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+3ginfo-text +luci-compat
PKG_VERSION:=1.3.3
PKG_RELEASE:=1

include $(TOPDIR)/feeds/luci/luci.mk

define Package/luci-app-3ginfo/postinst
#!/bin/sh
rm -rf /tmp/luci-indexcache
rm -rf /tmp/luci-modulecache/
exit 0
endef

# call BuildPackage - OpenWrt buildroot signature
