#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=brook
PKG_VERSION:=20210701
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/txthinking/brook/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=92c2253349af05ea5aa7a45cddd39ca638c732b2ffdb5066a5f03d2df40cb0b5

PKG_MAINTAINER:=CN_SZTL <cnsztl@immortalwrt.org>
PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_CONFIG_DEPENDS:= \
	CONFIG_BROOK_COMPRESS_GOPROXY \
	CONFIG_BROOK_COMPRESS_UPX

GO_PKG:=github.com/txthinking/brook
GO_PKG_BUILD_PKG:=github.com/txthinking/brook/cli/brook
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/brook
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	TITLE:=A cross-platform proxy software
	DEPENDS:=$(GO_ARCH_DEPENDS) +ca-bundle
	URL:=https://github.com/txthinking/brook
endef

define Package/brook/description
	Brook is a cross-platform strong encryption and not detectable proxy.
	Zero-Configuration.
endef

define Package/brook/config
	config BROOK_COMPRESS_GOPROXY
		bool "Compiling with GOPROXY proxy"
		default n

	config BROOK_COMPRESS_UPX
		bool "Compress executable files with UPX"
		default y
endef

ifeq ($(CONFIG_BROOK_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.baidu.com
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_BROOK_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/brook
endif
endef

$(eval $(call GoBinPackage,brook))
$(eval $(call BuildPackage,brook))
