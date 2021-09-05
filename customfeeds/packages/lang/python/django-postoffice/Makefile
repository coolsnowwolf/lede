#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

# Please do not update: 3.3.0 is the last version compatible with Django 1.11
PKG_NAME:=django-postoffice
PKG_VERSION:=3.3.0
PKG_RELEASE:=1

PYPI_NAME:=django-post-office
PYPI_SOURCE_NAME:=django-post_office
PKG_HASH:=b06514da601c22e955bd93a4ac6dd6b2218c571ca67c193e62bd1f22cec7536f

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.txt

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-postoffice
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A Django app to monitor and send mail asynchronously
  URL:=https://github.com/ui/django-post_office
  DEPENDS:= \
	+python3 \
	+python3-django1 \
	+python3-django-jsonfield2
endef

define Package/python3-django-postoffice/description
  A Django app to monitor and send mail asynchronously, complete with template
  support
endef

$(eval $(call Py3Package,python3-django-postoffice))
$(eval $(call BuildPackage,python3-django-postoffice))
$(eval $(call BuildPackage,python3-django-postoffice-src))
