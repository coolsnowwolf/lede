include $(TOPDIR)/rules.mk

PKG_NAME:=python-pynacl
PKG_VERSION:=1.4.0
PKG_RELEASE:=5

PYPI_NAME:=PyNaCl
PKG_HASH:=54e9a2c849c742006516ad56a88f5c74bf2ce92c9f67435187c3c5953b346505

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=libffi/host

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=cffi  # cffi>=1.4.1

PYTHON3_PKG_SETUP_VARS:= SODIUM_INSTALL=system

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-pynacl
  CATEGORY:=Languages
  SECTION:=lang
  SUBMENU:=Python
  TITLE:=Python binding to the libsodium library
  URL:=https://github.com/pyca/pynacl/
  DEPENDS:=+libsodium +python3-light +python3-cffi +python3-six
endef

define Package/python3-pynacl/description
  PyNaCl is a Python binding to libsodium, which is a fork of the Networking and Cryptography library.
endef

$(eval $(call Py3Package,python3-pynacl))
$(eval $(call BuildPackage,python3-pynacl))
$(eval $(call BuildPackage,python3-pynacl-src))
