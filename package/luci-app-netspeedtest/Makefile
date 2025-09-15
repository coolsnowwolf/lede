
# Copyright (C) 2020-2021 sirpdboy <herboy2008@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
# 

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-netspeedtest

PKG_VERSION:=2.3.1
PKG_RELEASE:=20250302

LUCI_TITLE:=LuCI Support for netspeedtest
LUCI_DEPENDS:=+python3 +iperf3-ssl +homebox
LUCI_PKGARCH:=all
PKG_MAINTAINER:=<https://github.com/sirpdboy/netspeedtest>


define Package/$(PKG_NAME)/conffiles
/etc/config/netspeedtest
endef

define Package/$(PKG_NAME)/postinst
endef


include $(TOPDIR)/feeds/luci/luci.mk


# call BuildPackage - OpenWrt buildroot signature
