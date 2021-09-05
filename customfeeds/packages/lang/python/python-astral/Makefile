#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-astral
PKG_VERSION:=1.10.1
PKG_RELEASE:=2

PYPI_NAME:=astral
PKG_HASH:=d2a67243c4503131c856cafb1b1276de52a86e5b8a1d507b7e08bee51cb67bf1

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-astral
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Calculations for the position of the sun and moon
  URL:=https://github.com/sffjunkie/astral
  DEPENDS+= \
      +python3-light \
      +python3-pytz
endef

define Package/python3-astral/description
Astral is a python module for calculating the times of various aspects of the sun and moon.
endef

$(eval $(call Py3Package,python3-astral))
$(eval $(call BuildPackage,python3-astral))
$(eval $(call BuildPackage,python3-astral-src))
