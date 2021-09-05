#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-defusedxml
PKG_VERSION:=0.6.0
PKG_RELEASE:=3

PKG_MAINTAINER:=Eneas U de Queiroz <cote2004-github@yahoo.com>
PKG_LICENSE:=Python-2.0
PKG_LICENSE_FILES:=LICENSE

PYPI_NAME:=defusedxml
PKG_HASH:=f684034d135af4c6cbb949b8a4d2ed61634515257a67299e5f940fbaa34377f5

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-defusedxml
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=XML bomb protection for Python
  URL:=https://github.com/tiran/defusedxml
  DEPENDS:= \
	+python3-light \
	+python3-xml
endef

define Package/python3-defusedxml/description
  The defusedxml package contains several Python-only workarounds and fixes for
  denial of service and other vulnerabilities in Python’s XML libraries.
endef

$(eval $(call Py3Package,python3-defusedxml))
$(eval $(call BuildPackage,python3-defusedxml))
$(eval $(call BuildPackage,python3-defusedxml-src))
