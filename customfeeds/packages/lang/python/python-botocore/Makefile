include $(TOPDIR)/rules.mk

PKG_NAME:=botocore
PKG_VERSION:=1.12.66
PKG_RELEASE:=2

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=25c39ecc71356287cf79d66981ec77deca374e28043b19b2f818d48aa34272a1

PKG_MAINTAINER:=Daniel Danzberger <daniel@dd-wrt.com>
PKG_LICENSE:=MIT

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-botocore
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=botocore
  URL:=https://github.com/boto/botocore
  DEPENDS:=+python3 \
	+python3-urllib3 \
	+python3-docutils \
	+python3-dateutil \
	+python3-jmespath \
	+python3-requests
endef

define Package/python3-botocore/description
A low-level interface to a growing number of Amazon Web Services.
The botocore package is the foundation for the AWS CLI as well as boto3.
endef

$(eval $(call Py3Package,python3-botocore))
$(eval $(call BuildPackage,python3-botocore))
$(eval $(call BuildPackage,python3-botocore-src))
