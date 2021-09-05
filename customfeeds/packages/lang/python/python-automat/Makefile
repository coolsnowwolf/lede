#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-automat
PKG_VERSION:=20.2.0
PKG_RELEASE:=2

PYPI_NAME:=Automat
PKG_HASH:=7979803c74610e11ef0c0d68a2942b152df52da55336e0c9d58daf1831cbdf33

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

PYTHON3_PKG_SETUP_VARS:= \
  PKG_VERSION="$(PKG_VERSION)"

define Package/python3-automat
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Concise, idiomatic finite-state automata
  URL:=https://github.com/glyph/Automat
  DEPENDS:= \
      +python3-light \
      +python3-attrs \
      +python3-six
endef

define Package/python3-automat/description
Automat is a library for concise, idiomatic Python expression of
finite-state automata (particularly deterministic finite-state
transducers).
endef

define Py3Package/python3-automat/filespec
+|$(PYTHON3_PKG_DIR)
-|$(PYTHON3_PKG_DIR)/automat/_visualize.py
endef

$(eval $(call Py3Package,python3-automat))
$(eval $(call BuildPackage,python3-automat))
$(eval $(call BuildPackage,python3-automat-src))
