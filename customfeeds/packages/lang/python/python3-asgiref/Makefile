include $(TOPDIR)/rules.mk

PKG_NAME:=asgiref
PKG_VERSION:=3.3.1
PKG_RELEASE:=1

PYPI_NAME:=asgiref
PKG_HASH:=7162a3cb30ab0609f1a4c95938fd73e8604f63bdba516a7f7d64b83ff09478f0

PKG_MAINTAINER:=Peter Stadler <peter.stadler@student.uibk.ac.at>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-asgiref
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Standard for Python asynchronous web apps and servers.
  URL:=https://asgi.readthedocs.io/en/latest/
  DEPENDS:=+python3-light +python3-logging +python3-asyncio
endef

define Package/python3-asgiref/description
  ASGI is a standard for Python asynchronous web apps and servers to communicate
  with each other, and positioned as an asynchronous successor to WSGI.
endef

$(eval $(call Py3Package,python3-asgiref))
$(eval $(call BuildPackage,python3-asgiref))
$(eval $(call BuildPackage,python3-asgiref-src))
