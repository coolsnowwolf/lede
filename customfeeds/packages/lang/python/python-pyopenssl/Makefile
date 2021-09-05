#
# Copyright (C) 2015-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyopenssl
PKG_VERSION:=20.0.1
PKG_RELEASE:=1

PYPI_NAME:=pyOpenSSL
PKG_HASH:=4c231c759543ba02560fcd2480c48dcec4dae34c9da7d3747c508227e0624b51

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>, Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_CPE_ID:=cpe:/a:pyopenssl_project:pyopenssl

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pyopenssl
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=OpenSSL wrapper
  URL:=https://pyopenssl.org/
  DEPENDS:= \
      +python3-light \
      +python3-cryptography \
      +python3-six
endef

define Package/python3-pyopenssl/description
Python wrapper module around the OpenSSL library
endef

$(eval $(call Py3Package,python3-pyopenssl))
$(eval $(call BuildPackage,python3-pyopenssl))
$(eval $(call BuildPackage,python3-pyopenssl-src))
