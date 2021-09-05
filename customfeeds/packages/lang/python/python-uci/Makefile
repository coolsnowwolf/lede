#
# Copyright (C) 2018-2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-uci
PKG_VERSION:=0.8.1
PKG_RELEASE:=$(AUTORELEASE)

PYPI_NAME:=pyuci
PKG_HASH:=9287fe41b427dc5c167592d429be48c1e6cfe276225681b1bdefddfe90d7e941

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-uci
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python UCI bindings
  URL:=https://gitlab.nic.cz/turris/pyuci/
  DEPENDS:=+python3-light +libuci
endef

define Package/python3-uci/description
  Python3 bindings for Unified Configuration Interface.
endef

$(eval $(call Py3Package,python3-uci))
$(eval $(call BuildPackage,python3-uci))
$(eval $(call BuildPackage,python3-uci-src))
