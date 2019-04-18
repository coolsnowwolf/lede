#
# Copyright (C) 2019 Xingwang Liao
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kcptun
PKG_VERSION:=20190416
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/xtaci/kcptun/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=0c60589872b87433da5d6dc045db1be3fdb9f3addb6458b0e5056c16dc4f86b0

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.md
PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/xtaci/kcptun

GO_PKG_LDFLAGS:=-s -w -X 'main.VERSION=$(PKG_VERSION)-$(PKG_RELEASE) for OpenWrt'

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/kcptun/Default
  TITLE:=Simple UDP Tunnel Based On KCP
  URL:=https://github.com/xtaci/kcptun
endef

define Package/kcptun/Default/description
A Stable & Secure Tunnel Based On KCP with N:M Multiplexing.
endef

define kcptun/templates
  define Package/kcptun-$(1)
  $$(call Package/kcptun/Default)
    TITLE+= ($(1))
    USERID:=kcptun=12900:kcptun=12900
    SECTION:=net
    CATEGORY:=Network
    SUBMENU:=Web Servers/Proxies
    DEPENDS:=$$(GO_ARCH_DEPENDS)
  endef

  define Package/kcptun-$(1)/description
  $$(call Package/kcptun/Default/description)

  This package contains the kcptun $(1).
  endef

  define Package/kcptun-$(1)/install
	$$(INSTALL_DIR) $$(1)/usr/bin
	$$(INSTALL_BIN) $$(GO_PKG_BUILD_BIN_DIR)/$(1) $$(1)/usr/bin/kcptun-$(1)
  endef
endef

define Package/golang-github-xtaci-kcptun-dev
$(call Package/kcptun/Default)
$(call GoPackage/GoSubMenu)
  TITLE+= (source files)
  PKGARCH:=all
endef

define Package/golang-github-xtaci-kcptun-dev/description
$(call Package/kcptun/Default/description)

This package provides the source files for the kcptun client/server.
endef

KCPTUN_COMPONENTS:=client server

$(foreach component,$(KCPTUN_COMPONENTS), \
  $(eval $(call kcptun/templates,$(component))) \
  $(eval $(call GoBinPackage,kcptun-$(component))) \
  $(eval $(call BuildPackage,kcptun-$(component))) \
)

$(eval $(call GoSrcPackage,golang-github-xtaci-kcptun-dev))
$(eval $(call BuildPackage,golang-github-xtaci-kcptun-dev))
