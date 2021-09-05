#
# Copyright (C) 2019 Sebastian Kemper <sebastian_ml@gmx.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmariadb
PKG_VERSION:=3.1.12
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=mariadb-connector-c-$(PKG_VERSION)-src.tar.gz
PKG_SOURCE_URL := \
	https://mirror.netcologne.de/mariadb/connector-c-$(PKG_VERSION) \
	https://mirror.lstn.net/mariadb/connector-c-$(PKG_VERSION) \
	https://ftp.yz.yamagata-u.ac.jp/pub/dbms/mariadb/connector-c-$(PKG_VERSION) \
	https://downloads.mariadb.org/interstitial/connector-c-$(PKG_VERSION)
PKG_HASH:=2f5ae14708b4813e4ff6857d152c22e6fc0e551c9fa743c1ef81a68e3254fe63
PKG_BUILD_DIR:=$(BUILD_DIR)/mariadb-connector-c-$(PKG_VERSION)-src

PKG_MAINTAINER:=Michal Hrusecky <Michal@Hrusecky.net>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING.LIB
PKG_CPE_ID:=cpe:/a:mariadb:mariadb

MARIADB_CONF_DIR:=/etc/mysql
MARIADB_PLUGIN_DIR:=/usr/lib/mariadb/plugin

MARIADB_PORT=3306
MARIADB_SOCKET=/var/run/mysqld/mysqld.sock

MARIADB_CLIENT_PLUGINS := \
	auth_gssapi_client \
	remote_io

PKG_BUILD_DEPENDS:=curl
PKG_CONFIG_DEPENDS := \
	$(patsubst %,CONFIG_PACKAGE_$(PKG_NAME)-plugin-%,$(subst _,-,$(MARIADB_CLIENT_PLUGINS)))

plugin-auth_gssapi_client       := CLIENT_PLUGIN_AUTH_GSSAPI_CLIENT
plugin-remote_io                := CLIENT_PLUGIN_REMOTE_IO

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/cmake.mk

# Pass CPPFLAGS in the CFLAGS as otherwise the build system will
# ignore them.
TARGET_CFLAGS+=$(TARGET_CPPFLAGS)

define Package/$(PKG_NAME)/install/plugin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)$(MARIADB_PLUGIN_DIR)/$(2).so $(1)$(MARIADB_PLUGIN_DIR)
endef

define Package/$(PKG_NAME)/Default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://mariadb.org/
  SUBMENU:=Database
endef

define Package/$(PKG_NAME)/description/Default
MariaDB is a very fast and robust SQL database server.
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  DEPENDS:=+mariadb-common \
	  $(ICONV_DEPENDS) \
	  +libopenssl \
	  +zlib
  TITLE:=MariaDB database client library
  MENU:=1
  PROVIDES:=libmariadbclient libmysqlclient libmysqlclient-r
  ABI_VERSION:=3
endef

define Package/$(PKG_NAME)/conffiles
$(MARIADB_CONF_DIR)/conf.d/50-client.cnf
endef

define Package/$(PKG_NAME)/description
$(call Package/$(PKG_NAME)/description/Default)

This package includes the client library.

endef

# We won't need unit tests
CMAKE_OPTIONS += -DWITH_UNIT_TESTS=0

# Make it explicit that we are cross-compiling
CMAKE_OPTIONS += -DCMAKE_CROSSCOMPILING=1

CMAKE_OPTIONS += \
	-DINSTALL_INCLUDEDIR=include/mysql \
	-DINSTALL_LIBDIR=lib \
	-DINSTALL_PLUGINDIR=lib/mariadb/plugin \
	-DMARIADB_UNIX_ADDR=$(MARIADB_SOCKET) \
	-DWITH_EXTERNAL_ZLIB=YES \
	-DWITH_MYSQLCOMPAT=ON \
	-DWITH_SSL=OPENSSL

# Help MariaDB find the correct libiconv.
# nls.mk sets it up so that with CONFIG_BUILD_NLS libiconv-full would be used,
# otherwise libiconv-stub (independent of the selected libc). MariaDB needs a
# leg up to find/pick the right lib.
CMAKE_OPTIONS += \
	-DICONV_INCLUDE_DIR=$(ICONV_PREFIX)/include \
	-DICONV_LIBRARIES=$(ICONV_PREFIX)/lib/libiconv.$(if $(CONFIG_BUILD_NLS),so,a)

