#
# Copyright (C) 2019-2O20 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-markdown
PKG_VERSION:=3.3.3
PKG_RELEASE:=1

PYPI_NAME:=Markdown
PKG_HASH:=5d9f2b5ca24bc4c7a390d22323ca4bad200368612b5aaa7796babf971d2b2f18

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE.md

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-markdown
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Markdown implementation in Python
  URL:=https://python-markdown.github.io/
  DEPENDS:= \
	+python3-light \
	+python3-logging
endef

define Package/python3-markdown/description
  A fast and complete Python implementation of Markdown.
endef

$(eval $(call Py3Package,python3-markdown))
$(eval $(call BuildPackage,python3-markdown))
$(eval $(call BuildPackage,python3-markdown-src))
