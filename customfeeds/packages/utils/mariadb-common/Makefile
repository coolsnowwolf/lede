#
# Copyright (C) 2019 Sebastian Kemper <sebastian_ml@gmx.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mariadb-common
PKG_VERSION:=1.0
PKG_RELEASE:=2

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0

CONF_DIR:=/etc/mysql

include $(INCLUDE_DIR)/package.mk

define Package/mariadb-common
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=https://mariadb.org/
  SUBMENU:=Database
  TITLE:=MariaDB database common files
  DEPENDS:=
endef

define Package/mariadb-common/conffiles
$(CONF_DIR)/my.cnf
endef

define Package/mariadb-common/description
MariaDB is a very fast and robust SQL database server.

This package includes shared files, for example $(CONF_DIR)/my.cnf.

endef

define Package/mariadb-common/install
	$(INSTALL_DIR) $(1)$(CONF_DIR)
	$(INSTALL_DATA) conf/my.cnf $(1)$(CONF_DIR)
endef

define Build/Compile
endef

$(eval $(call BuildPackage,mariadb-common))
