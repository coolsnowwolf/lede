#
# Copyright (C) 2015-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyasn1-modules
PKG_VERSION:=0.2.8
PKG_RELEASE:=2

PYPI_NAME:=pyasn1-modules
PKG_HASH:=905f84c712230b2c592c19470d3ca8d552de726050d1d1716282a1f6146be65e

PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE.txt
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>, Alexandru Ardelean <ardeleanalex@gmail.com>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pyasn1-modules
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Collection of ASN.1 modules
  URL:=https://github.com/etingof/pyasn1-modules
  DEPENDS:= \
      +python3-light \
      +python3-pyasn1
endef

define Package/python3-pyasn1-modules/description
This is a small but growing collection of ASN.1 data structures
expressed in Python terms using pyasn1 data model.
endef

$(eval $(call Py3Package,python3-pyasn1-modules))
$(eval $(call BuildPackage,python3-pyasn1-modules))
$(eval $(call BuildPackage,python3-pyasn1-modules-src))
