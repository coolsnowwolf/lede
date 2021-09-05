#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=make
PKG_VERSION:=4.3
PKG_RELEASE:=1

PKG_SOURCE_URL:=@GNU/make
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=e05fdde47c5f7ca45cb697e973894ff4f5d79e13b750ed57d7b66d8defc78e19

PKG_MAINTAINER:=Heinrich Schuchardt <xypron.glpk@gmx.de>
PKG_LICENSE:=GPL-3.0-or-later
PKG_CPE_ID:=cpe:/a:gnu:make

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/make
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=make
  URL:=https://www.gnu.org/software/make/
endef

define Package/make/description
  The Make package contains a tool to create executables from source files.
endef

define Package/make/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/make $(1)/usr/bin/
endef

CONFIGURE_VARS += ac_cv_lib_elf_elf_begin=no

# provide gnumake.h at build time for other packages
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/gnumake.h $(1)/usr/include/
endef

$(eval $(call BuildPackage,make))
