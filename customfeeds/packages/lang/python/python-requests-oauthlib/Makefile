#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-requests-oauthlib
PKG_VERSION:=1.3.0
PKG_RELEASE:=2

PKG_MAINTAINER:=Eneas U de Queiroz <cotequeiroz@gmail.com>
PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=LICENSE

PYPI_NAME:=requests-oauthlib
PKG_HASH:=b4261601a71fd721a8bd6d7aa1cc1d6a8a93b4a9f5e96626f8e4d91e8beeaa6a

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-requests-oauthlib
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=OAuthlib auth for Requests
  URL:=https://github.com/requests/requests-oauthlib
  DEPENDS:= \
	+python3 \
	+python3-oauthlib \
	+python3-requests
endef

define Package/python3-requests-oauthlib/description
  This python package provides first-class OAuth library support
  for Requests.
endef

$(eval $(call Py3Package,python3-requests-oauthlib))
$(eval $(call BuildPackage,python3-requests-oauthlib))
$(eval $(call BuildPackage,python3-requests-oauthlib-src))
