#
# Copyright (C) 2019 OpenWrt.org
#
# KFERMercer <KFER.Mercer@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=baidupcs-web
PKG_VERSION:=3.7.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=BaiduPCS-Web is a web controller for BaiduPCS-Go
	URL:=https://github.com/Erope/baidupcs-web
endef


define Package/$(PKG_NAME)/description
BaiduPCS-Web is a web controller for BaiduPCS-Go
endef

# 禁止OpenWrt对二进制文件进行strip
STRIP:=true

ifeq ($(ARCH),i386)
	PKG_ARCH_BAIDUPCS-WEB:=86
endif

ifeq ($(ARCH),x86_64)
	PKG_ARCH_BAIDUPCS-WEB:=amd64
endif

ifeq ($(ARCH),mipsel)
	PKG_ARCH_BAIDUPCS-WEB:=mipsle
endif

ifeq ($(ARCH),mips)
	PKG_ARCH_BAIDUPCS-WEB:=mipsle
endif

ifeq ($(ARCH),arm)
	PKG_ARCH_BAIDUPCS-WEB:=arm
endif

ifeq ($(ARCH),aarch64)
	PKG_ARCH_BAIDUPCS-WEB:=arm64
endif


PKG_SOURCE:=BaiduPCS-Go-v$(PKG_VERSION)-linux-$(PKG_ARCH_BAIDUPCS-WEB).zip

PKG_SOURCE_URL:=https://github.com/Erope/baidupcs-web/releases/download/$(PKG_VERSION)/

UNZIP_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/$(PKG_NAME)-unzip

PKG_HASH:=skip

define Build/Prepare
	mkdir -vp $(UNZIP_DIR)
	unzip -od $(UNZIP_DIR) $(DL_DIR)/$(PKG_SOURCE)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(UNZIP_DIR)/*/BaiduPCS-Go $(1)/usr/bin/$(PKG_NAME)
	chmod 755 $(1)/usr/bin/$(PKG_NAME)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
