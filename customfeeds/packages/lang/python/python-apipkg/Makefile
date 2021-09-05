#
# Copyright (C) 2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-apipkg
PKG_VERSION:=1.5
PKG_RELEASE:=2

PYPI_NAME:=apipkg
PKG_HASH:=37228cda29411948b422fae072f57e31d3396d2ee1c9783775980ee9c9990af6

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-apipkg
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:= Controler for exported namespace of a Python package a
  URL:=https://github.com/pytest-dev/apipkg
  DEPENDS:= +python3-light
endef

define Package/python3-apipkg/description
  apipkg is used to control the exported namespace of a Python package and greatly reduce
  the number of imports for users.
endef

$(eval $(call Py3Package,python3-apipkg))
$(eval $(call BuildPackage,python3-apipkg))
$(eval $(call BuildPackage,python3-apipkg-src))
