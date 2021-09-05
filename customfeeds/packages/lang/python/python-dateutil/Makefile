#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-dateutil
PKG_VERSION:=2.8.1
PKG_RELEASE:=2
PKG_LICENSE:=BSD-2-Clause

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=73ebfe9dbf22e832286dafa60473e4cd239f8592f699aa5adaf10050e6e1823c
PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk

include ../python3-package.mk

define Package/python3-dateutil
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Extensions to the standard Python datetime module
  URL:=https://dateutil.readthedocs.org/
  DEPENDS:=+python3 +python3-six
endef

define Package/python3-dateutil/description
  Extensions to the standard Python datetime module
endef

$(eval $(call Py3Package,python3-dateutil))
$(eval $(call BuildPackage,python3-dateutil))
$(eval $(call BuildPackage,python3-dateutil-src))
