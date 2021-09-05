include $(TOPDIR)/rules.mk

PKG_NAME:=django-cors-headers
PKG_VERSION:=3.5.0
PKG_RELEASE:=1

PYPI_NAME:=django-cors-headers
PKG_HASH:=db82b2840f667d47872ae3e4a4e0a0d72fbecb42779b8aa233fa8bb965f7836a

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Peter Stadler <peter.stadler@student.uibk.ac.at>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-cors-headers
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Django application for handling the server headers required for Cross-Origin Resource Sharing (CORS).
  URL:=https://github.com/adamchainz/django-cors-headers
  DEPENDS:=+python3-django +python3-urllib +python3-light
endef

define Package/python3-django-cors-headers/description
  Django application for handling the server headers required for Cross-Origin Resource Sharing (CORS).
endef

$(eval $(call Py3Package,python3-django-cors-headers))
$(eval $(call BuildPackage,python3-django-cors-headers))
$(eval $(call BuildPackage,python3-django-cors-headers-src))
