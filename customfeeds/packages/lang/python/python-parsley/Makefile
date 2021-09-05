#
# Copyright (C) 2015, 2017-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-parsley
PKG_VERSION:=1.3
PKG_RELEASE:=5

PYPI_NAME:=Parsley
PKG_HASH:=9444278d47161d5f2be76a767809a3cbe6db4db822f46a4fd7481d4057208d41

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

PYTHON3_PKG_SETUP_ARGS:=

define Package/python3-parsley
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Parsing and pattern matching
  URL:=http://launchpad.net/parsley
  DEPENDS:=+python3-light
endef

define Package/python3-parsley/description
Parsing and pattern matching made easy.
endef

$(eval $(call Py3Package,python3-parsley))
$(eval $(call BuildPackage,python3-parsley))
$(eval $(call BuildPackage,python3-parsley-src))
