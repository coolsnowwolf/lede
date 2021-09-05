include $(TOPDIR)/rules.mk

PKG_NAME:=python-texttable
PKG_VERSION:=1.6.3
PKG_RELEASE:=1

PYPI_NAME:=texttable
PKG_HASH:=ce0faf21aa77d806bbff22b107cc22cce68dc9438f97a2df32c93e9afa4ce436

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-texttable
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Module for creating simple ASCII tables
  URL:=https://github.com/foutaise/texttable/
  DEPENDS:=+python3-light +python3-codecs
endef

define Package/python3-texttable/description
  Module for creating simple ASCII tables
endef

$(eval $(call Py3Package,python3-texttable))
$(eval $(call BuildPackage,python3-texttable))
$(eval $(call BuildPackage,python3-texttable-src))