CMAKE_OPTIONS += \
	$(foreach p,$(MARIADB_CLIENT_PLUGINS),-D$(plugin-$(p))=$(if $(CONFIG_PACKAGE_$(PKG_NAME)-plugin-$(subst _,-,$(p))),DYNAMIC,OFF))

# LIBICONV_PLUG is used in GNU's libiconv for redefinition of exports [e.g.
# from libiconv_open() to iconv_open()]. But in OpenWrt this variable is not set
# when building libiconv-full. So when MariaDB sets LIBICONV_PLUG it expects
# iconv_open() to be available for example, which is not the case - only
# libiconv_open() is. To address this prevent the variable from being set.
# libiconv-stub does not use this variable, so there is no harm in always doing
# this.

define Build/Prepare
	$(call Build/Prepare/Default)
	$(SED) '/ADD_DEFINITIONS(-DLIBICONV_PLUG)/d' $(PKG_BUILD_DIR)/CMakeLists.txt
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/include $(1)/usr/lib/mysql $(1)/usr/lib/pkgconfig $(2)/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/include/mysql $(1)/usr/include
	cd $(1)/usr/include/mysql; $(LN) mariadb_version.h mysql_version.h
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{mariadb,mysqlclient}*.so* $(1)/usr/lib
	cd $(1)/usr/lib/mysql; $(LN) ../lib{mariadb,mysqlclient}*.so* .
	$(INSTALL_BIN) files/mysql_config $(1)/usr/bin
	$(LN) $(STAGING_DIR)/usr/bin/mysql_config $(2)/bin
	$(SED) 's/PORT/$(MARIADB_PORT)/' $(1)/usr/bin/mysql_config
	$(SED) 's|PLUGIN_DIR|$(MARIADB_PLUGIN_DIR)|' $(1)/usr/bin/mysql_config
	$(SED) 's|SOCKET|$(MARIADB_SOCKET)|' $(1)/usr/bin/mysql_config
	cd "$(PKG_BUILD_DIR)/mariadb_config"; \
		CLIENT_VERSION=`sed -n 's|^#define[[:blank:]]*VERSION[[:blank:]]*"\([0-9.]*\)"|\1|p' mariadb_config.c`; \
		$(SED) "s/VERSION/$$$${CLIENT_VERSION}/" $(1)/usr/bin/mysql_config
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libmariadb.pc $(1)/usr/lib/pkgconfig
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)$(MARIADB_CONF_DIR)/conf.d
	$(INSTALL_DIR) $(1)$(MARIADB_PLUGIN_DIR)
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)$(MARIADB_PLUGIN_DIR)/caching_sha2_password.so \
		$(PKG_INSTALL_DIR)$(MARIADB_PLUGIN_DIR)/client_ed25519.so \
		$(PKG_INSTALL_DIR)$(MARIADB_PLUGIN_DIR)/dialog.so \
		$(PKG_INSTALL_DIR)$(MARIADB_PLUGIN_DIR)/mysql_clear_password.so \
		$(PKG_INSTALL_DIR)$(MARIADB_PLUGIN_DIR)/sha256_password.so \
		$(1)$(MARIADB_PLUGIN_DIR)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libmariadb.so.$(ABI_VERSION) $(1)/usr/lib
	$(INSTALL_DATA) conf/50-client.cnf $(1)$(MARIADB_CONF_DIR)/conf.d
endef

define BuildPlugin
  define Package/$(PKG_NAME)-plugin-$(subst _,-,$(1))
    $(call Package/$(PKG_NAME)/Default)
    TITLE:=$(PKG_NAME) plugin
    DEPENDS:=$(PKG_NAME) $(patsubst +%,+PACKAGE_$(PKG_NAME)-plugin-$(subst _,-,$(1)):%,$(2))
  endef
  define Package/$(PKG_NAME)-plugin-$(subst _,-,$(1))/description
    $(call Package/$(PKG_NAME)/description/Default)

This package provides the $(1) plugin.

  endef
  define Package/$(PKG_NAME)-plugin-$(subst _,-,$(1))/install
	  $(INSTALL_DIR) $$(1)$(MARIADB_PLUGIN_DIR)
	  $(call Package/$(PKG_NAME)/install/plugin,$$(1),$(1))
  endef
  $$(eval $$(call BuildPackage,$(PKG_NAME)-plugin-$(subst _,-,$(1))))
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

$(eval $(call BuildPlugin,auth_gssapi_client,+krb5-libs))
$(eval $(call BuildPlugin,remote_io,+libcurl))
