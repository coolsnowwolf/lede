#
# Copyright (C) 2018 Jeffery To
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=obfs4proxy
PKG_VERSION:=0.0.11
PKG_RELEASE:=1

PKG_SOURCE:=obfs4-$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://gitlab.com/yawning/obfs4/-/archive/$(PKG_NAME)-$(PKG_VERSION)/
PKG_HASH:=b0964479300a3ea899a4354fdeb5196e52544554f8edf6190142cddd1072526c
PKG_BUILD_DIR:=$(BUILD_DIR)/obfs4-$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=gitlab.com/yawning/obfs4.git

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/obfs4proxy/Default
  TITLE:=A Tor pluggable transport proxy, implementing obfs4
  URL:=https://gitlab.com/yawning/obfs4
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/obfs4proxy
$(call Package/obfs4proxy/Default)
  SECTION:=net
  CATEGORY:=Network
endef

define Package/golang-gitlab-yawning-obfs4-dev
$(call Package/obfs4proxy/Default)
$(call GoPackage/GoSubMenu)
  TITLE+= (source files)
  PKGARCH:=all
endef

define Package/obfs4proxy/Default/description
obfs4proxy is a tool that attempts to circumvent censorship by
transforming the Tor traffic between the client and the bridge. This way
censors, who usually monitor traffic between the client and the bridge,
will see innocent-looking transformed traffic instead of the actual Tor
traffic.

obfs4proxy implements the obfsucation protocols obfs2, obfs3, and obfs4.

It is written in Go and is compliant with the Tor pluggable transports
specification, and its modular architecture allows it to support
multiple pluggable transports.
endef

define Package/obfs4proxy/description
$(call Package/obfs4proxy/Default/description)

This package contains both the client and the bridge in a single
program.
endef

define Package/golang-gitlab-yawning-obfs4-dev/description
$(call Package/obfs4proxy/Default/description)

This package provides the source files for the client/bridge program.
endef

$(eval $(call GoBinPackage,obfs4proxy))
$(eval $(call BuildPackage,obfs4proxy))
$(eval $(call GoSrcPackage,golang-gitlab-yawning-obfs4-dev))
$(eval $(call BuildPackage,golang-gitlab-yawning-obfs4-dev))
