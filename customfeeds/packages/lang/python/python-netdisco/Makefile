#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-netdisco
PKG_VERSION:=2.6.0
PKG_RELEASE:=2

PYPI_NAME:=netdisco
PKG_HASH:=2b3aca14a1807712a053f11fd80dc251dd821ee4899aefece515287981817762

PKG_MAINTAINER:=Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE.md

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-netdisco
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Discover devices on your local network
  URL:=https://github.com/home-assistant/netdisco
  DEPENDS:= \
	  +python3-light \
	  +python3-requests \
	  +python3-zeroconf
endef

define Package/python3-netdisco/description
  NetDisco is a Python 3 library to discover local devices and services.
  It allows to scan on demand or offer a service that will scan the network in the background in a set interval.
endef

$(eval $(call Py3Package,python3-netdisco))
$(eval $(call BuildPackage,python3-netdisco))
$(eval $(call BuildPackage,python3-netdisco-src))
