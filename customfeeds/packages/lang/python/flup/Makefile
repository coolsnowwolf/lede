#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=flup
PKG_VERSION:=1.0.3
PKG_RELEASE:=4

PYPI_NAME:=flup
PKG_HASH:=5eb09f26eb0751f8380d8ac43d1dfb20e1d42eca0fa45ea9289fa532a79cd159

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>, Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=PKG-INFO

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-flup
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Random assortment of WSGI servers
  URL:=https://www.saddi.com/software/flup/
  DEPENDS:=+python3-light +python3-logging 
endef

define Package/python3-flup/description
  Random assortment of WSGI servers.
endef

$(eval $(call Py3Package,python3-flup))
$(eval $(call BuildPackage,python3-flup))
$(eval $(call BuildPackage,python3-flup-src))
