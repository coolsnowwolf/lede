#
# Copyright (C) 2020-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pytest
PKG_VERSION:=6.2.3
PKG_RELEASE:=1

PYPI_NAME:=pytest
PKG_HASH:=671238a46e4df0f3498d1c3270e5deb9b32d25134c99b7d75370a68cfbe9b634

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pytest
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python testing framework
  URL:=https://docs.pytest.org/en/latest/
  DEPENDS:= \
	+python3 \
	+python3-more-itertools \
	+python3-py \
	+python3-attrs \
	+python3-pluggy \
	+python3-packaging \
	+python3-toml \
	+python3-iniconfig
endef

define Package/python3-pytest/description
  The pytest framework makes it easy to write small tests, yet scales to support
  complex functional testing for applications and libraries.
endef

$(eval $(call Py3Package,python3-pytest))
$(eval $(call BuildPackage,python3-pytest))
$(eval $(call BuildPackage,python3-pytest-src))
