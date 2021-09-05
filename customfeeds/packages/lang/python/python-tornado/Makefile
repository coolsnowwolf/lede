#
# Copyright (C) 2020 CZ.NIC z.s.p.o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-tornado
PKG_VERSION:=6.1
PKG_RELEASE:=1

PYPI_NAME:=tornado
PKG_HASH:=33c6e81d7bd55b468d2e793517c909b139960b6c790a60b7991b9b6b76fb9791

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-tornado
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Web framework and asynchronous networking library
  URL:=https://tornadoweb.org
  DEPENDS:= \
    +python3-asyncio \
    +python3-codecs \
    +python3-email \
    +python3-light \
    +python3-logging \
    +python3-multiprocessing \
    +python3-openssl \
    +python3-unittest \
    +python3-urllib
endef

define Package/python3-tornado/description
  Tornado is a Python web framework and asynchronous networking library,
  originally developed at FriendFeed. By using non-blocking network I/O,
  Tornado can scale to tens of thousands of open connections, making it
  ideal for long polling, WebSockets, and other applications that require
  a long-lived connection to each user.
endef

$(eval $(call Py3Package,python3-tornado))
$(eval $(call BuildPackage,python3-tornado))
$(eval $(call BuildPackage,python3-tornado-src))
