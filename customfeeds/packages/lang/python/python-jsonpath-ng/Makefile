# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-jsonpath-ng
PKG_VERSION:=1.5.2
PKG_RELEASE:=1
PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>

PYPI_NAME:=jsonpath-ng
PKG_HASH:=144d91379be14d9019f51973bd647719c877bfc07dc6f3f5068895765950c69d

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-jsonpath-ng
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:=https://github.com/h2non/jsonpath-ng
  TITLE:=python3-jsonpath-ng
  DEPENDS:=+python3-ply +python3-six +python3-decorator
endef

$(eval $(call Py3Package,python3-jsonpath-ng))
$(eval $(call BuildPackage,python3-jsonpath-ng))
$(eval $(call BuildPackage,python3-jsonpath-ng-src))
