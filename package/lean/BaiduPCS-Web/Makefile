#
# Copyright (C) 2014-2018 OpenWrt-dist
# Copyright (C) 2019 [CTCGFW] Project OpenWRT
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=BaiduPCS-Web
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_MAINTAINER:=[CTCGFW] Project OpenWRT

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=LuCI Support for BaiduPCS-Web.
	URL:=https://github.com/liuzhuoling2011/baidupcs-web
endef

define Package/$(PKG_NAME)/description
LuCI Support for BaiduPCS-Web.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
ifeq ($(ARCH),i386)
	$(INSTALL_BIN) ./files/i386 $(1)/usr/bin/baidupcs-web
endif
ifeq ($(ARCH),x86_64)
	$(INSTALL_BIN) ./files/x86_64 $(1)/usr/bin/baidupcs-web
endif
ifeq ($(ARCH),arm)
	$(INSTALL_BIN) ./files/arm $(1)/usr/bin/baidupcs-web
endif
ifeq ($(ARCH),arm64)
	$(INSTALL_BIN) ./files/arm64 $(1)/usr/bin/baidupcs-web
endif
ifeq ($(ARCH),mipsel)
	$(INSTALL_BIN) ./files/mipsel $(1)/usr/bin/baidupcs-web
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
