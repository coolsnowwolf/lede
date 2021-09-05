#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-cachetools
PKG_VERSION:=3.1.1
PKG_RELEASE:=2

PYPI_NAME:=cachetools
PKG_HASH:=8ea2d3ce97850f31e4a08b0e2b5e6c34997d7216a9d2c98e0f3978630d4da69a

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-cachetools
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Extensible memoizing collections and decorators
  URL:=https://github.com/tkem/cachetools
  DEPENDS:=+python3-light
endef

define Package/python3-cachetools/description
  This module provides various memoizing collections and decorators, including variants of the Python 3 Standard Library lru_cache function decorator.
endef

$(eval $(call Py3Package,python3-cachetools))
$(eval $(call BuildPackage,python3-cachetools))
$(eval $(call BuildPackage,python3-cachetools-src))
