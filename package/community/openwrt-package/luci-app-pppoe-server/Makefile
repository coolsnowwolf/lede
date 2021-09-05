# Copyright (C) 2018-2020 Lienol <lawlienol@gmail.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for PPPoE Server
LUCI_DEPENDS:=+rp-pppoe-common +rp-pppoe-server
LUCI_PKGARCH:=all
PKG_VERSION:=1
PKG_RELEASE:=8-20200326

define Package/luci-app-pppoe-server/preinst
#!/bin/sh
rm -rf $${IPKG_INSTROOT}/etc/config/pppoe-server >/dev/null 2>&1
rm -rf $${IPKG_INSTROOT}/etc/init.d/pppoe-server >/dev/null 2>&1
exit 0
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature