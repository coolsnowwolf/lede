#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=django-appconf
PKG_VERSION:=1.0.4
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=be58deb54a43d77d2e1621fe59f787681376d3cd0b8bd8e4758ef6c3a6453380

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-appconf
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Helper class for handling config defaults
  URL:=https://django-appconf.readthedocs.io
  DEPENDS:= \
	+python3 \
	+python3-django1 \
	+python3-six
endef

define Package/python3-django-appconf/description
  A helper class for handling configuration defaults of packaged apps gracefully
endef

$(eval $(call Py3Package,python3-django-appconf))
$(eval $(call BuildPackage,python3-django-appconf))
$(eval $(call BuildPackage,python3-django-appconf-src))
