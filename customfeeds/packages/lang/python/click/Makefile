# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=click
PKG_VERSION:=7.1.2
PKG_RELEASE:=1

PYPI_NAME:=click
PKG_HASH:=d2b5255c7c6349bc1bd1e59e08cd12acbbd63ce649f2588755783aa94dfb6b1a

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-click
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Click
  URL:=https://palletsprojects.com/p/click/
  DEPENDS:=+python3-light
endef

define Package/python3-click/description
  Composable command line interface toolkit
endef

$(eval $(call Py3Package,python3-click))
$(eval $(call BuildPackage,python3-click))
$(eval $(call BuildPackage,python3-click-src))
