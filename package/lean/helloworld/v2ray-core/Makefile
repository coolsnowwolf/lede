# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray-core
PKG_VERSION:=4.41.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/v2fly/v2ray-core/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=9d0ca27b78beba96b8ee00141a3e45d7c706d57b3d7c449d7975a8dec815e327

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_V2RAY_CORE_COMPRESS_GOPROXY \
	CONFIG_V2RAY_CORE_COMPRESS_UPX \
	CONFIG_V2RAY_CTL_COMPRESS_GOPROXY \
	CONFIG_V2RAY_CTL_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/v2fly/v2ray-core/v4
GO_PKG_BUILD_PKG:=github.com/v2fly/v2ray-core/v4/main
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	$(GO_PKG).build=OpenWrt \
	$(GO_PKG).version=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/v2ray/template
  TITLE:=A platform for building proxies to bypass network restrictions
  SECTION:=net
  CATEGORY:=Network
  URL:=https://www.v2fly.org
endef

define Package/v2ray-core
  $(call Package/v2ray/template)
  DEPENDS:=$(GO_ARCH_DEPENDS) +ca-bundle
endef

define Package/v2ray-ctl
  $(call Package/v2ray/template)
  DEPENDS:=v2ray-core
endef

define Package/v2ray-extra
  $(call Package/v2ray/template)
  TITLE+= (extra resources)
  DEPENDS:=v2ray-core
  PKGARCH:=all
endef

define Package/v2ray-geodata
  $(call Package/v2ray/template)
  TITLE+= (geodata files)
  DEPENDS:=v2ray-core
  PKGARCH:=all
endef

define Package/v2ray/description
  Project V is a set of network tools that help you to build your own computer network.
  It secures your network connections and thus protects your privacy.
endef

define Package/v2ray-core/description
  $(call Package/v2ray/description)
endef

define Package/v2ray-ctl/description
  $(call Package/v2ray/description)

  This includes configuration tool for v2ray-core.
endef

define Package/v2ray-extra/description
  $(call Package/v2ray/description)

  This includes extra resources for v2ray-core.
endef

define Package/v2ray-geodata/description
  $(call Package/v2ray/description)

  This includes GEO datas used for v2ray-core.
endef

define Package/v2ray-core/config
menu "v2ray-core Configuration"
	depends on PACKAGE_v2ray-core

config V2RAY_CORE_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config V2RAY_CORE_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endmenu
endef

define Package/v2ray-ctl/config
menu "v2ray-ctl Configuration"
	depends on PACKAGE_v2ray-ctl

config V2RAY_CTL_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config V2RAY_CTL_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endmenu
endef

GEOIP_VER:=202106030115
GEOIP_FILE:=geoip.dat.$(GEOIP_VER)

define Download/geoip
  URL:=https://github.com/v2fly/geoip/releases/download/$(GEOIP_VER)/
  URL_FILE:=geoip.dat
  FILE:=$(GEOIP_FILE)
  HASH:=acf231d7c6461d088ae479fe0c1cb143b5ee3cf7048a897c5a9b7807ab7005df
endef

GEOSITE_VER:=20210531212831
GEOSITE_FILE:=dlc.dat.$(GEOSITE_VER)

define Download/geosite
  URL:=https://github.com/v2fly/domain-list-community/releases/download/$(GEOSITE_VER)/
  URL_FILE:=dlc.dat
  FILE:=$(GEOSITE_FILE)
  HASH:=332bbe53af49582dbf89bda04ee5e60e474f9293fc368ab55b517fdcd4a78ed2
endef

ifneq ($(CONFIG_V2RAY_CORE_COMPRESS_GOPROXY)$(CONFIG_V2RAY_CTL_COMPRESS_GOPROXY),)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Build/Prepare
	$(call Build/Prepare/Default)
ifneq ($(CONFIG_PACKAGE_v2ray-geodata),)
	$(call Download,geoip)
	$(call Download,geosite)
endif
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
	mv $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/v2ray
ifneq ($(CONFIG_V2RAY_CORE_COMPRESS_UPX),)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/v2ray
endif

ifneq ($(CONFIG_PACKAGE_v2ray-ctl),)
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG)/infra/control/main)
	$(call GoPackage/Build/Compile)
	mv $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/v2ctl
ifneq ($(CONFIG_V2RAY_CTL_COMPRESS_UPX),)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/v2ctl
endif
endif
endef

define Package/v2ray-core/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/v2ray $(1)/usr/bin/
endef

define Package/v2ray-ctl/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))

	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/v2ctl $(1)/usr/bin/
endef

define Package/v2ray-extra/install
	$(INSTALL_DIR) $(1)/usr/share/v2ray/
	$(CP) $(PKG_BUILD_DIR)/release/extra/* $(1)/usr/share/v2ray/
endef

define Package/v2ray-geodata/install
	$(INSTALL_DIR) $(1)/usr/share/v2ray/
	$(INSTALL_DATA) $(DL_DIR)/$(GEOIP_FILE) $(1)/usr/share/v2ray/geoip.dat
	$(INSTALL_DATA) $(DL_DIR)/$(GEOSITE_FILE) $(1)/usr/share/v2ray/geosite.dat
endef

$(eval $(call BuildPackage,v2ray-core))
$(eval $(call BuildPackage,v2ray-ctl))
$(eval $(call BuildPackage,v2ray-extra))
$(eval $(call BuildPackage,v2ray-geodata))
