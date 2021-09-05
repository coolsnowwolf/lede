#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luaposix
PKG_VERSION:=35.0
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://codeload.github.com/$(PKG_NAME)/$(PKG_NAME)/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=a4edf2f715feff65acb009e8d1689e57ec665eb79bc36a6649fae55eafd56809
PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS:=luarocks/host

include $(INCLUDE_DIR)/package.mk

define Package/luaposix
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=POSIX binding for LuaJIT, Lua 5.1, 5.2 and 5.3
  URL:=https://github.com/luaposix/luaposix
  DEPENDS:=+lua +lua-bit32
endef

define Package/luaposix/description
  luaposix is a general POSIX library for Lua providing access
  to various low level libc functions.
endef

TARGET_PATH_PKG:=$(CURDIR)/scripts:$(TARGET_PATH_PKG)

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include

define Build/Compile
  cd $(PKG_BUILD_DIR) && \
	LDFLAGS="$(TARGET_LDFLAGS) $(FPIC)" \
	CFLAGS="$(TARGET_CFLAGS) $(FPIC)" \
    CC="$(TARGET_CC)" LD="$(TARGET_CC)" \
    LUA_PKGNAME=lua5.1 \
    LUA_LIBDIR=$(STAGING_DIR)/usr/lib/lua \
    luarocks make --force --deps-mode=none --pack-binary-rock luaposix-$(PKG_VERSION)-1.rockspec \
    LUA_LIBDIR=$(STAGING_DIR)/usr/lib/lua \
    LUA_PKGNAME=lua5.1 \
    CC="$(TARGET_CC)" LD="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS) $(FPIC)" \
	LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/luaposix/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/posix/sys
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/linux/posix/*.so $(1)/usr/lib/lua/posix/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/linux/posix/sys/*.so $(1)/usr/lib/lua/posix/sys/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/posix/*.lua $(1)/usr/lib/lua/posix/
	rm -f $(1)/usr/lib/lua/posix/{deprecated,compat}.lua
endef

$(eval $(call BuildPackage,luaposix))
