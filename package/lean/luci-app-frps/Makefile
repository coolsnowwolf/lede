#
# Copyright 2020 lwz322 <lwz322@qq.com>
# Licensed to the public under the MIT License.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-frps
PKG_VERSION:=0.0.2
PKG_RELEASE:=3

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_MAINTAINER:=lwz322 <lwz322@qq.com>

LUCI_TITLE:=LuCI support for Frps
LUCI_DEPENDS:=+wget +frps
LUCI_PKGARCH:=all

define Package/$(PKG_NAME)/conffiles
/etc/config/frps
endef

include $(TOPDIR)/feeds/luci/luci.mk

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/40_luci-frps ) && rm -f /etc/uci-defaults/40_luci-frps
fi

chmod 755 "$${IPKG_INSTROOT}/etc/init.d/frps" >/dev/null 2>&1
ln -sf "../init.d/frps" \
	"$${IPKG_INSTROOT}/etc/rc.d/S99frps" >/dev/null 2>&1
exit 0
endef

# call BuildPackage - OpenWrt buildroot signature
