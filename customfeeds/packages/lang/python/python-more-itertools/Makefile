#
# Copyright (C) 2019-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-more-itertools
PKG_VERSION:=8.7.0
PKG_RELEASE:=1

PYPI_NAME:=more-itertools
PKG_HASH:=c5d6da9ca3ff65220c3bfd2a8db06d698f05d4d2b9be57e1deb2be5a45019713

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-more-itertools
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=More routines for operating on iterables, beyond itertools
  URL:=https://github.com/erikrose/more-itertools
  DEPENDS:=+python3-light
endef

define Package/python3-more-itertools/description
  more-itertools library collects additional building blocks,
  recipes, and routines for working with Python iterables.
endef

$(eval $(call Py3Package,python3-more-itertools))
$(eval $(call BuildPackage,python3-more-itertools))
$(eval $(call BuildPackage,python3-more-itertools-src))
