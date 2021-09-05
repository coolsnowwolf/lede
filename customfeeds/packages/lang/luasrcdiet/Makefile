include $(TOPDIR)/rules.mk

PKG_NAME:=luasrcdiet
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Matthias Schiffer <mschiffer@universe-factory.net>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYRIGHT

PKG_SOURCE_URL:=https://github.com/jirutka/luasrcdiet.git
PKG_SOURCE_VERSION:=f138fc9359821d9201cd6b57cfa2fcbed5b9af97
PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=d4e9c396312ef3d91d86ea192569f03875459432994bc822dbdbbd0a34e306c5

HOST_BUILD_DEPENDS:=lua/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/luasrcdiet
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=luasrcdiet
  URL:=https://github.com/jirutka/luasrcdiet
  DEPENDS:=+lua
  PKGARCH:=all
endef

define Package/luasrcdiet/description
  Compresses Lua source code by removing unnecessary characters
endef

define Build/Compile
endef

define Package/luasrcdiet/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/luasrcdiet $(1)/usr/bin/luasrcdiet

	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/luasrcdiet $(1)/usr/lib/lua/
endef

define Host/Compile
endef

define Host/Install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/luasrcdiet $(1)/bin/

	$(INSTALL_DIR) $(1)/lib/lua/5.1
	$(CP) $(HOST_BUILD_DIR)/luasrcdiet $(1)/lib/lua/5.1/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,luasrcdiet))
