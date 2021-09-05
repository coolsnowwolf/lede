#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyrsistent
PKG_VERSION:=0.16.0
PKG_RELEASE:=1

PYPI_NAME:=pyrsistent
PKG_HASH:=28669905fe725965daa16184933676547c5bb40a5153055a8dee2a4bd7933ad3

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.mit

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pyrsistent
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Persistent/Functional/Immutable data structures
  URL:=https://github.com/tobgu/pyrsistent
  DEPENDS:=+python3-light +python3-six
endef

define Package/python3-pyrsistent/description
  Pyrsistent is a number of persistent collections (by some referred to as functional data structures).
  Persistent in the sense that they are immutable.
endef

$(eval $(call Py3Package,python3-pyrsistent))
$(eval $(call BuildPackage,python3-pyrsistent))
$(eval $(call BuildPackage,python3-pyrsistent-src))
