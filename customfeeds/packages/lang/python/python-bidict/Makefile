#
# Copyright (C) 2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-bidict
PKG_VERSION:=0.21.2
PKG_RELEASE:=1

PYPI_NAME:=bidict
PKG_HASH:=4fa46f7ff96dc244abfc437383d987404ae861df797e2fd5b190e233c302be09

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MPL-2.0
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-bidict
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=The bidirectional mapping library
  URL:=https://github.com/jab/bidict
  DEPENDS:= \
	+python3-light
endef

define Package/python3-bidict/description
  The bidirectional mapping library for Python.
endef

$(eval $(call Py3Package,python3-bidict))
$(eval $(call BuildPackage,python3-bidict))
$(eval $(call BuildPackage,python3-bidict-src))
