#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-voluptuous
PKG_VERSION:=0.12.1
PKG_RELEASE:=1

PYPI_NAME:=voluptuous
PKG_HASH:=663572419281ddfaf4b4197fd4942d181630120fb39b333e3adad70aeb56444b

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-voluptuous
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Data validation library
  URL:=https://github.com/alecthomas/voluptuous
  DEPENDS:= \
      +python3-light
endef

define Package/python3-voluptuous/description
It is primarily intended for validating data coming into Python as JSON, YAML, etc.
endef

$(eval $(call Py3Package,python3-voluptuous))
$(eval $(call BuildPackage,python3-voluptuous))
$(eval $(call BuildPackage,python3-voluptuous-src))
