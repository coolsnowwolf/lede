#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libredblack
PKG_VERSION:=1.3
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/libredblack
PKG_HASH:=a0ecc59b0aae2df01558a6950532c711a782a099277b439a51d270003092f44f

PKG_MAINTAINER:=Mislav Novakovic <mislav.novakovic@sartura.hr>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/libredblack
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=RedBlack tree library
  URL:=http://libredblack.sourceforge.net/
endef

define Package/libredblack/description
 RedBlack Balanced Tree Searching and Sorting Library.
endef

CONFIGURE_ARGS += --without-rbgen
CONFIGURE_VARS += lt_cv_prog_cc_pic="$(FPIC)"
MAKE_FLAGS += CFLAGS="$(TARGET_CFLAGS)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libredblack.{so*,a,la} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/redblack.h $(1)/usr/include/
endef

define Package/libredblack/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libredblack.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/include/redblack.h $(1)/usr/include
endef

$(eval $(call BuildPackage,libredblack))
