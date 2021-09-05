#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bandwidthd
PKG_VERSION:=2.0.1-35
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/NethServer/bandwidthd/tar.gz/$(PKG_VERSION)?
PKG_HASH:=75f526d9e81c5a543accbb9e197b6b582c293aa20d6cdfc8be5cef43046981c5

PKG_MAINTAINER:=Jean-Michel Lacroix <lacroix@lepine-lacroix.info>
PKG_LICENSE:=GPL-2.0-or-later

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

define Package/bandwidthd/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Bandwidthd
  URL:=http://bandwidthd.sourceforge.net/
endef

define Package/bandwidthd
$(call Package/bandwidthd/Default)
  TITLE += (without database)
  VARIANT:=no-db
  DEPENDS:=+libgd +libpcap
endef

define Package/bandwidthd-pgsql
$(call Package/bandwidthd/Default)
  TITLE += (with postgresql enabled)
  VARIANT:=pgsql
  DEPENDS:=+libgd +libpcap +libpq
endef

define Package/bandwidthd-sqlite
$(call Package/bandwidthd/Default)
  TITLE += (with sqlite enabled)
  VARIANT:=sqlite
  DEPENDS:=+libgd +libpcap +libsqlite3 +php7 +php7-cgi +php7-mod-gd +php7-mod-pdo +php7-mod-pdo-sqlite
endef

define Package/bandwidthd-php
$(call Package/bandwidthd/Default)
  TITLE = PHP files to graph bandwidthd data from postgresql
  VARIANT:=php
  DEPENDS:=+libpcre +libxml2 +php7 +php7-cgi +php7-mod-pgsql +php7-mod-gd
endef

define Package/bandwidthd/description
  A bandwidthd tracking utility.
endef

define Package/bandwidthd-pgsql/description
$(call Package/bandwidthd/description)
With the ability to store in a postgresql database.
endef

define Package/bandwidthd-sqlite/description
$(call Package/bandwidthd/description)
With the ability to store in a sqlite database.
endef

define Package/bandwidthd-php/description
  PHP files to graph bandwidthd data from pgsql.
endef

define Package/bandwidthd/daemon
 This package contains bandwidthd, a bandwidth tracking utility.
endef

define Package/bandwidthd-pgsql/daemon
 This package contains bandwidthd, a bandwidth tracking utility.
endef

define Package/bandwidthd-sqlite/daemon
 This package contains bandwidthd, a bandwidth tracking utility.
endef

define Package/bandwidthd-php/daemon
 This package contains the PHP files to graph the data from a pgsql database.
endef

ifeq ($(BUILD_VARIANT),no-db)
CONFIGURE_ARGS += \
	ac_cv_file__sw_lib=no \
	ac_cv_file__sw_include=no \
	ac_cv_file__usr_pkg_lib=no \
	ac_cv_file__usr_pkg_include=no \
	ac_cv_file__usr_local_pgsql_lib=no \
	ac_cv_file__usr_local_pgsql_include=no \
	ac_cv_lib_pq_PQconnectdb=no \
	ac_cv_lib_sqlite3_sqlite3_open=no
endif

ifeq ($(BUILD_VARIANT),pgsql)
CONFIGURE_ARGS += \
	ac_cv_file__sw_lib=no \
	ac_cv_file__sw_include=no \
	ac_cv_file__usr_pkg_lib=no \
	ac_cv_file__usr_pkg_include=no \
	ac_cv_file__usr_local_pgsql_lib=no \
	ac_cv_file__usr_local_pgsql_include=no \
	ac_cv_lib_sqlite3_sqlite3_open=no
endif

ifeq ($(BUILD_VARIANT),sqlite)
CONFIGURE_ARGS += \
	ac_cv_file__sw_lib=no \
	ac_cv_file__sw_include=no \
	ac_cv_file__usr_pkg_lib=no \
	ac_cv_file__usr_pkg_include=no \
	ac_cv_file__usr_local_pgsql_lib=no \
	ac_cv_file__usr_local_pgsql_include=no \
	ac_cv_lib_pq_PQconnectdb=no
