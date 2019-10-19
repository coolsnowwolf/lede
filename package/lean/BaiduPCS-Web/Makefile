#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#
# By https://github.com/KFERMercer/openwrt-baidupcs-web

include $(TOPDIR)/rules.mk

PKG_NAME:=BaiduPCS-Web
PKG_VERSION:=3.6.8
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=BaiduPCS-Web is a web controller for BaiduPCS-Go
	URL:=https://github.com/liuzhuoling2011/baidupcs-web
endef


define Package/$(PKG_NAME)/description
BaiduPCS-Web is a web controller for BaiduPCS-Go
endef

STRIP:=true

ifeq ($(ARCH),x86_64)
	PKG_ARCH_BAIDUPCS-WEB:=amd64
endif

ifeq ($(ARCH),mipsel)
	PKG_ARCH_BAIDUPCS-WEB:=mipsle
endif

ifeq ($(ARCH),i386)
	PKG_ARCH_BAIDUPCS-WEB:=86
endif

ifeq ($(ARCH),arm)
	PKG_ARCH_BAIDUPCS-WEB:=arm
endif

ifeq ($(ARCH),arm64)
	PKG_ARCH_BAIDUPCS-WEB:=arm64
endif

ifeq ($(ARCH),aarch64)
	PKG_ARCH_BAIDUPCS-WEB:=arm64
endif

BAIDUPCS-WEB_ZIP_NAME:=BaiduPCS-Go-$(PKG_VERSION)-linux-$(PKG_ARCH_BAIDUPCS-WEB)
PKG_SOURCE:=$(BAIDUPCS-WEB_ZIP_NAME).zip
PKG_SOURCE_URL:=https://github.com/liuzhuoling2011/baidupcs-web/releases/download/$(PKG_VERSION)/
UNZIP_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/$(PKG_NAME)-unzip/$(PKG_ARCH_BAIDUPCS-WEB)
PKG_HASH:=skip

define Build/Prepare
	mkdir -vp $(UNZIP_DIR)
	unzip -od $(UNZIP_DIR) $(DL_DIR)/$(BAIDUPCS-WEB_ZIP_NAME).zip
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(UNZIP_DIR)/$(BAIDUPCS-WEB_ZIP_NAME)/BaiduPCS-Go $(1)/usr/bin/$(PKG_NAME)
	chmod 755 $(1)/usr/bin/$(PKG_NAME)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
