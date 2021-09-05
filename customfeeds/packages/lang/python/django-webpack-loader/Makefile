#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=django-webpack-loader
PKG_VERSION:=0.6.0
PKG_RELEASE:=8

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=60bab6b9a037a5346fad12d2a70a6bc046afb33154cf75ed640b93d3ebd5f520

PKG_MAINTAINER:=Eneas U de Queiroz <cotequeiroz@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-webpack-loader
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Transparently use webpack in django
  URL:=https://github.com/owais/django-webpack-loader
  DEPENDS:= \
	+python3 \
	+python3-django1
endef

define Package/python3-django-webpack-loader/description
  Use webpack to generate your static bundles without django’s staticfiles or opaque wrappers.
endef

$(eval $(call Py3Package,python3-django-webpack-loader))
$(eval $(call BuildPackage,python3-django-webpack-loader))
$(eval $(call BuildPackage,python3-django-webpack-loader-src))
