#
# Copyright (C) 2019 OpenWrt.org
#
# KFERMercer <KFER.Mercer@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=baidupcs-web
PKG_VERSION:=3.7.4-nnew
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Erope/BaiduPCS-Go.git
PKG_SOURCE_VERSION:=cffd49d0709f9c7d3dd6ae5c9625f68bfd446166
PKG_MIRROR_HASH:=2cb9c9e27613cf4126234c969f1928e392108ad4f1580b3eac92339c42a2ab0a

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS:= \
	CONFIG_BAIDUPCS_WEB_COMPRESS_GOPROXY \
	CONFIG_BAIDUPCS_WEB_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/Erope/BaiduPCS-Go
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:=main.Version=v$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/baidupcs-web
  TITLE:=BaiduPCS-Web is a web controller for BaiduPCS-Go
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  URL:=https://github.com/Erope/BaiduPCS-Go
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/baidupcs-web/description
BaiduPCS-Web is a web controller for BaiduPCS-Go
endef

define Package/baidupcs-web/config
config BAIDUPCS_WEB_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config BAIDUPCS_WEB_COMPRESS_UPX
	bool "Compress executable files with UPX"
	depends on !mips64
	default n
endef

ifeq ($(CONFIG_BAIDUPCS_WEB_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.baidu.com
endif

define Build/Compile
( \
  GOOS=$$$$(go env GOOS) GOARCH=$$$$(go env GOARCH) go get -v github.com/GeertJohan/go.rice/rice/... ; \
  cd $(PKG_BUILD_DIR)/internal/pcsweb ; \
  "$$$$(go env GOPATH)/bin/rice" embed-go ; \
)
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_BAIDUPCS_WEB_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/BaiduPCS-Go
endif
endef

define Package/baidupcs-web/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/BaiduPCS-Go $(1)/usr/bin/baidupcs-web
endef

$(eval $(call GoBinPackage,baidupcs-web))
$(eval $(call BuildPackage,baidupcs-web))
