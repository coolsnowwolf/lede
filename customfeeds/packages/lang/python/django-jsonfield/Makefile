#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=django-jsonfield
PKG_VERSION:=2.1.1
PKG_RELEASE:=2

PYPI_NAME:=jsonfield
PKG_HASH:=ed7c5e1829e9453e24a8bebef1e702ffe402e6def6b326f0e0b88764c59a6dc7

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-jsonfield
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A reusable Django field to store validated JSON in models
  URL:=https://github.com/rpkilby/jsonfield
  DEPENDS:= \
	+python3 \
	+python3-django1 \
	+python3-six
endef

define Package/python3-django-jsonfield/description
  jsonfield is a reusable model field that allows you to store validated
  JSON, automatically handling serialization to and from the database.
endef

$(eval $(call Py3Package,python3-django-jsonfield))
$(eval $(call BuildPackage,python3-django-jsonfield))
$(eval $(call BuildPackage,python3-django-jsonfield-src))
