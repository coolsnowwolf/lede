#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=patch
PKG_VERSION:=2.7.6
PKG_RELEASE:=6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/patch
PKG_HASH:=ac610bda97abe0d9f6b7c963255a11dcb196c25e337c61f94e4778d632f1d8fd

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnu:patch

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/patch
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=patch
  URL:=https://savannah.gnu.org/projects/patch/
  MAINTAINER:=Russell Senior <russell@personaltelco.net>
endef

define Package/patch/description
  The Patch package contains a program for modifying or creating files
  by applying a "patch" file typically created by the diff program.
endef

CONFIGURE_ARGS+= --disable-xattr

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Package/patch/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/patch $(1)/usr/bin/
endef

$(eval $(call BuildPackage,patch))
