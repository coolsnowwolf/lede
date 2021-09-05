# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=tmate
PKG_VERSION:=2.4.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/tmate-io/tmate/tar.gz/$(PKG_VERSION)?
PKG_HASH:=62b61eb12ab394012c861f6b48ba0bc04ac8765abca13bdde5a4d9105cb16138

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

PKG_FIXUP:=autoreconf
PKG_BUILD_DEPENDS:=ncurses
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tmate
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=SSH
  TITLE:=Instant Terminal Sharing
  URL:=https://tmate.io
  DEPENDS:=+libevent2 +libncurses +libpthread +libssh +msgpack-c +terminfo
endef

define Package/tmate/description
  Tmate is a fork of tmux. It provides an instant pairing solution.
endef

CONFIGURE_ARGS+= \
	--oldincludedir="$(STAGING_DIR)"

CONFIGURE_VARS+= \
	LIBSSH_CFLAGS="-I$(STAGING_DIR)/usr/include" \
	LIBSSH_LIBS="-lssh"

define Package/tmate/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tmate $(1)/usr/bin/tmate
endef

$(eval $(call BuildPackage,tmate))
