#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aiohttp-cors
PKG_VERSION:=0.7.0
PKG_RELEASE:=2

PYPI_NAME:=aiohttp_cors
PYPI_SOURCE_NAME:=aiohttp-cors
PKG_HASH:=4d39c6d7100fd9764ed1caf8cebf0eb01bf5e3f24e2e073fda6234bc48b19f5d

PKG_MAINTAINER:=Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-aiohttp-cors
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=CORS support for aiohttp
  URL:=https://github.com/aio-libs/aiohttp-cors
  DEPENDS:= \
	+python3-aiohttp \
	+python3-light
endef

define Package/python3-aiohttp-cors/description
  Implements Cross Origin Resource Sharing (CORS) support for aiohttp asyncio-powered asynchronous HTTP server.
endef

$(eval $(call Py3Package,python3-aiohttp-cors))
$(eval $(call BuildPackage,python3-aiohttp-cors))
$(eval $(call BuildPackage,python3-aiohttp-cors-src))
