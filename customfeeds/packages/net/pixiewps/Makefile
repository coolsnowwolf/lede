# Copyright (C) 2017-2018 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=pixiewps
PKG_VERSION:=1.4.2
PKG_RELEASE:=2
PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/wiire-a/pixiewps/releases/download/v$(PKG_VERSION)
PKG_HASH:=c4dc0569e476ebdbd85992da2d1ff799db97ed0040da9dc44e13d08a97a9de1e

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE.md

include $(INCLUDE_DIR)/package.mk

define Package/pixiewps
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Wireless
  TITLE:=An offline WPS bruteforce utility
  URL:=https://github.com/wiire-a/pixiewps
  DEPENDS:=+libpthread
endef

define Package/pixiewps/description
  Pixiewps is a tool written in C used to bruteforce offline the WPS PIN
  exploiting the low or non-existing entropy of some Access Points, the so-called
  "pixie-dust attack" discovered by Dominique Bongard in summer 2014.  It is
  meant for educational purposes only.
endef

# override flags by pixiewps' own Makefile
MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
	CPPFLAGS="$(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS)" \

define Package/pixiewps/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pixiewps $(1)/usr/bin/
endef

$(eval $(call BuildPackage,pixiewps))
