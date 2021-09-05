#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=async-timeout
PKG_VERSION:=3.0.1
PKG_RELEASE:=2

PYPI_NAME:=async_timeout
PYPI_SOURCE_NAME:=async-timeout
PKG_HASH:=0c3c816a028d47f659d6ff5c745cb2acf1f966da1fe5c19c77a70282b25f4c5f

PKG_MAINTAINER:=Josef Schlehofer <pepe.schlehofer@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-async-timeout
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Timeout context manager for asyncio programs
  URL:=https://github.com/aio-libs/async-timeout
  DEPENDS:= \
  +python3-light \
  +python3-asyncio
endef

define Package/python3-async-timeout/description
Timeout context manager for asyncio programs
endef

$(eval $(call Py3Package,python3-async-timeout))
$(eval $(call BuildPackage,python3-async-timeout))
$(eval $(call BuildPackage,python3-async-timeout-src))
