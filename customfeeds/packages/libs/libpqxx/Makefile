include $(TOPDIR)/rules.mk

PKG_NAME:=libpqxx
PKG_VERSION:=7.5.2
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/jtv/libpqxx
PKG_SOURCE_VERSION:=$(PKG_VERSION)
PKG_MIRROR_HASH:=9f7fc055f16289e871f8ff59b7fe085f5eab34264503671f357ef71d5f1bbc02

CMAKE_INSTALL:=1

PKG_MAINTAINER:=Igor Bezzubchenko <garikello@gmail.com>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libpqxx
	SECTION:=libs
	CATEGORY:=Libraries
	DEPENDS:=+libpq +libstdcpp
	TITLE:=PostgreSQL client library (C++ interface)
	URL:=http://pqxx.org/development/libpqxx
	SUBMENU:=Database
endef

define Package/libpqxx/config
	source "$(SOURCE)/Config.in"
endef

CMAKE_OPTIONS += \
	-DBUILD_DOC=OFF \
	-DBUILD_STATIC_LIBS=O$(if $(CONFIG_LIBPQXX_STATIC),N,FF) \
	-DBUILD_SHARED_LIBS=O$(if $(CONFIG_LIBPQXX_SHARED),N,FF) \
	$(if $(CONFIG_LIBPQXX_INSTALL_TEST),\
		-DINSTALL_TEST=ON  -DSKIP_BUILD_TEST=OFF, \
		-DINSTALL_TEST=OFF -DSKIP_BUILD_TEST=ON \
	)

define Package/libpqxx/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(if $(CONFIG_LIBPQXX_SHARED), \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpqxx*.so $(1)/usr/lib/)
	$(if $(CONFIG_LIBPQXX_INSTALL_TEST), \
		$(CP) $(PKG_INSTALL_DIR)/usr/bin/libpqxx* $(1)/usr/bin/)
endef

$(eval $(call BuildPackage,libpqxx))
