#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pip-conf
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/python-pip-conf
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Configuration file for pip/pip3
  URL:=https://pip.pypa.io
  MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
endef

define Package/python-pip-conf/description
 Configuration file for pip/pip3
endef

Build/Compile:=

define Package/python-pip-conf/install
	$(INSTALL_DIR) $(1)/etc
	$(CP) ./files/pip.conf $(1)/etc
endef

$(eval $(call BuildPackage,python-pip-conf))
