include $(TOPDIR)/rules.mk

PKG_NAME:=luv
PKG_VERSION:=1.40.0-0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/luvit/luv/releases/download/$(PKG_VERSION)
PKG_HASH:=24473a081c3928eec2a352369cbafda97059574f4a4276861274473e7c7d17a0

PKG_MAINTAINER:=Morteza Milani <milani@pichak.co>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/luv
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Luv
  URL:=https://github.com/luvit/luv
  DEPENDS:=+libuv +!LUV_USE_LUAJIT_ENGINE:lua
endef

define Package/luv/description
  Bare libuv bindings for lua
endef

define Package/luv/config
	source "$(SOURCE)/Config.in"
endef

CMAKE_OPTIONS += \
	-DLUA_BUILD_TYPE=System \
	-DWITH_SHARED_LIBUV=ON \
	-DBUILD_MODULE=OFF \
	-DBUILD_SHARED_LIBS=ON \
	-DWITH_LUA_ENGINE=$(if $(CONFIG_LUV_USE_LUAJIT_ENGINE),Luajit,Lua)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/luv
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/luv/*.h $(1)/usr/include/luv
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libluv.so* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libluv.pc $(1)/usr/lib/pkgconfig
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libluv.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libluv.pc
endef

define Package/luv/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libluv.so.1.40.0 $(1)/usr/lib/lua/luv.so
endef


$(eval $(call BuildPackage,luv))
