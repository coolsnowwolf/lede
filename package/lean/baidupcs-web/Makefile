#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=baidupcs-web
PKG_VERSION:=v3.6.7
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=baidupcs-web is a web controller for BaiduPCS-Go
	URL:=https://github.com/liuzhuoling2011/baidupcs-web
endef


define Package/$(PKG_NAME)/description
baidupcs-web is a web controller for BaiduPCS-Go
endef

BAIDUPCS-WEB_BIN:=BaiduPCS-Go

ifeq ($(ARCH),x86_64)
	PKG_ARCH_BAIDUPCS-WEB:=amd64
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_BAIDUPCS-WEB:=mipsel
endif

ifeq ($(ARCH),i386)
	PKG_ARCH_BAIDUPCS-WEB:=i386
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_BAIDUPCS-WEB:=arm
endif

ifeq ($(ARCH),arm64)
	PKG_ARCH_BAIDUPCS-WEB:=arm64
endif

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/$(PKG_ARCH_BAIDUPCS-WEB)/$(BAIDUPCS-WEB_BIN) $(1)/usr/bin/baidupcs-web
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
