#
# Copyright (C) 2007-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-requests
PKG_VERSION:=2.25.1
PKG_RELEASE:=2

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>, Alexandru Ardelean <ardeleanalex@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:python-requests:requests

PYPI_NAME:=requests
PKG_HASH:=27973dd4a904a4f13b263a19c866c13b92a39ed1c964655f025f3f8d3d75b804

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-requests
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=HTTP library for Python
  URL:=https://2.python-requests.org/
  DEPENDS:= \
	  +python3-light  \
	  +python3-chardet  \
	  +python3-idna  \
	  +python3-urllib3  \
	  +python3-certifi
endef

define Package/python3-requests/description
  Requests is the only Non-GMO HTTP library for Python, safe for human consumption
endef

$(eval $(call Py3Package,python3-requests))
$(eval $(call BuildPackage,python3-requests))
$(eval $(call BuildPackage,python3-requests-src))
