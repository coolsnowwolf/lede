#
# Copyright (C) 2017 adbyby-ram
# Copyright (C) 2017 yushi studio <ywb94@qq.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=adbyby-ram
PKG_VERSION:=1.0


PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=yushi studio <ywb94@qq.com>



include $(INCLUDE_DIR)/package.mk

define Package/luci-app-adbyby-ram
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=adbyby-ram LuCI interface
	URL:=https://github.com/ywb94/adbyby-ram
	PKGARCH:=all
endef

define Package/luci-app-adbyby-ram/description
	LuCI Support for adbyby-ram.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-adbyby-ram/prerm
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/adbyby_ram disable
     /etc/init.d/adbyby_ram stop
fi
exit 0
endef






define Package/luci-app-adbyby-ram/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	chmod 755 /etc/init.d/adbyby_ram >/dev/null 2>&1
	/etc/init.d/adbyby_ram enable >/dev/null 2>&1
fi
exit 0
endef

ifeq ($(ARCH),mips)
define Package/luci-app-adbyby-ram/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/adbyby.lua $(1)/usr/lib/lua/luci/controller/adbyby.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/adbyby.*.lmo $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) ./files/luci/model/cbi/*.lua $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/adbyby_ar71xx.sh $(1)/usr/bin/adbyby.sh
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/adbyby.config $(1)/etc/config/adbyby
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/adbyby.init $(1)/etc/init.d/adbyby_ram
endef
endif	
	
ifeq ($(ARCH),mipsel)
define Package/luci-app-adbyby-ram/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/adbyby.lua $(1)/usr/lib/lua/luci/controller/adbyby.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/adbyby.*.lmo $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) ./files/luci/model/cbi/*.lua $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/adbyby_ramips.sh $(1)/usr/bin/adbyby.sh
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/adbyby.config $(1)/etc/config/adbyby
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/adbyby.init $(1)/etc/init.d/adbyby_ram
endef
endif

$(eval $(call BuildPackage,luci-app-adbyby-ram))
