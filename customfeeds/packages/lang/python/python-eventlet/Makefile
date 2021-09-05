#
# Copyright (C) 2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-eventlet
PKG_VERSION:=0.30.2
PKG_RELEASE:=1

PYPI_NAME:=eventlet
PKG_HASH:=1811b122d9a45eb5bafba092d36911bca825f835cb648a862bbf984030acff9d

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-eventlet
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python concurrent networking library
  URL:=https://eventlet.net/
  DEPENDS:= \
	+python3-light \
	+python3-six \
	+python3-dns \
	+python3-greenlet \
	+python3-distutils \
	+python3-email \
	+python3-logging \
	+python3-openssl \
	+python3-urllib
endef

define Package/python3-eventlet/description
  Eventlet is a concurrent networking library for Python that
  allows you to change how you run your code, not how you write it.
endef

$(eval $(call Py3Package,python3-eventlet))
$(eval $(call BuildPackage,python3-eventlet))
$(eval $(call BuildPackage,python3-eventlet-src))
