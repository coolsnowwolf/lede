#
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luasql
PKG_VERSION:=2.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/keplerproject/luasql/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=db2458a8c8c5f3bc717e4030fe2878f1ad8d71e437ec6149c381eebad5d525c5

PKG_MAINTAINER:=
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=docs/us/license.html

include $(INCLUDE_DIR)/package.mk

define Package/luasql/Default
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua SQL binding
  URL:=https://keplerproject.github.io/luasql/
  DEPENDS:= +lua
endef

define Package/luasql/Default/description
 LuaSQL is a simple interface from Lua to a DBMS.
endef


define Package/luasql-mysql
$(call Package/luasql/Default)
  TITLE+= for MySQL
  DEPENDS+= +libmysqlclient
  VARIANT:=mysql
endef

define Package/luasql-mysql/description
$(call Package/luasql/Default/description)
 .
 This package contains the MySQL binding.
endef


define Package/luasql-pgsql
$(call Package/luasql/Default)
  TITLE+= for PostgreSQL
  DEPENDS+= +libpq
  VARIANT:=postgres
endef

define Package/luasql-pgsql/description
$(call Package/luasql/Default/description)
 .
 This package contains the PostgreSQL binding.
endef


define Package/luasql-sqlite3
$(call Package/luasql/Default)
  TITLE+= for SQLite 3
  DEPENDS+= +libsqlite3
  VARIANT:=sqlite3
endef

define Package/luasql-sqlite3/description
$(call Package/luasql/Default/description)
 .
 This package contains the SQLite 3 binding.
endef


TARGET_CFLAGS += $(FPIC) -std=gnu99
TARGET_CPPFLAGS += -DLUA_USE_LINUX

ifeq ($(BUILD_VARIANT),mysql)
  MAKE_FLAGS += DRIVER_INCS_mysql='-I$(STAGING_DIR)/usr/include/mysql' \
		DRIVER_LIBS_mysql='$(TARGET_LDFLAGS) -L$(STAGING_DIR)/usr/lib/mysql -lmysqlclient -lz'
endif

ifeq ($(BUILD_VARIANT),postgres)
  MAKE_FLAGS += DRIVER_LIBS_postgres='$(TARGET_LDFLAGS) -lpq'
endif

ifeq ($(BUILD_VARIANT),sqlite3)
  MAKE_FLAGS += DRIVER_LIBS_sqlite='$(TARGET_LDFLAGS) -lsqlite3 -lpthread'
endif

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	$(BUILD_VARIANT)

define Package/Install/Default
	$(INSTALL_DIR) $(1)/usr/lib/lua/luasql
	$(CP) $(PKG_BUILD_DIR)/src/*.so $(1)/usr/lib/lua/luasql/
endef

Package/luasql-mysql/install = $(Package/Install/Default)
Package/luasql-pgsql/install = $(Package/Install/Default)
Package/luasql-sqlite3/install = $(Package/Install/Default)

$(eval $(call BuildPackage,luasql-mysql))
$(eval $(call BuildPackage,luasql-pgsql))
$(eval $(call BuildPackage,luasql-sqlite3))
