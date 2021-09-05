#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-xmltodict
PKG_VERSION:=0.12.0
PKG_RELEASE:=2

PYPI_NAME:=xmltodict
PKG_HASH:=50d8c638ed7ecb88d90561beedbf720c9b4e851a9fa6c47ebd64e99d166d8a21

PKG_MAINTAINER:=Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-xmltodict
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Work with XML like JSON
  URL:=https://github.com/martinblech/xmltodict
  DEPENDS:= \
	+python3-light \
	+python3-xml \
	+python3-urllib
endef

define Package/python3-xmltodict/description
  xmltodict is a Python module that makes working with XML feel like you are working with JSON.
endef

$(eval $(call Py3Package,python3-xmltodict))
$(eval $(call BuildPackage,python3-xmltodict))
$(eval $(call BuildPackage,python3-xmltodict-src))
