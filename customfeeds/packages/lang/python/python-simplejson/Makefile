#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-simplejson
PKG_VERSION:=3.17.2
PKG_RELEASE:=1
PKG_LICENSE:=MIT
PKG_CPE_ID:=cpe:/a:simplejson_project:simplejson

PYPI_NAME:=simplejson
PKG_HASH:=75ecc79f26d99222a084fbdd1ce5aad3ac3a8bd535cd9059528452da38b68841

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-simplejson
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
  TITLE:=Simple, fast, extensible JSON encoder/decoder
  URL:=https://simplejson.readthedocs.org/
  DEPENDS:=+python3-light +python3-decimal
endef

define Package/python3-simplejson/description
  Simple, fast, extensible JSON encoder/decoder for Python
endef

define Py3Package/python3-simplejson/filespec
+|$(PYTHON3_PKG_DIR)
-|$(PYTHON3_PKG_DIR)/simplejson/tests
endef

$(eval $(call Py3Package,python3-simplejson))
$(eval $(call BuildPackage,python3-simplejson))
$(eval $(call BuildPackage,python3-simplejson-src))
