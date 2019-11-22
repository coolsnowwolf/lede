#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dogcom
PKG_VERSION:=1.6.5
PKG_RELEASE:=3

PKG_LICENSE:=AGPL-3.0
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/mchome/dogcom.git
PKG_SOURCE_VERSION:=309db8f545d7454b464a5d5d1d7dc4bde313f07a

PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_BUILD_PARALLEL:=1

#PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dogcom
  SECTION:=net
  CATEGORY:=Network
  TITLE:=dogcom for openwrt.
  URL:=https://github.com/mchome/dogcom
endef

define Package/dogcom/description
	dogcom is drcom-generic implementation in C.
endef

define Package/dogcom/install
	$(INSTALL_DIR)  $(1)/usr/bin
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/dogcom $(1)/usr/bin
	$(INSTALL_DIR)  $(1)/etc/init.d
	$(INSTALL_BIN)  ./files/etc/init.d/dogcom $(1)/etc/init.d
	$(INSTALL_BIN)  ./files/etc/init.d/dogcom_log $(1)/etc/init.d
	$(INSTALL_BIN)  ./files/etc/init.d/dogcom_proxy $(1)/etc/init.d
	
	$(INSTALL_DIR)   $(1)/etc/config
	$(INSTALL_DATA)  ./files/etc/config/dogcom $(1)/etc/config
	$(INSTALL_DATA)  ./files/etc/config/dogcom_proxy $(1)/etc/config
	$(INSTALL_DIR)   $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA)  ./files/usr/lib/lua/luci/controller/dogcom.lua $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR)   $(1)/usr/lib/lua/luci/model/cbi/dogcom
	$(INSTALL_DATA)  ./files/usr/lib/lua/luci/model/cbi/dogcom/dogcom.lua $(1)/usr/lib/lua/luci/model/cbi/dogcom
	$(INSTALL_DATA)  ./files/usr/lib/lua/luci/model/cbi/dogcom/dogcom_proxy.lua $(1)/usr/lib/lua/luci/model/cbi/dogcom
	$(INSTALL_DIR)   $(1)/usr/lib/lua/luci/view/dogcom
	$(INSTALL_DATA)  ./files/usr/lib/lua/luci/view/dogcom/dogcom.htm $(1)/usr/lib/lua/luci/view/dogcom
	$(INSTALL_DATA)  ./files/usr/lib/lua/luci/view/dogcom/dogcom_mac.htm $(1)/usr/lib/lua/luci/view/dogcom

endef

$(eval $(call BuildPackage,dogcom))
