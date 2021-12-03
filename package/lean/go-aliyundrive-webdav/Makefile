# SPDX-Identifier-License: GPL-3.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=go-aliyundrive-webdav
PKG_VERSION:=1.0.20
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/LinkLeong/go-aliyundrive-webdav/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=ce7108eca5c6d4dacfc32d5c3daf2a3465ad48cfe0c09f08c7eb77045d1c1b3c

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_GO_ALIYUNDRIVE_WEBDAV_COMPRESS_GOPROXY \
	CONFIG_GO_ALIYUNDRIVE_WEBDAV_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=go-aliyun-webdav
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:=main.Version=v$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/go-aliyundrive-webdav
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
  TITLE:=A WebDav server for AliyunDrive
  URL:=https://github.com/LinkLeong/go-aliyundrive-webdav
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/go-aliyundrive-webdav/config
config GO_ALIYUNDRIVE_WEBDAV_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config GO_ALIYUNDRIVE_WEBDAV_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_GO_ALIYUNDRIVE_WEBDAV_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.baidu.com
endif

define Package/go-aliyundrive-webdav/conffiles
/etc/config/go-aliyundrive-webdav
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_GO_ALIYUNDRIVE_WEBDAV_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/go-aliyun-webdav
endif
endef

define Package/go-aliyundrive-webdav/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/go-aliyun-webdav $(1)/usr/bin/go-aliyundrive-webdav

	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_CONF) $(CURDIR)/files/go-aliyundrive-webdav.config $(1)/etc/config/go-aliyundrive-webdav
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) $(CURDIR)/files/go-aliyundrive-webdav.init $(1)/etc/init.d/go-aliyundrive-webdav
endef

$(eval $(call GoBinPackage,go-aliyundrive-webdav))
$(eval $(call BuildPackage,go-aliyundrive-webdav))
