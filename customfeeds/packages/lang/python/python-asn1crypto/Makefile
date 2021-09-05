#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-asn1crypto
PKG_VERSION:=1.4.0
PKG_RELEASE:=1

PYPI_NAME:=asn1crypto
PKG_HASH:=f4f6e119474e58e04a2b1af817eb585b4fd72bdd89b998624712b5c99be7641c

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-asn1crypto
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:=https://github.com/wbond/asn1crypto
  TITLE:=Fast pure Python lib for ASN1 structures
  DEPENDS:=+python3-light
endef

define Package/python3-asn1crypto/description
Fast ASN.1 parser and serializer with definitions for
private keys, public keys, certificates, CRL, OCSP,
CMS, PKCS#3, PKCS#7, PKCS#8, PKCS#12, PKCS#5, X.509 and TSP
endef

$(eval $(call Py3Package,python3-asn1crypto))
$(eval $(call BuildPackage,python3-asn1crypto))
$(eval $(call BuildPackage,python3-asn1crypto-src))
