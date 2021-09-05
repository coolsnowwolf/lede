# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-intelhex
PKG_VERSION:=2.2.1
PKG_RELEASE:=3

PYPI_NAME:=IntelHex
PYPI_SOURCE_NAME:=intelhex
PKG_HASH:=009d8511e0d50639230c39af9607deee771cf026f67ef7507a8c3fd4fa927832

PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE.txt

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-intelhex
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=python3-intelhex
  URL:=https://github.com/bialix/intelhex
  DEPENDS:=+python3-light
endef

define Package/python3-intelhex/description
  This work implements an intelhex Python library to read, write, create from
  scratch and manipulate data from Intel HEX file format.
endef

PYTHON3_PKG_SETUP_ARGS:=

$(eval $(call Py3Package,python3-intelhex))
$(eval $(call BuildPackage,python3-intelhex))
$(eval $(call BuildPackage,python3-intelhex-src))
