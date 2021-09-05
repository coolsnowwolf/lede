include $(TOPDIR)/rules.mk

PKG_NAME:=docker-compose
PKG_VERSION:=1.29.2
PKG_RELEASE:=1

PYPI_NAME:=docker-compose
PKG_HASH:=4c8cd9d21d237412793d18bd33110049ee9af8dab3fe2c213bbd0733959b09b7

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include ../../lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk

PYTHON3_PKG_SETUP_ARGS:=

define Package/docker-compose
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Docker Compose
  URL:=https://github.com/docker/compose
  DEPENDS+=+docker \
      +python3-light \
      +python3-cached-property \
      +python3-distro \
      +python3-distutils \
      +python3-docopt \
      +python3-docker \
      +python3-dockerpty \
      +python3-dotenv \
      +python3-jsonschema \
      +python3-logging \
      +python3-openssl \
      +python3-pkg-resources \
      +python3-requests \
      +python3-texttable \
      +python3-websocket-client \
      +python3-yaml
endef

define Package/docker-compose/description
  Multi-container orchestration for Docker
endef

$(eval $(call Py3Package,docker-compose))
$(eval $(call BuildPackage,docker-compose))
$(eval $(call BuildPackage,docker-compose-src))
