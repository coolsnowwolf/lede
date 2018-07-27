#
# Copyright 2016-2017 Xingwang Liao <kuoruan@gmail.com>
# Licensed to the public under the Apache License 2.0.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-kcptun
PKG_VERSION:=1.4.4
PKG_RELEASE:=1

PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>

LUCI_TITLE:=LuCI support for Kcptun
LUCI_DEPENDS:=+jshn +wget +luci-lib-jsonc
LUCI_PKGARCH:=all

define Package/$(PKG_NAME)/conffiles
/etc/config/kcptun
endef

include $(TOPDIR)/feeds/luci/luci.mk

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/40_luci-kcptun ) && rm -f /etc/uci-defaults/40_luci-kcptun
fi
chmod 755 $${IPKG_INSTROOT}/etc/init.d/kcptun >/dev/null 2>&1
$${IPKG_INSTROOT}/etc/init.d/kcptun enable >/dev/null 2>&1
exit 0
endef

# call BuildPackage - OpenWrt buildroot signature
