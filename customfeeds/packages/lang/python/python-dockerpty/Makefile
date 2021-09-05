include $(TOPDIR)/rules.mk

PKG_NAME:=python-dockerpty
PKG_VERSION:=0.4.1
PKG_RELEASE:=3

PYPI_NAME:=dockerpty
PKG_HASH:=69a9d69d573a0daa31bcd1c0774eeed5c15c295fe719c61aca550ed1393156ce

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE.txt

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

PYTHON3_PKG_SETUP_ARGS:=

define Package/python3-dockerpty
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python library to use the pseudo-tty of a docker container
  URL:=https://github.com/d11wtq/dockerpty
  DEPENDS:=+python3-light +python3-openssl +python3-six
endef

define Package/python3-dockerpty/description
  Python library to use the pseudo-tty of a docker container
endef

$(eval $(call Py3Package,python3-dockerpty))
$(eval $(call BuildPackage,python3-dockerpty))
$(eval $(call BuildPackage,python3-dockerpty-src))
