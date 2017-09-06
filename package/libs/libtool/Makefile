#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libtool
PKG_VERSION:=2.4
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/libtool
PKG_HASH:=afcce660d3dc54c63a0a5ba3cf05272239dc3c54bbeba20f6bad250f9dc007ae

PKG_LICENSE:=GPL-2.0+
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

CONFIGURE_PREFIX=$(STAGING_DIR_HOSTPKG)
export GLOBAL_LIBDIR=$(STAGING_DIR)/usr/lib

define Package/libltdl
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A generic dynamic object loading library
  URL:=http://www.gnu.org/software/libtool/
endef

define Build/InstallDev
	$(MAKE) -C $(PKG_BUILD_DIR) \
		bindir="$(2)/libltdl/bin" \
		datadir="$(2)/libltdl/share" \
		prefix="$(2)/libltdl" \
		exec_prefix="$(2)/libltdl" \
		install
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	mv $(2)/libltdl/lib/* $(1)/usr/lib/
	mv $(2)/libltdl/include/* $(1)/usr/include/
endef

define Package/libltdl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libltdl/.libs/libltdl.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libltdl))
