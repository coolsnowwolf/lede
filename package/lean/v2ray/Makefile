#
# Copyright (C) 2019 Xingwang Liao
# Copyright (C) 2019 Alonzo Liu <unl89@protonmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray
PKG_VERSION:=4.20.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/v2ray/v2ray-core/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=579a7098b8f6cc68c3f94c5453fa274299f8b072736f364654298bd4cf57d24e

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Alonzo Liu <unl89@protonmail.com>

PKG_CONFIG_DEPENDS := 

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=v2ray.com/core

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/v2ray/Default
  TITLE:=A platform for building proxies
  URL:=https://www.v2ray.com
endef

define Package/v2ray/Default/description
Project V is a set of network tools that help you to build your own computer network.
It secures your network connections and thus protects your privacy.
endef

define project-v/SubMenu
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Project V
endef

define Package/v2ray
  $(call Package/v2ray/Default)
  $(call project-v/SubMenu)
  TITLE+= (full)
  USERID:=v2ray=10800:v2ray=10800
  DEPENDS:=$(GO_ARCH_DEPENDS) +ca-certificates
endef

define Package/v2ray/config
endef

define Package/v2ray/description
$(call Package/v2ray/Default/description)

  This package contains v2ray, v2ctl and v2ray-assets.
endef

define Build/Prepare
	$(Build/Prepare/Default)
	( \
		sed -i \
			's/\(version[[:space:]]*=[[:space:]]*"\).*\("\)/\1$(PKG_VERSION)\2/; \
			s/\(build[[:space:]]*=[[:space:]]*"\).*\("\)/\1OpenWrt - Release $(PKG_RELEASE)\2/' \
			$(PKG_BUILD_DIR)/core.go ; \
	)
	wget -O $(PKG_BUILD_DIR)/release/config/geoip.dat \
		https://github.com/v2ray/geoip/releases/download/20190516.1/geoip.dat

	wget -O $(PKG_BUILD_DIR)/release/config/geosite.dat \
		https://github.com/v2ray/domain-list-community/releases/download/20190829.1/dlc.dat
endef

GO_PKG_LDFLAGS:=-s -w

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=v2ray.com/core/main)
	$(call GoPackage/Build/Compile,)
	mv -f $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/v2ray

	$(eval GO_PKG_BUILD_PKG:=v2ray.com/core/infra/control/main)
	$(call GoPackage/Build/Compile,)
	mv -f $(GO_PKG_BUILD_BIN_DIR)/main $(GO_PKG_BUILD_BIN_DIR)/v2ctl
endef

define Package/v2ray/install
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) \
		$(GO_PKG_BUILD_BIN_DIR)/v2ray \
		$(1)/usr/bin

	$(INSTALL_BIN) \
		$(GO_PKG_BUILD_BIN_DIR)/v2ctl \
		$(1)/usr/bin

	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/release/config/{geoip,geosite}.dat \
		$(1)/usr/bin

endef

define Package/golang-v2ray-dev
$(call Package/v2ray/Default)
$(call GoPackage/GoSubMenu)
  TITLE+= (source files)
  PKGARCH:=all
endef

define Package/golang-v2ray-dev/description
$(call Package/v2ray/Default/description)

This package provides the source files for v2ray-core.
endef

$(eval $(call GoBinPackage,v2ray))
$(eval $(call BuildPackage,v2ray))
$(eval $(call GoSrcPackage,golang-v2ray-dev))
$(eval $(call BuildPackage,golang-v2ray-dev))
