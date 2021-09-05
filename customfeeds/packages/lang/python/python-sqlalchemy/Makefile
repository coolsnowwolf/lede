#
# Copyright (C) 2019-2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-sqlalchemy
PKG_VERSION:=1.3.23
PKG_RELEASE:=1

PYPI_NAME:=SQLAlchemy
PKG_HASH:=6fca33672578666f657c131552c4ef8979c1606e494f78cd5199742dfb26918b

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:sqlalchemy:sqlalchemy

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-sqlalchemy
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Database Abstraction Library
  URL:=https://www.sqlalchemy.org
  DEPENDS:= \
	+python3-light \
	+python3-urllib \
	+python3-decimal \
	+python3-logging
endef

define Package/python3-sqlalchemy/description
  SQLAlchemy is the Python SQL toolkit and Object Relational Mapper
  that gives application developers the full power and flexibility of SQL.
  SQLAlchemy provides a full suite of well known enterprise-level persistence patterns,
  designed for efficient and high-performing database access,
  adapted into a simple and Pythonic domain language.
endef

$(eval $(call Py3Package,python3-sqlalchemy))
$(eval $(call BuildPackage,python3-sqlalchemy))
$(eval $(call BuildPackage,python3-sqlalchemy-src))
