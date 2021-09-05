# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-schema
PKG_VERSION:=0.7.2
PKG_RELEASE:=1
PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>

PYPI_NAME:=schema
PKG_HASH:=b536f2375b49fdf56f36279addae98bd86a8afbd58b3c32ce363c464bed5fc1c

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-schema
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:=https://github.com/keleshev/schema
  TITLE:=python3-schema
  DEPENDS:=+python3-light +python3-contextlib2
endef

define Package/python3-schema/description
  Schema validation just got Pythonic
endef

$(eval $(call Py3Package,python3-schema))
$(eval $(call BuildPackage,python3-schema))
$(eval $(call BuildPackage,python3-schema-src))
