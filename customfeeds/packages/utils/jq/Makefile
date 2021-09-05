#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=jq
PKG_VERSION:=1.6
PKG_RELEASE:=1
PKG_LICENSE:=BSD

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/stedolan/jq/releases/download/jq-$(PKG_VERSION)/
PKG_HASH:=9625784cf2e4fd9842f1d407681ce4878b5b0dcddbcd31c6135114a30c71e6a8

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

ifdef CONFIG_USE_MIPS16
  TARGET_CFLAGS += -fno-ipa-sra
endif

TARGET_CFLAGS += -std=c99 -D_GNU_SOURCE

CONFIGURE_ARGS+= \
	--disable-docs \
	--disable-valgrind \
	--without-oniguruma

define Package/jq
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Lightweight and flexible command-line JSON processor.
  URL:=https://stedolan.github.io/jq/
  MAINTAINER:=Marko Ratkaj <marko.ratkaj@sartura.hr>
endef

define Package/jq/description
	Lightweight and flexible command-line JSON processor.
endef

define Package/jq/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,jq))
