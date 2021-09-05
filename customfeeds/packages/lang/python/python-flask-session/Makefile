#
# Copyright (C) 2019-2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-flask-session
PKG_VERSION:=0.3.2
PKG_RELEASE:=1

PYPI_NAME:=Flask-Session
PKG_HASH:=0768e2bbf06f963ec1aa711bde7aa32dc39ff70f89b495d6db687d899eae4423

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-flask-session
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Flask Session
  URL:=https://github.com/fengsp/flask-session
  DEPENDS:= \
    +python3-cachelib \
    +python3-flask \
    +python3-light
endef

define Package/python3-flask-session/description
  Adds server-side session support to your Flask application.
endef

$(eval $(call Py3Package,python3-flask-session))
$(eval $(call BuildPackage,python3-flask-session))
$(eval $(call BuildPackage,python3-flask-session-src))
