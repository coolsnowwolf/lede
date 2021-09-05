#
# Copyright (C) 2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-iniconfig
PKG_VERSION:=1.1.1
PKG_RELEASE:=1

PYPI_NAME:=iniconfig
PKG_HASH:=bc3af051d7d14b2ee5ef9969666def0cd1a000e121eaea580d4a313df4b37f32

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-iniconfig
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Simple config-ini parser
  URL:=https://github.com/RonnyPfannschmidt/iniconfig
  DEPENDS:= +python3-light
endef

define Package/python3-iniconfig/description
  iniconfig is a small and simple INI-file parser module.
endef

$(eval $(call Py3Package,python3-iniconfig))
$(eval $(call BuildPackage,python3-iniconfig))
$(eval $(call BuildPackage,python3-iniconfig-src))
