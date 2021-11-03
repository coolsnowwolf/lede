# SPDX-Identifier-License: GPL-3.0-or-later
#
# Copyright (C) 2019 Elon Huang <elonhhuang@gmail.com>
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone
PKG_VERSION:=1.57.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rclone/rclone/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=294f7a6b0874509997d3a9ffae7c74f0c45b687df0ac7d7742f284ad3814fe55

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=Elon Huang <elonhhuang@gmail.com> \
		Tianling Shen <cnsztl@immortalwrt.org>

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

define Package/rclone/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
  TITLE:=rsync for cloud storage
  URL:=https://rclone.org
endef

define Package/rclone
  $(call Package/rclone/Default)
  DEPENDS:=$(GO_ARCH_DEPENDS) +ca-bundle +fuse-utils
  USERID:=rclone:rclone
endef

define Package/rclone-config
  TITLE+= (Config Scripts)
  DEPENDS:=+rclone
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

define Package/rclone-config/conffiles
/etc/config/rclone
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_RCLONE_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/rclone
endif
endef

define Package/rclone/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rclone $(1)/usr/bin/
endef

define Package/rclone-config/install
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_CONF) $(CURDIR)/files/rclone.config $(1)/etc/config/rclone
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) $(CURDIR)/files/rclone.init $(1)/etc/init.d/rclone
endef

$(eval $(call GoBinPackage,rclone))
$(eval $(call BuildPackage,rclone))
$(eval $(call BuildPackage,rclone-config))
