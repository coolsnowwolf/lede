#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua
PKG_VERSION:=5.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.lua.org/ftp/ \
	http://ftp.gwdg.de/pub/languages/lua/ \
	http://mirrors.dotsrc.org/lua/ \
	http://www.tecgraf.puc-rio.br/lua/ftp/
PKG_HASH:=2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333
PKG_BUILD_PARALLEL:=1

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYRIGHT

HOST_PATCH_DIR := ./patches-host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/lua/Default
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua programming language
  URL:=http://www.lua.org/
  MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
endef

define Package/lua/Default/description
 Lua is a powerful light-weight programming language designed for extending 
 applications. Lua is also frequently used as a general-purpose, stand-alone 
 language. Lua is free software.
endef

define Package/liblua
$(call Package/lua/Default)
  SUBMENU:=
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= (libraries)
endef

define Package/liblua/description
$(call Package/lua/Default/description)
 This package contains the Lua shared libraries, needed by other programs.
endef

define Package/lua
$(call Package/lua/Default)
  DEPENDS:=+liblua
  TITLE+= (interpreter)
endef

define Package/lua/description
$(call Package/lua/Default/description)
 This package contains the Lua language interpreter.
endef

define Package/luac
$(call Package/lua/Default)
  DEPENDS:=+liblua
  TITLE+= (compiler)
endef

define Package/luac/description
$(call Package/lua/Default/description)
 This package contains the Lua language compiler.
endef

define Package/lua-examples
$(call Package/lua/Default)
  DEPENDS:=lua
  TITLE+= (examples)
endef

define Package/lua-examples/description
$(call Package/lua/Default/description)
 This package contains Lua language examples.
endef

define Build/Configure
endef

TARGET_CFLAGS += -DLUA_USE_LINUX $(FPIC) -std=gnu99

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		AR="$(TARGET_CROSS)ar rcu" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		INSTALL_ROOT=/usr \
		CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
		MYLDFLAGS="$(TARGET_LDFLAGS)" \
		PKG_VERSION=$(PKG_VERSION) \
		linux
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		INSTALL_TOP="$(PKG_INSTALL_DIR)/usr" \
		install
endef

define Host/Configure
	$(SED) 's,"/usr/local/","$(STAGING_DIR_HOSTPKG)/",' $(HOST_BUILD_DIR)/src/luaconf.h
endef

ifeq ($(HOST_OS),Darwin)
	LUA_OS:=macosx
else
	ifeq ($(HOST_OS),FreeBSD)
		LUA_OS:=freebsd
	else
		LUA_OS:=linux
	endif
endif

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) \
		CC="$(HOSTCC) -std=gnu99" \
		$(LUA_OS)
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) \
		INSTALL_TOP="$(STAGING_DIR_HOSTPKG)" \
		install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lua{,lib,conf}.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lauxlib.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/lnum_config.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblua.{a,so*} $(1)/usr/lib/
	$(LN) liblua.so.$(PKG_VERSION) $(1)/usr/lib/liblualib.so
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/etc/lua.pc $(1)/usr/lib/pkgconfig/
endef

define Package/liblua/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblua.so.* $(1)/usr/lib/
endef

define Package/lua/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lua $(1)/usr/bin/
endef

define Package/luac/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/luac $(1)/usr/bin/
endef

define Package/lua-examples/install
	$(INSTALL_DIR) $(1)/usr/share/lua/examples
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/test/*.lua \
		$(1)/usr/share/lua/examples/
endef

$(eval $(call BuildPackage,liblua))
$(eval $(call BuildPackage,lua))
$(eval $(call BuildPackage,luac))
$(eval $(call BuildPackage,lua-examples))
$(eval $(call HostBuild))

