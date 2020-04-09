# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-serverchan
PKG_VERSION:=1.49
PKG_RELEASE:=7

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config

config PACKAGE_$(PKG_NAME)_INCLUDE_CUSTOM
	bool "Include Custom messages"

endef

define Package/$(PKG_NAME)
  	SECTION:=luci
  	CATEGORY:=LuCI
  	SUBMENU:=3. Applications
  	DEPENDS:=+iputils-arping +curl \
	+PACKAGE_$(PKG_NAME)_INCLUDE_CUSTOM:ccal
  	TITLE:=LuCI support for serverchan
  	PKGARCH:=all
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/serverchan
	/etc/config/serverchan_custom
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/usr/bin/serverchan $(1)/etc/config $(1)/usr/lib/lua/luci/controller $(1)/usr/lib/lua/luci/model/cbi/serverchan $(1)/usr/lib/lua/luci/view/serverchan

	$(INSTALL_CONF) ./root/etc/config/serverchan $(1)/etc/config
	$(INSTALL_BIN) ./root/etc/init.d/serverchan $(1)/etc/init.d
	$(INSTALL_BIN) ./root/usr/bin/serverchan/serverchan $(1)/usr/bin/serverchan
	$(INSTALL_DATA)  ./luasrc/controller/serverchan.lua $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA)  ./luasrc/model/cbi/serverchan/serverchan.lua $(1)/usr/lib/lua/luci/model/cbi/serverchan
	$(INSTALL_DATA)  ./luasrc/view/serverchan/serverchan_status.htm $(1)/usr/lib/lua/luci/view/serverchan
	ifneq ($((PKG_NAME)_INCLUDE_CUSTOM),y)
		$(INSTALL_CONF) ./root/etc/config/serverchan_custom $(1)/etc/config
		$(INSTALL_BIN) ./root/etc/init.d/sccustom
		$(INSTALL_BIN) ./root/usr/bin/serverchan/sccustom $(1)/usr/bin/serverchan
		$(INSTALL_DATA)  ./luasrc/model/cbi/serverchan/sccustom.lua $(1)/usr/lib/lua/luci/model/cbi/serverchan
		$(INSTALL_DATA)  ./luasrc/view/serverchan/serverchan_datepicker.htm $(1)/usr/lib/lua/luci/view/serverchan
	endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
