# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-cachelib
PKG_VERSION:=0.1
PKG_RELEASE:=2

PYPI_NAME:=cachelib
PKG_HASH:=8b889b509d372095357b8705966e1282d40835c4126d7c2b07fd414514d8ae8d

PKG_MAINTAINER:=Stepan Henek <stepan.henek@nic.cz>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-cachelib
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=cachelib
  URL:=https://github.com/pallets/cachelib
  DEPENDS:=+python3-light
endef

define Package/python3-cachelib/description
  A collection of cache libraries in the same API interface.
  Extracted from werkzeug.
endef

$(eval $(call Py3Package,python3-cachelib))
$(eval $(call BuildPackage,python3-cachelib))
$(eval $(call BuildPackage,python3-cachelib-src))
