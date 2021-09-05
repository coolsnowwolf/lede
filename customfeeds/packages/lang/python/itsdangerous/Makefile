# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=itsdangerous
PKG_VERSION:=1.1.0
PKG_RELEASE:=3

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=321b033d07f2a4136d3ec762eac9f16a10ccd60f53c0c91af90217ace7ba1f19

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-itsdangerous
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=ItsDangerous
  URL:=https://palletsprojects.com/p/itsdangerous/
  DEPENDS:=+python3-light
endef

define Package/python3-itsdangerous/description
  Various helpers to pass trusted data to untrusted environments and back.
endef

$(eval $(call Py3Package,python3-itsdangerous))
$(eval $(call BuildPackage,python3-itsdangerous))
$(eval $(call BuildPackage,python3-itsdangerous-src))

