#
# Copyright (C) 2016 Tobias Ilte <tobias.ilte@campus.tu-berlin.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dynapoint
PKG_RELEASE:=3

PKG_MAINTAINER:=Tobias Ilte <tobias.ilte@campus.tu-berlin.de>
PKG_LICENSE:=GPL-3.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/dynapoint
    SECTION:=net
    CATEGORY:=Network
    SUBMENU:=Wireless
    DEPENDS:=+lua +libubus-lua +libuci-lua +libubox-lua +luci-lib-nixio
    TITLE:=Dynamic access point manager
    PKGARCH:=all
endef

define Package/dynapoint/description
  Dynapoint uses LUA scripts to allow dynamic access point creation
  and deletion depending on changes of certain network conditions.
endef

define Package/dynapoint/conffiles
/etc/config/dynapoint
endef

define Build/Compile
endef

define Package/dynapoint/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./src/dynapoint.lua $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./src/dynapoint.init $(1)/etc/init.d/dynapoint
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./src/dynapoint.config $(1)/etc/config/dynapoint
endef

$(eval $(call BuildPackage,dynapoint))
