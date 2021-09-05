#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=m4
PKG_VERSION:=1.4.18
PKG_RELEASE:=3

PKG_SOURCE_URL:=@GNU/m4
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=f2c1e86ca0a404ff281631bdc8377638992744b175afb806e25871a24a934e07
PKG_MAINTAINER:=Heinrich Schuchardt <xypron.glpk@gmx.de>
PKG_LICENSE:=GPL-3.0-or-later
PKG_CPE_ID:=cpe:/a:gnu:m4

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/m4
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=m4
  URL:=https://www.gnu.org/software/m4/
endef

define Package/m4/description
  GNU M4 is an implementation of the traditional Unix macro processor.
  It is used by GNU Autoconf and Automake.
endef

define Package/m4/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/m4 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,m4))
