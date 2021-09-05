#
# Copyright (C) 2019-2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python3-pyroute2
PKG_VERSION:=0.5.16
PKG_RELEASE:=1

PYPI_NAME:=pyroute2
PKG_HASH:=fe681a2d008cac815b9f287250d69a333fbfc2b2d89c37d58798104057149989

PKG_MAINTAINER:=Martin Matějek <martin.matejek@nic.cz>
PKG_LICENSE:=GPL-2.0-or-later Apache-2.0
PKG_LICENSE_FILES:=LICENSE.GPL.v2 LICENSE.Apache.v2

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pyroute2
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python netlink library
  URL:=https://github.com/svinota/pyroute2
  DEPENDS:= \
	  +python3-light \
	  +python3-distutils \
	  +python3-logging \
	  +python3-multiprocessing \
	  +python3-sqlite3 \
	  +python3-ctypes
endef

define Package/python3-pyroute2/description
  Pyroute2 is a pure Python netlink library.
  The library was started as an RTNL protocol implementation,
  but now it supports many netlink protocols.
endef

PYTHON3_PKG_SETUP_ARGS:=

$(eval $(call Py3Package,python3-pyroute2))
$(eval $(call BuildPackage,python3-pyroute2))
$(eval $(call BuildPackage,python3-pyroute2-src))
