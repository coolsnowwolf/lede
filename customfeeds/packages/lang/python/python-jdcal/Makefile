#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-jdcal
PKG_VERSION:=1.4.1
PKG_RELEASE:=2

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>, Eneas U de Queiroz <cote2004-github@yahoo.com>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE.txt

PYPI_NAME:=jdcal
PKG_HASH:=472872e096eb8df219c23f2689fc336668bdb43d194094b5cc1707e1640acfc8

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-jdcal
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Julian dates from proleptic Gregorian and Julian calendars
  URL:=https://github.com/phn/jdcal
  DEPENDS:= +python3-light
endef

define Package/python3-jdcal/description
  This module contains functions for converting between Julian dates and calendar dates.
endef

$(eval $(call Py3Package,python3-jdcal))
$(eval $(call BuildPackage,python3-jdcal))
$(eval $(call BuildPackage,python3-jdcal-src))
