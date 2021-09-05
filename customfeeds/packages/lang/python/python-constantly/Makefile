#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-constantly
PKG_VERSION:=15.1.0
PKG_RELEASE:=2

PYPI_NAME:=constantly
PKG_HASH:=586372eb92059873e29eba4f9dec8381541b4d3834660707faf8ba59146dfc35

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-constantly
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Symbolic constants in Python
  URL:=https://github.com/twisted/constantly
  DEPENDS:=+python3-light
endef

define Package/python3-constantly/description
A library that provides symbolic constant support. It includes
collections and constants with text, numeric, and bit flag values.
Originally twisted.python.constants from the Twisted project.
endef

$(eval $(call Py3Package,python3-constantly))
$(eval $(call BuildPackage,python3-constantly))
$(eval $(call BuildPackage,python3-constantly-src))
