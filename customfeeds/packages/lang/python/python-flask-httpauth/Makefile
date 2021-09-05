#
# Copyright (C) 2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python3-flask-httpauth
PKG_VERSION:=4.2.0
PKG_RELEASE:=1

PYPI_NAME:=Flask-HTTPAuth
PKG_HASH:=8c7e49e53ce7dc14e66fe39b9334e4b7ceb8d0b99a6ba1c3562bb528ef9da84a

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-flask-httpauth
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Basic and Digest HTTP authentication for Flask routes
  URL:=https://github.com/miguelgrinberg/flask-httpauth
  DEPENDS:= \
	+python3-light \
	+python3-flask
endef

define Package/python3-flask-httpauth/description
  Flask-HTTPAuth is a Flask extension that simplifies the
  use of HTTP authentication with Flask routes
endef

$(eval $(call Py3Package,python3-flask-httpauth))
$(eval $(call BuildPackage,python3-flask-httpauth))
$(eval $(call BuildPackage,python3-flask-httpauth-src))
