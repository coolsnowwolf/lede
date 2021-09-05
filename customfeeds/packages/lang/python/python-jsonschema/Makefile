include $(TOPDIR)/rules.mk

PKG_NAME:=python-jsonschema
PKG_VERSION:=3.2.0
PKG_RELEASE:=5

PYPI_NAME:=jsonschema
PKG_HASH:=c8a85b28d377cc7737e46e2d9f2b4f44ee3c0e1deac6bf46ddefc7187d30797a

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-jsonschema
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=An implementation of JSON Schema validation
  URL:=https://github.com/Julian/jsonschema
  DEPENDS:=+python3-light +python3-attrs +python3-urllib \
	  +python3-six +python3-pyrsistent +python3-setuptools
endef

define Package/python3-jsonschema/description
  jsonschema is an implementation of JSON Schema validation for Python.
endef

$(eval $(call Py3Package,python3-jsonschema))
$(eval $(call BuildPackage,python3-jsonschema))
$(eval $(call BuildPackage,python3-jsonschema-src))
