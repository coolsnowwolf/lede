#
# Copyright (C) 2019-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-py
PKG_VERSION:=1.10.0
PKG_RELEASE:=1

PYPI_NAME:=py
PKG_HASH:=21b81bda15b66ef5e1a777a21c4dcd9c20ad3efd0b3f817e7a809035269e1bd3

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-py
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=py
  URL:=https://github.com/pytest-dev/py
  DEPENDS:=+python3-light +python3-xml +python3-urllib
endef

define Package/python3-py/description
  Library with cross-python path, ini-parsing, io, code, log facilities
endef

$(eval $(call Py3Package,python3-py))
$(eval $(call BuildPackage,python3-py))
$(eval $(call BuildPackage,python3-py-src))
