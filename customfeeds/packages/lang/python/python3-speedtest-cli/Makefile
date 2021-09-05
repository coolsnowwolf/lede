include $(TOPDIR)/rules.mk

PKG_NAME:=python3-speedtest-cli
PKG_VERSION:=2.1.3
PKG_RELEASE:=1

PYPI_NAME:=speedtest-cli
PKG_HASH:=5e2773233cedb5fa3d8120eb7f97bcc4974b5221b254d33ff16e2f1d413d90f0

PKG_MAINTAINER:=Jaymin Patel <jem.patel@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-speedtest-cli
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Command line interface for testing internet bandwidth using speedtest.net
  URL:=https://github.com/sivel/speedtest-cli
  DEPENDS:=+python3-light +python3-pkg-resources +python3-xml +python3-email \
	  +python3-urllib +python3-codecs +python3-openssl
endef

define Package/python3-speedtest-cli/description
  Command line interface for testing internet bandwidth using speedtest.net
endef

$(eval $(call Py3Package,python3-speedtest-cli))
$(eval $(call BuildPackage,python3-speedtest-cli))
$(eval $(call BuildPackage,python3-speedtest-cli-src))
