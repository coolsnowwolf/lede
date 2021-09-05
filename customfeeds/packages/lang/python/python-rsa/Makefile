include $(TOPDIR)/rules.mk

PKG_NAME:=rsa
PKG_VERSION:=4.6
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=109ea5a66744dd859bf16fe904b8d8b627adafb9408753161e766a92e7d681fa

PKG_MAINTAINER:=Daniel Danzberger <daniel@dd-wrt.com>
PKG_LICENSE:=MIT
PKG_CPE_ID:=cpe:/a:python-rsa_project:python-rsa

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-rsa
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=rsa
  URL:=https://stuvel.eu/rsa
  DEPENDS:=+python3 +python3-pyasn1
endef

define Package/python3-rsa/description
  Is a pure-Python RSA implementation. It supports encryption and decryption,
  signing and verifying signatures, and key generation according to PKCS#1 version 1.5.
  It can be used as a Python library as well as on the commandline. 
endef

$(eval $(call Py3Package,python3-rsa))
$(eval $(call BuildPackage,python3-rsa))
$(eval $(call BuildPackage,python3-rsa-src))
