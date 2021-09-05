#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=django-picklefield
PKG_VERSION:=2.1.1
PKG_RELEASE:=2

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=67a5e156343e3b032cac2f65565f0faa81635a99c7da74b0f07a0f5db467b646

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

PYTHON3_PKG_SETUP_VARS:= \
  PKG_VERSION="$(PKG_VERSION)"

define Package/python3-django-picklefield
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Pickled object field for Django
  URL:=https://github.com/gintas/django-picklefield/
  DEPENDS:= \
	+python3 \
	+python3-django1
endef

define Package/python3-django-picklefield/description
  Pickled object field for Django
endef

$(eval $(call Py3Package,python3-django-picklefield))
$(eval $(call BuildPackage,python3-django-picklefield))
$(eval $(call BuildPackage,python3-django-picklefield-src))
