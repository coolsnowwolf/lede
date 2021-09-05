#
# Copyright (C) 2016-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-service-identity
PKG_VERSION:=18.1.0
PKG_RELEASE:=2

PYPI_NAME:=service_identity
PKG_HASH:=0858a54aabc5b459d1aafa8a518ed2081a285087f349fe3e55197989232e2e2d

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-service-identity
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Service identity verification
  URL:=https://service-identity.readthedocs.io/
  DEPENDS:= \
      +python3-light \
      +python3-attrs \
      +python3-cryptography \
      +python3-pyasn1 \
      +python3-pyasn1-modules
endef

define Package/python3-service-identity/description
service_identity aspires to give you all the tools you need for
verifying whether a certificate is valid for the intended purposes.
endef

$(eval $(call Py3Package,python3-service-identity))
$(eval $(call BuildPackage,python3-service-identity))
$(eval $(call BuildPackage,python3-service-identity-src))
