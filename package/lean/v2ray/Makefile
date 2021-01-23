#
# Copyright (C) 2019-2020 Xingwang Liao
# Copyright (C) 2019-2021 Mattraks
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray
PKG_VERSION:=4.34.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/v2ray-core-$(PKG_VERSION)

PKG_SOURCE:=v2ray-core-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/v2fly/v2ray-core/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=b250f569cb0369f394f63184e748f1df0c90500feb8a1bf2276257c4c8b81bee

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_V2RAY_EXCLUDE_V2CTL \
	CONFIG_V2RAY_EXCLUDE_ASSETS \
	CONFIG_V2RAY_COMPRESS_GOPROXY \
	CONFIG_V2RAY_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=v2ray.com/core
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	$(GO_PKG).version=$(PKG_VERSION) \
	$(GO_PKG).codename=OpenWrt

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
  TITLE:=A platform for building proxies
  URL:=https://www.v2fly.org
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Project V
  DEPENDS:=$(GO_ARCH_DEPENDS) +ca-certificates
endef

define Package/$(PKG_NAME)/config
menu "V2Ray Configuration"
	depends on PACKAGE_v2ray

config V2RAY_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config V2RAY_EXCLUDE_V2CTL
	bool "Exclude V2Ctl"
	default y

config V2RAY_EXCLUDE_ASSETS
	bool "Exclude geoip.dat & geosite.dat"
	default y

config V2RAY_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endmenu
endef

define Package/$(PKG_NAME)/description
Project V is a set of network tools that help you to build your own computer network.
It secures your network connections and thus protects your privacy.

  This package contains v2ray, v2ctl, geoip.dat and geosite.dat.
endef

ifeq ($(CONFIG_V2RAY_COMPRESS_GOPROXY),y)
export GO111MODULE=on
export GOPROXY=https://goproxy.io
#export GOPROXY=https://mirrors.aliyun.com/goproxy/
endif

GEOIP_VER:=latest
GEOIP_FILE:=geoip-$(GEOIP_VER).dat

define Download/geoip.dat
  URL:=https://github.com/v2fly/geoip/releases/$(GEOIP_VER)/download
  URL_FILE:=geoip.dat
  FILE:=$(GEOIP_FILE)
  HASH:=skip
endef

GEOSITE_VER:=latest
GEOSITE_FILE:=geosite-$(GEOSITE_VER).dat

define Download/geosite.dat
  URL:=https://github.com/v2fly/domain-list-community/releases/$(GEOSITE_VER)/download
  URL_FILE:=dlc.dat
  FILE:=$(GEOSITE_FILE)
  HASH:=skip
endef

define Build/Prepare
	$(call Build/Prepare/Default)
ifneq ($(CONFIG_V2RAY_EXCLUDE_ASSETS),y)
	# move file to make sure download new file every build
	mv -f $(DL_DIR)/$(GEOIP_FILE) $(PKG_BUILD_DIR)/geoip.dat
	mv -f $(DL_DIR)/$(GEOSITE_FILE) $(PKG_BUILD_DIR)/geosite.dat
endif
endef

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG)/main)
	$(call GoPackage/Build/Compile)
	mv -f $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/v2ray
ifeq ($(CONFIG_V2RAY_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/v2ray || true
endif
ifneq ($(CONFIG_V2RAY_EXCLUDE_V2CTL),y)
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG)/infra/control/main)
	$(call GoPackage/Build/Compile)
	mv -f $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/v2ctl
ifeq ($(CONFIG_V2RAY_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/v2ctl || true
endif
endif
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/v2ray $(1)/usr/bin
ifneq ($(CONFIG_V2RAY_EXCLUDE_V2CTL),y)
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/v2ctl $(1)/usr/bin
endif
ifneq ($(CONFIG_V2RAY_EXCLUDE_ASSETS),y)
	$(INSTALL_DIR) $(1)/usr/share/v2ray
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/{geoip,geosite}.dat $(1)/usr/share/v2ray
endif
endef

ifneq ($(CONFIG_V2RAY_EXCLUDE_ASSETS),y)
$(eval $(call Download,geoip.dat))
$(eval $(call Download,geosite.dat))
endif

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
