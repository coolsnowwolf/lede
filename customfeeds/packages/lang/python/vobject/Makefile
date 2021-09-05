# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=vobject
PKG_VERSION:=0.9.6.1
PKG_RELEASE:=2
PKG_LICENSE:=Apache-2.0

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=96512aec74b90abb71f6b53898dd7fe47300cc940104c4f79148f0671f790101

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk

include ../python3-package.mk

define Package/python3-vobject
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=VObject
  URL:=http://eventable.github.io/vobject/
  DEPENDS:=+python3 +python3-dateutil
endef

define Package/python3-vobject/description
  vCard and vCalendar support for Python
endef

$(eval $(call Py3Package,python3-vobject))
$(eval $(call BuildPackage,python3-vobject))
$(eval $(call BuildPackage,python3-vobject-src))
