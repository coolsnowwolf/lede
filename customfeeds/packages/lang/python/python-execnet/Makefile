#
# Copyright (C) 2020-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-execnet
PKG_VERSION:=1.8.0
PKG_RELEASE:=1

PYPI_NAME:=execnet
PKG_HASH:=b73c5565e517f24b62dea8a5ceac178c661c4309d3aa0c3e420856c072c411b4

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-execnet
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Library for distributed interact mechanism
  URL:=https://execnet.readthedocs.io
  DEPENDS:= +python3-light +python3-apipkg
endef

define Package/python3-execnet/description
  The execnet provides a share-nothing model with channel-send/receive communication for
  distributing execution across many Python interpreters across version, platform and network
  barriers.complex functional testing for applications and libraries.
endef

$(eval $(call Py3Package,python3-execnet))
$(eval $(call BuildPackage,python3-execnet))
$(eval $(call BuildPackage,python3-execnet-src))
