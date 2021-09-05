#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python3-netifaces
PKG_VERSION:=0.10.9
PKG_RELEASE:=2

PYPI_NAME:=netifaces
PKG_HASH:=2dee9ffdd16292878336a58d04a20f0ffe95555465fee7c9bd23b3490ef2abf3

PKG_MAINTAINER:=Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-netifaces
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Portable network interface information
  URL:=https://github.com/al45tair/netifaces
  DEPENDS:=+python3-light
endef

define Package/python3-netifaces/description
  Portable network interface information.
endef

$(eval $(call Py3Package,python3-netifaces))
$(eval $(call BuildPackage,python3-netifaces))
$(eval $(call BuildPackage,python3-netifaces-src))
