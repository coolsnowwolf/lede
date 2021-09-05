include $(TOPDIR)/rules.mk

PKG_NAME:=python-docker
PKG_VERSION:=5.0.0
PKG_RELEASE:=1

PYPI_NAME:=docker
PKG_HASH:=3e8bc47534e0ca9331d72c32f2881bb13b93ded0bcdeab3c833fb7cf61c0a9a5

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

PYTHON3_PKG_SETUP_ARGS:=

define Package/python3-docker
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=A Python library for the Docker Engine API
  URL:=https://github.com/docker/docker-py
  DEPENDS:=\
	  +python3-light +python3-distutils +python3-logging \
	  +python3-openssl +python3-paramiko +python3-six +python3-requests \
	  +python3-websocket-client
endef

define Package/python3-docker/description
  A Python library for the Docker Engine API. It lets you do anything the
  docker command does, but from within Python apps – run containers manage
  containers, manage Swarms, etc.
endef

$(eval $(call Py3Package,python3-docker))
$(eval $(call BuildPackage,python3-docker))
$(eval $(call BuildPackage,python3-docker-src))
