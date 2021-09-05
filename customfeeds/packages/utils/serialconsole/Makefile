#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sc
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/serialconsole/sc
PKG_HASH:=2e7a27d588a51173dda3093381604e6f0a16bb04e95b00fd652f37c34c2c6d04

PKG_MAINTAINER:=Stefan Bethke <stb@lassitu.de>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/serialconsole
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Terminal
  TITLE:=Serial Console - minimal terminal program
  URL:=http://sourceforge.net/projects/serialconsole/
endef

define Package/serialconsole/description
serialconsole (sc) is a minimal terminal program allowing to use one machine
to access the serial console of another machine.
endef

define Package/serialconsole/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/bin/sc $(1)/usr/bin/
endef

$(eval $(call BuildPackage,serialconsole))
