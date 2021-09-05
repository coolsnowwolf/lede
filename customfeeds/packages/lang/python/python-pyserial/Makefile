#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyserial
PKG_VERSION:=3.4
PKG_RELEASE:=2

PYPI_NAME:=pyserial
PKG_HASH:=6e2d401fdee0eab996cf734e67773a0143b932772ca8b42451440cfed942c627

PKG_LICENSE:=BSD
PKG_MAINTAINER:=Micke Prag <micke.prag@telldus.se>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pyserial
  SECTION:=lang-python
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=python3-pyserial
  URL:=https://github.com/pyserial/pyserial
  DEPENDS:=+python3-light
endef

define Package/python3-pyserial/description
This module encapsulates the access for the serial port. It provides backends
for Python running on Windows, OSX, Linux, BSD (possibly any POSIX compliant
system) and IronPython. The module named "serial" automatically selects the
appropriate backend.
endef

$(eval $(call Py3Package,python3-pyserial))
$(eval $(call BuildPackage,python3-pyserial))
$(eval $(call BuildPackage,python3-pyserial-src))
