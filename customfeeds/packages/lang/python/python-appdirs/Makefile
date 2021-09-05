# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-appdirs
PKG_VERSION:=1.4.4
PKG_RELEASE:=1
PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>

PYPI_NAME:=appdirs
PKG_HASH:=7d5d0167b2b1ba821647616af46a749d1c653740dd0d2415100fe26e27afdf41
PKG_LICENSE:=MIT

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-appdirs
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:=https://github.com/ActiveState/appdirs
  TITLE:=Determining platform-specific dirs
  DEPENDS:=+python3-light
endef

define Package/python-appdirs/description
  A small Python module for determining appropriate platform-specific
  dirs, e.g. a "user data dir".
endef

$(eval $(call Py3Package,python3-appdirs))
$(eval $(call BuildPackage,python3-appdirs))
$(eval $(call BuildPackage,python3-appdirs-src))
