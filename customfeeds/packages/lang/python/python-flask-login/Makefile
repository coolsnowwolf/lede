#
# Copyright (C) 2019-2020 CZ.NIC z.s.p.o. (http://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-flask-login
PKG_VERSION:=0.5.0
PKG_RELEASE:=2

PYPI_NAME:=Flask-Login
PKG_HASH:=6d33aef15b5bcead780acc339464aae8a6e28f13c90d8b1cf9de8b549d1c0b4b

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-flask-login
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=User session management plugin for Flask
  URL:=https://github.com/maxcountryman/flask-login
  DEPENDS:=+python3-light +python3-flask
endef

define Package/python3-flask-login/description
  Flask-Login provides user session management for Flask.
  It handles the common tasks of logging in, logging out,
  and remembering your users’ sessions over extended periods of time.
endef

$(eval $(call Py3Package,python3-flask-login))
$(eval $(call BuildPackage,python3-flask-login))
$(eval $(call BuildPackage,python3-flask-login-src))
