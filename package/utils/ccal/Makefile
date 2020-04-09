#
# Copyright (C) http://ccal.chinesebay.com/ccal.htm 
# Copyright (C) 2020 Twinzo <1282055288@qq.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ccal
PKG_VERSION=2.5.3
PKG_RELEASE:=1

PKG_SOURCE_DATE:=2020-03-29
PKG_LICENSE:=GPL-2.0

PKG_MAINTAINER:=Twinzo1 <1282055288@qq.com>


include $(INCLUDE_DIR)/package.mk

define Package/ccal/description
	This is a command for Chinese calendar
endef

define Package/ccal
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+libstdcpp
	TITLE:=Chinese calendar in linux
	URL:=http://ccal.chinesebay.com/
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/ccal/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ccal $(1)/sbin/
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(1)/sbin/ccal
endef

$(eval $(call BuildPackage,ccal))
