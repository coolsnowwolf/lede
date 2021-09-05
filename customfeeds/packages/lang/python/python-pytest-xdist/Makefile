#
# Copyright (C) 2020-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pytest-xdist
PKG_VERSION:=2.2.1
PKG_RELEASE:=1

PYPI_NAME:=pytest-xdist
PKG_HASH:=718887296892f92683f6a51f25a3ae584993b06f7076ce1e1fd482e59a8220a2

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pytest-xdist
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Pytest plugin for parallel test running
  URL:=https://github.com/pytest-dev/pytest-xdist
  DEPENDS:= \
	+python3-light \
	+python3-pytest \
	+python3-pytest-forked \
	+python3-execnet
endef

define Package/python3-pytest-xdist/description
  The pytest framework makes it easy to write small tests, yet scales to support
  complex functional testing for applications and libraries.
endef

$(eval $(call Py3Package,python3-pytest-xdist))
$(eval $(call BuildPackage,python3-pytest-xdist))
$(eval $(call BuildPackage,python3-pytest-xdist-src))
