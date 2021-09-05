#
# Copyright (C) 2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pytest-forked
PKG_VERSION:=1.3.0
PKG_RELEASE:=2

PYPI_NAME:=pytest-forked
PKG_HASH:=6aa9ac7e00ad1a539c41bec6d21011332de671e938c7637378ec9710204e37ca

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pytest-forked
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Runs tests in isolated forked subprocesses
  URL:=https://github.com/pytest-dev/pytest-forked
  DEPENDS:= +python3-light +python3-pytest +python3-py
endef

define Package/python3-pytest-forked/description
  Pytest plugin which enables to run tests in isolated forked subprocesses
endef

$(eval $(call Py3Package,python3-pytest-forked))
$(eval $(call BuildPackage,python3-pytest-forked))
$(eval $(call BuildPackage,python3-pytest-forked-src))
