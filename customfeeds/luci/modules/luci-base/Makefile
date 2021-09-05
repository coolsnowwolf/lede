#
# Copyright (C) 2008-2015 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-base

LUCI_TYPE:=mod
LUCI_BASENAME:=base

LUCI_TITLE:=LuCI core libraries
LUCI_DEPENDS:=+lua +libuci-lua +luci-lib-nixio +luci-lib-ip +rpcd +libubus-lua +luci-lib-jsonc

PKG_SOURCE:=LuaSrcDiet-0.12.1.tar.bz2
PKG_SOURCE_URL:=https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/luasrcdiet
PKG_HASH:=ed7680f2896269ae8633756e7edcf09050812f78c8f49e280e63c30d14f35aea
PKG_LICENSE:=Apache-2.0

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/LuaSrcDiet-0.12.1

include $(INCLUDE_DIR)/host-build.mk

define Package/luci-base/conffiles
/etc/luci-uploads
/etc/config/luci
endef

include ../../luci.mk

define Host/Configure
endef

define Host/Compile
	$(MAKE) -C src/ clean po2lmo
	$(MAKE) -C $(HOST_BUILD_DIR) bin/LuaSrcDiet.lua
endef

define Host/Install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) src/po2lmo $(1)/bin/po2lmo
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/LuaSrcDiet.lua $(1)/bin/LuaSrcDiet
endef

$(eval $(call HostBuild))

# call BuildPackage - OpenWrt buildroot signature
