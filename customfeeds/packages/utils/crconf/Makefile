#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=crconf
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.code.sf.net/p/crconf/code
PKG_SOURCE_DATE:=2018-03-02
PKG_SOURCE_VERSION:=8bd996400d087028ba56b724abc1f5b378eaa77f
PKG_MIRROR_HASH:=454307cb40a8743b53933cbdd4d9367996ffcf1bd2946413d2862cf050df3bca

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=GPL-2.0-only

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/crconf
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Linux crypto layer configuraton tool
  URL:=https://sourceforge.net/projects/crconf
endef

define Package/crconf/description
  Linux crypto layer configuraton tool.
  Use this to manipulate kernel Crypto API/framework stuff,
  like drivers, templates, etc.
  Example: https://wiki.strongswan.org/projects/strongswan/wiki/Pcrypt
  Most interesting stuff you can find on https://wiki.strongswan.org
endef

MAKE_FLAGS += CFLAGS="$(TARGET_CFLAGS) -I../include"

define Package/crconf/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/crconf $(1)/sbin
endef

$(eval $(call BuildPackage,crconf))
