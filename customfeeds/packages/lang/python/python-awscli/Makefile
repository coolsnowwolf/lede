include $(TOPDIR)/rules.mk

PKG_NAME:=awscli
PKG_VERSION:=1.16.75
PKG_RELEASE:=2

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=8d96ec0de325ea8271cc6aa95b7392bbf548ec4aabd3ffbcdc0619b64edd4a45

PKG_MAINTAINER:=Daniel Danzberger <daniel@dd-wrt.com>
PKG_LICENSE:=MIT

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-awscli
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=awscli
  URL:=https://github.com/aws/aws-cli
  DEPENDS:=+python3 \
	+python3-yaml \
	+python3-pyasn1 \
	+python3-botocore \
	+python3-rsa \
	+python3-colorama \
	+python3-docutils \
	+python3-s3transfer
endef

define Package/python3-awscli/description
This package provides a unified command line interface to Amazon Web Services.
endef

$(eval $(call Py3Package,python3-awscli))
$(eval $(call BuildPackage,python3-awscli))
$(eval $(call BuildPackage,python3-awscli-src))
