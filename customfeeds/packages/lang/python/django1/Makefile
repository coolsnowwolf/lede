#
# Copyright (C) Alexandru Ardelean <ardeleanalex@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=django1
PKG_VERSION:=1.11.29
PKG_RELEASE:=4

PYPI_NAME:=Django
PKG_HASH:=4200aefb6678019a0acf0005cd14cfce3a5e6b9b90d06145fcdd2e474ad4329c

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE LICENSE.python
PKG_CPE_ID:=cpe:/a:djangoproject:django

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django1
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=The web framework for perfectionists with deadlines.
  URL:=https://www.djangoproject.com/
  DEPENDS:= \
	+python3 \
	+python3-pytz
  PROVIDES:=django
endef

define Package/python3-django1/description
  The web framework for perfectionists with deadlines (LTS 1.11 series).
endef

define Py3Package/python3-django1/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/django-admin.py \
		$(1)/usr/bin/django-admin
endef

$(eval $(call Py3Package,python3-django1))
$(eval $(call BuildPackage,python3-django1))
$(eval $(call BuildPackage,python3-django1-src))
