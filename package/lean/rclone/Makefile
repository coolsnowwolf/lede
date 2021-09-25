# SPDX-Identifier-License: GPL-3.0-or-later
#
# Copyright (C) 2019 ElonH <elonhhuang@gmail.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone
PKG_VERSION:=1.56.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rclone/rclone/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=15e32033778a010b90aad58cb214bc65fde6e6eff7a7c9a19710549f18799e4c

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_RCLONE_COMPRESS_GOPROXY \
	CONFIG_RCLONE_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/rclone/rclone
GO_PKG_EXCLUDES:=test
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	github.com/rclone/rclone/fs.Version=v$(PKG_VERSION) \
	main.Version=v$(PKG_VERSION) \
	main.BuildUser=openwrt \
	main.BuildHost=openwrt

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/rclone
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
  TITLE:=rsync for cloud storage
  URL:=https://rclone.org
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/rclone/description
  Rclone ("rsync for cloud storage") is a command line program to sync
  files and directories to and from different cloud storage providers.
endef

define Package/rclone/config
config RCLONE_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config RCLONE_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default n
endef

ifeq ($(CONFIG_RCLONE_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.baidu.com
endif

# Disable CGO due to ld linker issue
GO_PKG_TARGET_VARS:=$(filter-out CGO_ENABLED=%,$(GO_PKG_TARGET_VARS)) CGO_ENABLED=0

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_RCLONE_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/rclone
endif
endef

$(eval $(call GoBinPackage,rclone))
$(eval $(call BuildPackage,rclone))
