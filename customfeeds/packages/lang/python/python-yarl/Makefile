#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=yarl
PKG_VERSION:=1.6.2
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=c45b49b59a5724869899798e1bbd447ac486215269511d3b76b4c235a1b766b6

PKG_MAINTAINER:=Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-yarl
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Yet another URL library
  URL:=https://github.com/aio-libs/yarl
  DEPENDS:= \
  +python3-light \
  +python3-multidict \
  +python3-urllib \
  +python3-idna
endef

define Package/python3-yarl/description
Yet another URL library
endef

$(eval $(call Py3Package,python3-yarl))
$(eval $(call BuildPackage,python3-yarl))
$(eval $(call BuildPackage,python3-yarl-src))
