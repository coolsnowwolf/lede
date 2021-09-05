#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sqlite
PKG_VERSION:=3330000
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-autoconf-$(PKG_VERSION).tar.gz
PKG_HASH:=106a2c48c7f75a298a7557bcc0d5f4f454e5b43811cc738b7ca294d6956bbb15
PKG_SOURCE_URL:=https://www.sqlite.org/2020/

PKG_LICENSE:=PUBLICDOMAIN
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-autoconf-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1

PKG_CPE_ID:=cpe:/a:sqlite:sqlite

PKG_FIXUP:=autoreconf

PKG_INSTALL:=1

PKG_CONFIG_DEPENDS := \
	CONFIG_SQLITE3_BATCH_ATOMIC_WRITE \
	CONFIG_SQLITE3_DYNAMIC_EXTENSIONS \
	CONFIG_SQLITE3_FTS3 \
	CONFIG_SQLITE3_FTS4 \
	CONFIG_SQLITE3_FTS5 \
	CONFIG_SQLITE3_JSON1 \
	CONFIG_SQLITE3_LIBEDIT \
	CONFIG_SQLITE3_READLINE \
	CONFIG_SQLITE3_RTREE \
	CONFIG_SQLITE3_SESSION

include $(INCLUDE_DIR)/package.mk

define Package/sqlite3/Default
  SUBMENU:=Database
  TITLE:=SQLite (v3.x) database engine
  URL:=http://www.sqlite.org/
endef

define Package/sqlite3/Default/description
 SQLite is a small C library that implements a self-contained, embeddable,
 zero-configuration SQL database engine.
endef

define Package/libsqlite3
  $(call Package/sqlite3/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpthread +zlib
  TITLE+= (library)
  ABI_VERSION:=0
endef

define Package/libsqlite3/description
$(call Package/sqlite3/Default/description)
 This package contains the SQLite (v3.x) shared library, used by other
 programs.
endef

define Package/libsqlite3/config
  source "$(SOURCE)/Config-lib.in"
endef

define Package/sqlite3-cli
  $(call Package/sqlite3/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS := \
	  +libsqlite3 \
	  +SQLITE3_LIBEDIT:libedit \
	  +SQLITE3_READLINE:libreadline \
	  +SQLITE3_READLINE:libncursesw
  TITLE+= (cli)
endef

define Package/sqlite3-cli/description
$(call Package/sqlite3/Default/description)
 This package contains a terminal-based front-end to the SQLite (v3.x) library
 that can evaluate queries interactively and display the results in multiple
 formats.
endef

define Package/sqlite3-cli/config
  source "$(SOURCE)/Config-cli.in"
endef

TARGET_CFLAGS += \
	-DHAVE_ISNAN \
	-DHAVE_MALLOC_USABLE_SIZE \
	-DSQLITE_ENABLE_UNLOCK_NOTIFY \
	$(if $(CONFIG_SQLITE3_BATCH_ATOMIC_WRITE),-DSQLITE_ENABLE_BATCH_ATOMIC_WRITE)

CONFIGURE_ARGS += \
	--disable-debug \
	--disable-static-shell \
	--enable-shared \
	--enable-static \
	--enable-threadsafe \
	$(if $(CONFIG_SQLITE3_DYNAMIC_EXTENSIONS),--enable-dynamic-extensions,--disable-dynamic-extensions) \
	$(if $(CONFIG_SQLITE3_FTS3),--enable-fts3,--disable-fts3) \
	$(if $(CONFIG_SQLITE3_FTS3),--enable-fts4,--disable-fts4) \
	$(if $(CONFIG_SQLITE3_FTS3),--enable-fts5,--disable-fts5) \
	$(if $(CONFIG_SQLITE3_JSON1),--enable-json1,--disable-json1) \
	$(if $(CONFIG_SQLITE3_RTREE),--enable-rtree,--disable-rtree) \
	$(if $(CONFIG_SQLITE3_SESSION),--enable-session,--disable-session)

ifeq ($(CONFIG_SQLITE3_LIBEDIT),y)
CONFIGURE_ARGS+=--disable-readline
else ifeq ($(CONFIG_SQLITE3_READLINE),y)
CONFIGURE_ARGS+=--disable-editline
else
CONFIGURE_ARGS+=--disable-editline --disable-readline
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsqlite3.{a,so*} $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/sqlite3{,ext}.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/sqlite3.pc $(1)/usr/lib/pkgconfig
endef

define Package/libsqlite3/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsqlite3.so.$(ABI_VERSION)* $(1)/usr/lib
endef

define Package/sqlite3-cli/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sqlite3 $(1)/usr/bin
endef

$(eval $(call BuildPackage,libsqlite3))
$(eval $(call BuildPackage,sqlite3-cli))
