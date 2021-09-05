#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-certifi
PKG_VERSION:=2020.6.20
PKG_RELEASE:=1

PKG_MAINTAINER:=Eneas U de Queiroz <cotequeiroz@gmail.com>
PKG_LICENSE:=MPL-2.0
PKG_LICENSE_FILES:=LICENSE

PYPI_NAME:=certifi
PKG_HASH:=5930595817496dd21bb8dc35dad090f1c2cd0adfaf21204bf6732ca5d8ee34d3

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-certifi
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python package for Mozilla's CA Bundle
  URL:=http://certifi.io/
  DEPENDS:=+python3-light
endef

define Package/python3-certifi/description
  Certifi is a carefully curated collection of Root Certificates for validating the
  trustworthiness of SSL certificates while verifying the identity of TLS hosts.
endef

$(eval $(call Py3Package,python3-certifi))
$(eval $(call BuildPackage,python3-certifi))
$(eval $(call BuildPackage,python3-certifi-src))
