#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 9349 2007-10-18 18:46:37Z blogic $

include $(TOPDIR)/rules.mk

PKG_NAME:=bwmon_gargoyle
PKG_VERSION:=$(GARGOYLE_VERSION)
ifeq ($(GARGOYLE_VERSION),)
     PKG_VERSION:=1.0.0
endif
PKG_RELEASE:=1


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/bwmon-gargoyle
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libericstools +gargoyle-firewall-util
	TITLE:=Bandwidth monitor for Gargoyle Web Interface
	MAINTAINER:=Eric Bishop <eric@gargoyle-router.com>
endef

define Package/bwmon-gargoyle/description
	Bandwidth monitor for Gargoyle Web Interface
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		STAGING_DIR="$(STAGING_DIR)" \
		CFLAGS="$(TARGET_CFLAGS) -I $(STAGING_DIR)/usr/include" \
		LDFLAGS="$(TARGET_LDFLAGS) -L $(STAGING_DIR)/usr/lib" 
endef

define Package/bwmon-gargoyle/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/bwmond.init $(1)/etc/init.d/bwmon_gargoyle
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bw_convert $(1)/usr/bin/bw_convert
endef

$(eval $(call BuildPackage,bwmon-gargoyle))
