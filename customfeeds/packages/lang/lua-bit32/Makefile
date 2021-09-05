#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-bit32
PKG_VERSION:=5.3.0
PKG_RELEASE:=1


PKG_SRC_NAME:=lua-compat-5.2
PKG_SRC_VERSION:=0.3
PKG_SOURCE_URL=https://codeload.github.com/keplerproject/$(PKG_SRC_NAME)/tar.gz/v$(PKG_SRC_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=627e842c488c1afece4383da9f9a839a7ddbc2f7fedfc456f76e3a590d4ca67f
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SRC_NAME)-$(PKG_SRC_VERSION)

PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=luarocks/host

include $(INCLUDE_DIR)/package.mk

define Package/lua-bit32
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua 5.2 bit manipulation library
  DEPENDS:=+lua
endef

define Package/lua-bit32/description
  lua-bit32 is the native Lua 5.2 bit manipulation library,
  backported to Lua 5.1
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include

define Build/Compile
  cd $(PKG_BUILD_DIR) && \
  luarocks make --pack-binary-rock bit32-scm-1.rockspec \
    LUA_LIBDIR=$(STAGING_DIR)/usr/lib/lua \
    LUA_PKGNAME=lua5.1 \
	CFLAGS="$(TARGET_CFLAGS) $(FPIC)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
    CC="$(TARGET_CC)" LD="$(TARGET_CC)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bit32.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lua-bit32))