endif

ifeq ($(BUILD_VARIANT),php)
CONFIGURE_ARGS += \
	ac_cv_file__sw_lib=no \
	ac_cv_file__sw_include=no \
	ac_cv_file__usr_pkg_lib=no \
	ac_cv_file__usr_pkg_include=no \
	ac_cv_file__usr_local_pgsql_lib=no \
	ac_cv_file__usr_local_pgsql_include=no \
	ac_cv_lib_pq_PQconnectdb=no \
	ac_cv_lib_sqlite3_sqlite3_open=no
endif

EXTRA_CFLAGS+= $(TARGET_CPPFLAGS) -fgnu89-inline
EXTRA_LDFLAGS+= $(TARGET_LDFLAGS) -Wl,-rpath-link,$(STAGING_DIR)/usr/lib

define Package/bandwidthd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bandwidthd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/bandwidthd.config $(1)/etc/config/bandwidthd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/bandwidthd.init $(1)/etc/init.d/bandwidthd
	$(INSTALL_DIR) $(1)/www
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/phphtdocs/legend.gif $(1)/www/
	$(INSTALL_DATA) ./files/logo-openwrt.gif $(1)/www/logo.gif
endef

define Package/bandwidthd-pgsql/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bandwidthd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/bandwidthd-pgsql.config $(1)/etc/config/bandwidthd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/bandwidthd.init $(1)/etc/init.d/bandwidthd
	$(INSTALL_DIR) $(1)/www
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/phphtdocs/legend.gif $(1)/www/
	$(INSTALL_DATA) ./files/logo-openwrt.gif $(1)/www/logo.gif
	$(INSTALL_DIR) $(1)/usr/share/postgresql
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/phphtdocs/bd_pgsql_purge.sh $(1)/usr/share/postgresql
endef

define Package/bandwidthd-sqlite/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bandwidthd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/bandwidthd-sqlite.config $(1)/etc/config/bandwidthd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/bandwidthd-sqlite.init $(1)/etc/init.d/bandwidthd
	$(INSTALL_DIR) $(1)/www/phphtdocs
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/phphtdocs/legend.gif $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs-sqlite/details.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs-sqlite/footer.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs-sqlite/graph.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs-sqlite/include.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs-sqlite/index.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/logo-openwrt.gif $(1)/www/phphtdocs/logo.gif
	ln -s /var/etc/bandwidthd-php.conf $(1)/www/phphtdocs/config.conf.php
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/phphtdocs/legend.gif $(1)/www/
	$(INSTALL_DATA) ./files/logo-openwrt.gif $(1)/www/logo.gif
endef

define Package/bandwidthd-php/install
	$(INSTALL_DIR) $(1)/www/phphtdocs
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/phphtdocs/legend.gif $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs/details.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs/footer.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs/graph.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs/include.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/phphtdocs/index.php $(1)/www/phphtdocs/
	$(INSTALL_DATA) ./files/logo-openwrt.gif $(1)/www/phphtdocs/logo.gif
	ln -s /var/etc/bandwidthd-php.conf $(1)/www/phphtdocs/config.conf
	$(INSTALL_DIR) $(1)//etc/config
	$(INSTALL_CONF) ./files/bandwidthd-php.config $(1)/etc/config/bandwidthd-php
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/bandwidthd-php.init $(1)/etc/init.d/bandwidthd-php
endef

define Package/bandwidthd/conffiles
/etc/config/bandwidthd
endef

define Package/bandwidthd-pgsql/conffiles
/etc/config/bandwidthd
endef

define Package/bandwidthd-sqlite/conffiles
/etc/config/bandwidthd
endef

define Package/bandwidthd-php/conffiles
/etc/config/bandwidthd-php
endef

$(eval $(call BuildPackage,bandwidthd))
$(eval $(call BuildPackage,bandwidthd-pgsql))
$(eval $(call BuildPackage,bandwidthd-sqlite))
$(eval $(call BuildPackage,bandwidthd-php))
