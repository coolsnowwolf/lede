#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

# Please do not update: 3.0.3 is the last version compatible with Django 1.11
PKG_NAME:=django-jsonfield2
PKG_VERSION:=3.0.3
PKG_RELEASE:=1

# Source for 3.0.3 is not available from PyPI
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rpkilby/jsonfield2/tar.gz/$(PKG_VERSION)?
PKG_HASH:=4b79ac28738671fe27cadbd537d50130c35a286207d31d8320c7b48b6cda36ca

PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/jsonfield2-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-jsonfield2
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A reusable Django field to store validated JSON in models
  URL:=https://github.com/rpkilby/jsonfield2
  DEPENDS:=+python3-light +python3-decimal +django
  CONFLICTS:=python3-django-jsonfield
endef

define Package/python3-django-jsonfield2/description
  A modern fork of django-jsonfield, compatible with the latest versions
  of Django.

  jsonfield2 is a reusable model field that allows you to store
  validated JSON, automatically handling serialization to and from the
  database.
endef

$(eval $(call Py3Package,python3-django-jsonfield2))
$(eval $(call BuildPackage,python3-django-jsonfield2))
$(eval $(call BuildPackage,python3-django-jsonfield2-src))
