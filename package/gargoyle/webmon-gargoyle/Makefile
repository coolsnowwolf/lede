#
# Copyright (C) 2006 OpenWrt.org
# Copyright (C) 2008 Eric Bishop <eric@gargoyle-router.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=webmon_gargoyle
PKG_VERSION:=$(GARGOYLE_VERSION)
ifeq ($(GARGOYLE_VERSION),)
     PKG_VERSION:=1.0.0
endif
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/webmon-gargoyle

include $(INCLUDE_DIR)/package.mk

define Package/webmon-gargoyle
	SUBMENU:=Web
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Init scripts to setup and use iptables webmon module
	URL:=http://www.gargoyle-router.com
	DEPENDS:= +iptables-mod-webmon
	MAINTAINER:=Eric Bishop <eric@gargoyle-router.com>
endef


define Package/webmon-gargoyle/description
  Init scripts to setup and use iptables webmon module
endef


define Build/Prepare
endef


define Build/Configure
endef

define Build/Compile
endef

define Package/webmon-gargoyle/install

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/webmon_gargoyle.conf $(1)/etc/config/webmon_gargoyle
	
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/webmon_gargoyle.init $(1)/etc/init.d/webmon_gargoyle
	
endef


$(eval $(call BuildPackage,webmon-gargoyle))
