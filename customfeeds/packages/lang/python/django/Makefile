#
# Copyright (C) Alexandru Ardelean <ardeleanalex@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=django
PKG_VERSION:=3.2.3
PKG_RELEASE:=1

PYPI_NAME:=Django
PKG_HASH:=13ac78dbfd189532cad8f383a27e58e18b3d33f80009ceb476d7fcbfc5dcebd8

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>, Peter Stadler <peter.stadler@student.uibk.ac.at>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE LICENSE.python
PKG_CPE_ID:=cpe:/a:djangoproject:django

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=The web framework for perfectionists with deadlines.
  URL:=https://www.djangoproject.com/
  DEPENDS:= +python3 +python3-pytz +python3-sqlparse +python3-asgiref
  CONFLICTS:=python3-django1
  PROVIDES:=django
endef

define Package/python3-django/description
    The web framework for perfectionists with deadlines.
endef

define Py3Package/python3-django/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/django-admin.py \
		$(1)/usr/bin/django-admin
endef

$(eval $(call Py3Package,python3-django))
$(eval $(call BuildPackage,python3-django))
$(eval $(call BuildPackage,python3-django-src))
