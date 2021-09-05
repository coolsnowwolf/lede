#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=haserl
PKG_VERSION:=0.9.36
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/haserl
PKG_HASH:=8f6048bfc77dfe8d1032aec2e708fddfa36d225c25d14b474f70ba7d6eefabc1
PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_CONFIG_DEPENDS:= \
	CONFIG_HASERL_with_lua \
	CONFIG_HASERL_shell_luac \
	CONFIG_HASERL_shell_lua \

include $(INCLUDE_DIR)/package.mk

define Package/haserl
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A CGI wrapper to embed shell scripts in HTML documents
  URL:=http://haserl.sourceforge.net/
  MENU:=1
  DEPENDS:= +HASERL_with_lua:liblua
endef

define Package/haserl/config
	source "$(SOURCE)/Config.in"
endef

ifeq ($(CONFIG_HASERL_with_lua),y)
	CONFIGURE_ARGS+=--with-lua
	TARGET_LDFLAGS += -ldl
endif
ifneq ($(CONFIG_HASERL_shell_lua),y)
	CONFIGURE_ARGS+=--disable-luashell
endif
ifneq ($(CONFIG_HASERL_shell_luac),y)
	CONFIGURE_ARGS+=--disable-luacshell
endif

define Package/haserl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/haserl $(1)/usr/bin/
endef

$(eval $(call BuildPackage,haserl))
