#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-frpc
LUCI_TITLE:=LuCI for FRPC
LUCI_DEPENDS:=+wget +frpc
LUCI_PKGARCH:=all
PKG_VERSION:=1.2
PKG_RELEASE:=1

define Package/$(PKG_NAME)/conffiles
/etc/config/frp
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature


