#
# Copyright (C) 2019-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aiohttp
PKG_VERSION:=3.7.4
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=5d84ecc73141d0a0d61ece0742bb7ff5751b0657dab8405f899d3ceb104cc7de

PKG_MAINTAINER:=Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:aio-libs_project:aiohttp

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-aiohttp
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Async http client/server framework (asyncio)
  URL:=https://github.com/aio-libs/aiohttp
  DEPENDS:= \
	+python3-light \
	+python3-attrs \
	+python3-chardet \
	+python3-multidict \
	+python3-async-timeout \
	+python3-yarl \
	+python3-logging \
	+python3-codecs \
	+python3-cgi \
	+python3-openssl \
	+python3-typing-extensions
endef

define Package/python3-aiohttp/description
  Asynchronous HTTP client/server framework for asyncio and Python3.
endef

$(eval $(call Py3Package,python3-aiohttp))
$(eval $(call BuildPackage,python3-aiohttp))
$(eval $(call BuildPackage,python3-aiohttp-src))
