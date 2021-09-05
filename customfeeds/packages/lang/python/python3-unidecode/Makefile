#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python3-unidecode
PKG_VERSION:=1.1.1
PKG_RELEASE:=2

PYPI_NAME:=Unidecode
PKG_HASH:=2b6aab710c2a1647e928e36d69c21e76b453cd455f4e2621000e54b2a9b8cce8

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-unidecode
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=ASCII transliterations of Unicode text
  URL:=https://github.com/avian2/unidecode
  DEPENDS:=+python3-light
endef

define Package/python-unidecode/description
  Unidecode, lossy ASCII transliterations of Unicode text
endef

$(eval $(call Py3Package,python3-unidecode))
$(eval $(call BuildPackage,python3-unidecode))
$(eval $(call BuildPackage,python3-unidecode-src))
