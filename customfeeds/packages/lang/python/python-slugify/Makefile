#
# Copyright (C) 2019-2020 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-slugify
PKG_VERSION:=4.0.1
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=69a517766e00c1268e5bbfc0d010a0a8508de0b18d30ad5a1ff357f8ae724270

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-slugify
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Slugify application that handles Unicode
  URL:=https://github.com/un33k/python-slugify
  DEPENDS:= \
    +python3-light \
    +python3-codecs \
    +python3-setuptools \
    +python3-text-unidecode
endef

define Package/python3-slugify/description
  A Python slugify application that handles unicode.
endef

$(eval $(call Py3Package,python3-slugify))
$(eval $(call BuildPackage,python3-slugify))
$(eval $(call BuildPackage,python3-slugify-src))
