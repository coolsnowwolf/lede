#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xray-plugin
PKG_VERSION:=1.3.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/teddysun/xray-plugin/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=a74adb1ea36819b634fb2b09fc2ac14ec2c57488285c82e6e26f1347de1d6324

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Tianling Shen <cnsztl@project-openwrt.eu.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_XRAY_PLUGIN_PROVIDE_V2RAY_PLUGIN \
	CONFIG_XRAY_PLUGIN_COMPRESS_GOPROXY \
	CONFIG_XRAY_PLUGIN_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/teddysun/xray-plugin
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/xray-plugin/config
config XRAY_PLUGIN_PROVIDE_V2RAY_PLUGIN
	bool "Provide v2ray-plugin binary using xray-plugin"
	default n

config XRAY_PLUGIN_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config XRAY_PLUGIN_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifneq ($(CONFIG_XRAY_PLUGIN_COMPRESS_GOPROXY),)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Package/xray-plugin
	SECTION:=net
	CATEGORY:=Network
	TITLE:=SIP003 plugin for Shadowsocks, based on Xray
	URL:=https://github.com/teddysun/xray-plugin
	DEPENDS:=$(GO_ARCH_DEPENDS) +ca-bundle
endef

define Package/xray-plugin/description
	Yet another SIP003 plugin for Shadowsocks, based on Xray.
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
ifneq ($(CONFIG_XRAY_PLUGIN_COMPRESS_UPX),)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/xray-plugin
endif
endef

define Package/xray-plugin/install
	$(call GoPackage/Package/Install/Bin,$(1))
ifneq ($(CONFIG_XRAY_PLUGIN_PROVIDE_V2RAY_PLUGIN),)
	$(LN) xray-plugin $(1)/usr/bin/v2ray-plugin
endif
endef

$(eval $(call GoBinPackage,xray-plugin))
$(eval $(call BuildPackage,xray-plugin))
