#
# Copyright (C) 2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pysocks
PKG_VERSION:=1.7.1
PKG_RELEASE:=1

PYPI_NAME:=PySocks
PKG_HASH:=3f8804571ebe159c380ac6de37643bb4685970655d3bba243530d6558b799aa0

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pysocks
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python SOCKS client module
  URL:=https://github.com/Anorov/PySocks
  DEPENDS:= \
	+python3-light \
	+python3-logging \
	+python3-openssl \
	+python3-urllib
endef

define Package/python3-pysocks/description
  PySocks lets you send traffic through SOCKS proxy servers.
  It is a modern fork of SocksiPy with bug fixes and extra features.
endef

$(eval $(call Py3Package,python3-pysocks))
$(eval $(call BuildPackage,python3-pysocks))
$(eval $(call BuildPackage,python3-pysocks-src))
