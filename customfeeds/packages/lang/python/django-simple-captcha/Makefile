#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=django-simple-captcha
PKG_VERSION:=0.5.12
PKG_RELEASE:=4

PYPI_NAME:=$(PKG_NAME)
PYPI_SOURCE_EXT:=zip
PKG_HASH:=fc25f0425e282aa82d2a65013049a8dc7c0682f8e05d32681c39a0c55ed322bd

PKG_MAINTAINER:=Eneas U de Queiroz <cotequeiroz@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-simple-captcha
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Simple Django captcha application
  URL:=https://github.com/mbi/django-simple-captcha
  DEPENDS:= \
	+python3 \
	+python3-six \
	+python3-django1 \
	+python3-pillow \
	+python3-django-ranged-response
endef

define Package/python3-django-simple-captcha/description
  Django Simple Captcha is an extremely simple, yet highly customizable Django
  application to add captcha images to any Django form.
endef

$(eval $(call Py3Package,python3-django-simple-captcha))
$(eval $(call BuildPackage,python3-django-simple-captcha))
$(eval $(call BuildPackage,python3-django-simple-captcha-src))
