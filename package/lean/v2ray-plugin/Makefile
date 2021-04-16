#
# Copyright (C) 2020 SharerMax
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray-plugin
PKG_VERSION:=4.37.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/teddysun/v2ray-plugin/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=e776a2294dba5dbf832dc5ea673ffbb66ae6fac54df90ff286a5d1833a4ae874

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=madeye <max.c.lv@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/shadowsocks/v2ray-plugin
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:=main.VERSION=v$(PKG_VERSION)

PKG_CONFIG_DEPENDS := CONFIG_$(PKG_NAME)_INCLUDE_GOPROXY

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)/config
config $(PKG_NAME)_INCLUDE_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default y

endef

ifeq ($(CONFIG_$(PKG_NAME)_INCLUDE_GOPROXY),y)
export GO111MODULE=on
export GOPROXY=https://goproxy.cn
endif

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Project V
	TITLE:=SIP003 plugin for shadowsocks, based on v2ray
	URL:=https://github.com/teddysun/v2ray-plugin
	DEPENDS:=$(GO_ARCH_DEPENDS) +ca-certificates
endef

define Package/$(PKG_NAME)/description
	Yet another SIP003 plugin for shadowsocks, based on v2ray
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/v2ray-plugin || true
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/v2ray-plugin $(1)/usr/bin/v2ray-plugin
endef

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
