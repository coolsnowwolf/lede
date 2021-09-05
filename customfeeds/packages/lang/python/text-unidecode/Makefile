#
# Copyright (C) 2019 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-text-unidecode
PKG_VERSION:=1.3
PKG_RELEASE:=2

PYPI_NAME:=text-unidecode
PKG_HASH:=bad6603bb14d279193107714b288be206cac565dfa49aa5b105294dd5c4aab93

PKG_LICENSE:=Artistic-1.0-cl8
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-text-unidecode
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=The most basic Text::Unidecode port
  URL:=https://github.com/kmike/text-unidecode/
  DEPENDS:= \
      +python3-light
endef

define Package/python3-text-unidecode/description
text-unidecode is the most basic port of the Text::Unidecode Perl library.
endef

$(eval $(call Py3Package,python3-text-unidecode))
$(eval $(call BuildPackage,python3-text-unidecode))
$(eval $(call BuildPackage,python3-text-unidecode-src))
