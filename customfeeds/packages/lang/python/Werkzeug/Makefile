# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=Werkzeug
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=6c80b1e5ad3665290ea39320b91e1be1e0d5f60652b964a3070216de83d2e47c

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE.rst

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-werkzeug
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Werkzeug
  URL:=https://palletsprojects.com/p/werkzeug/
  DEPENDS:=+python3-light +python3-email
endef

define Package/python3-werkzeug/description
 The comprehensive WSGI web application library.
endef

$(eval $(call Py3Package,python3-werkzeug))
$(eval $(call BuildPackage,python3-werkzeug))
$(eval $(call BuildPackage,python3-werkzeug-src))
