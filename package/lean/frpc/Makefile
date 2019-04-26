#
# Copyright (C) 2019 Xingwang Liao
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=frp
PKG_VERSION:=0.26.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/fatedier/frp/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=b5a52c1f84af57af57d955c130876ad966baa66f48d063b10956e806dde45823

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/fatedier/frp
GO_PKG_BUILD_PKG:=github.com/fatedier/frp/cmd/...

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/frp/Default
  TITLE:=A fast reverse proxy
  URL:=https://github.com/fatedier/frp
endef

define Package/frp/Default/description
frp is a fast reverse proxy to help you expose a local server behind a NAT or firewall
to the internet. As of now, it supports tcp & udp, as well as httpand https protocols,
where requests can be forwarded to internal services by domain name.
endef

define frp/templates
  define Package/$(1)
  $$(call Package/frp/Default)
    TITLE+= ($(1))
    USERID:=frp=7000:frp=7000
    SECTION:=net
    CATEGORY:=Network
    SUBMENU:=Web Servers/Proxies
    DEPENDS:=$$(GO_ARCH_DEPENDS)
  endef

  define Package/$(1)/description
  $$(call Package/frp/Default/description)

  This package contains the $(1).
  endef

  define Package/$(1)/install
	$$(INSTALL_DIR) $$(1)/usr/bin
	$$(INSTALL_BIN) $$(GO_PKG_BUILD_BIN_DIR)/$(1) $$(1)/usr/bin/
  endef
endef

define Package/golang-github-fatedier-frp-dev
$(call Package/frp/Default)
$(call GoPackage/GoSubMenu)
  TITLE+= (source files)
  PKGARCH:=all
endef

define Package/golang-github-fatedier-frp-dev/description
$(call Package/frp/Default/description)

This package provides the source files for frpc/frps.
endef

define Build/Compile
$(call GoPackage/Build/Compile,-ldflags "-s -w")
endef

FRP_COMPONENTS:=frpc frps

$(foreach component,$(FRP_COMPONENTS), \
  $(eval $(call frp/templates,$(component))) \
  $(eval $(call GoBinPackage,$(component))) \
  $(eval $(call BuildPackage,$(component))) \
)

$(eval $(call GoSrcPackage,golang-github-fatedier-frp-dev))
$(eval $(call BuildPackage,golang-github-fatedier-frp-dev))
