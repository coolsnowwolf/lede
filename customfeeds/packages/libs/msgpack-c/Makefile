# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=msgpack-c
PKG_VERSION:=3.3.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/msgpack/msgpack-c/tar.gz/cpp-$(PKG_VERSION)?
PKG_HASH:=754c3ace499a63e45b77ef4bcab4ee602c2c414f58403bce826b76ffc2f77d0b
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-cpp-$(PKG_VERSION)

PKG_LICENSE:=BSL-1.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= -DMSGPACK_BUILD_EXAMPLES:BOOL=OFF

define Package/msgpack-c
  SECTION:=lib
  CATEGORY:=Libraries
  TITLE:=MessagePack implementation for C and C++
  URL:=https://github.com/msgpack/msgpack-c
endef

define Package/msgpack-c/description
  MessagePack is an efficient binary serialization format, which lets
  you exchange data among multiple languages like JSON, except that
  it's faster and smaller. Small integers are encoded into a single
  byte and short strings require only one extra byte in addition to
  the strings themselves.
endef

define Package/msgpack-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmsgpackc.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,msgpack-c))
