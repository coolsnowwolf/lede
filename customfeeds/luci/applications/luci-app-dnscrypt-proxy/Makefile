# Copyright 2017 Dirk Brenken (dev@brenken.org)
# This is free software, licensed under the Apache License, Version 2.0
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for DNSCrypt-Proxy
LUCI_DEPENDS:=+uclient-fetch +dnscrypt-proxy +luci-lib-httpprotoutils
LUCI_PKGARCH:=all

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
