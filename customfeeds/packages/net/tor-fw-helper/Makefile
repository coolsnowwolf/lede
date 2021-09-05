#
# Copyright (C) 2018 Jeffery To
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tor-fw-helper
PKG_VERSION:=0.3
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.torproject.org/tor-fw-helper.git
PKG_SOURCE_VERSION:=481599ee37dd3135c9e17d1df4810f36b4de4e3d
PKG_SOURCE_DATE:=20150805
PKG_MIRROR_HASH:=f22d1400bec6b62636bd59cb3a51befc9cddbacccb790a758694c589cb2bc032

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=git.torproject.org/tor-fw-helper.git

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/tor-fw-helper/Default
  TITLE:=Firewall helper for tor
  URL:=https://gitweb.torproject.org/tor-fw-helper.git/
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/tor-fw-helper
$(call Package/tor-fw-helper/Default)
  SECTION:=net
  CATEGORY:=Network
endef

define Package/golang-torproject-tor-fw-helper-dev
$(call Package/tor-fw-helper/Default)
$(call GoPackage/GoSubMenu)
  TITLE+= (source files)
  PKGARCH:=all
endef

define Package/tor-fw-helper/Default/description
tor-fw-helper is a helper to automatically configuring port forwarding
for tor, using UPnP or NAT-PMP NAT traversal.

This is a tor-fw-helper rewrite in Go that functions as a drop in
replacement for the original C code.
endef

define Package/tor-fw-helper/description
$(call Package/tor-fw-helper/Default/description)

This package contains the main helper program.
endef

define Package/golang-torproject-tor-fw-helper-dev/description
$(call Package/tor-fw-helper/Default/description)

This package provides the source files for the helper program.
endef

GO_PKG_BUILD_VARS += GO111MODULE=auto

$(eval $(call GoBinPackage,tor-fw-helper))
$(eval $(call BuildPackage,tor-fw-helper))

$(eval $(call GoSrcPackage,golang-torproject-tor-fw-helper-dev))
$(eval $(call BuildPackage,golang-torproject-tor-fw-helper-dev))
