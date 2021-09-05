include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=python-libmodbus
PKG_VERSION:=0.5.0
PKG_RELEASE:=2

PKG_LICENSE:=BSD-3-Clause
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PYPI_NAME:=pylibmodbus
PKG_HASH:=80f837304ffa8476145ea643f6b98aa94b205013a96f1e5173d7bdc776426aee

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-libmodbus
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=pylibmodbus
  URL:=https://pypi.org/project/pylibmodbus
  DEPENDS:=+libmodbus \
     +python3-light \
     +python3-cffi
endef

define Package/python3-libmodbus/description
	Python Interface for libmodbus written with CFFI.
endef

$(eval $(call Py3Package,python3-libmodbus))
$(eval $(call BuildPackage,python3-libmodbus))
