#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gotop
PKG_VERSION:=4.1.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xxxserxxx/gotop/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=81518fecfdab4f4c25a4713e24d9c033ba8311bbd3e2c0435ba76349028356da

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@immortalwrt.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_GOTOP_COMPRESS_GOPROXY \
	CONFIG_GOTOP_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/xxxserxxx/gotop
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:=main.Version=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/gotop
	SECTION:=admin
	CATEGORY:=Administration
	TITLE:=A terminal based graphical activity monitor
	URL:=https://github.com/xxxserxxx/gotop
	DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/gotop/description
	Another terminal based graphical activity monitor,
	inspired by gtop and vtop, this time written in Go!
endef

define Package/gotop/config
config GOTOP_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config GOTOP_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_GOTOP_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.baidu.com
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_GOTOP_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/gotop
endif
endef

$(eval $(call GoBinPackage,gotop))
$(eval $(call BuildPackage,gotop))
