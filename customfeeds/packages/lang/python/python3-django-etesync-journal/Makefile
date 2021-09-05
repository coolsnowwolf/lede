include $(TOPDIR)/rules.mk

PKG_NAME:=django-etesync-journal
PKG_VERSION:=1.2.2
PKG_RELEASE:=1

PYPI_NAME:=django-etesync-journal
PKG_HASH:=1b10a6bca45078bff9b78da3757ba118ecae8f0cc1d9db278bd96eab85f594db

PKG_LICENSE:=AGPL-3.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Peter Stadler <peter.stadler@student.uibk.ac.at>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-django-etesync-journal
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=The server side implementation of the EteSync protocol.
  URL:=https://www.etesync.com/
  DEPENDS:=+django +python3-django-restframework +python3-light
endef

define Package/python3-django-etesync-journal/description
  The reusable django app that implements the server side of the EteSync protocol.
endef

$(eval $(call Py3Package,python3-django-etesync-journal))
$(eval $(call BuildPackage,python3-django-etesync-journal))
$(eval $(call BuildPackage,python3-django-etesync-journal-src))
