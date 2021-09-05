#
# Copyright (C) 2007-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=django-statici18n
PKG_VERSION:=1.9.0
PKG_RELEASE:=2

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=913dd056c1abcbe129b1db806cb4cfb821c4077d7e857d6eb29043ea64f4d1d0

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-statici18n
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Helper for generating JavaScript catalog to static files
  URL:=https://django-statici18n.readthedocs.io
  DEPENDS:= \
	+python3 \
	+python3-django1 \
	+python3-django-appconf \
	+python3-six
endef

define Package/python3-django-statici18n/description
  A Django app that provides helper for generating JavaScript catalog to static files.
endef

$(eval $(call Py3Package,python3-django-statici18n))
$(eval $(call BuildPackage,python3-django-statici18n))
$(eval $(call BuildPackage,python3-django-statici18n-src))
