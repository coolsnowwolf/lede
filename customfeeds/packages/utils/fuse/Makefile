#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=fuse
PKG_VERSION:=2.9.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/libfuse/libfuse/releases/download/$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=d0e69d5d608cc22ff4843791ad097f554dd32540ddc9bed7638cc6fea7c1b4b5

PKG_MAINTAINER:=
PKG_CPE_ID:=cpe:/a:fuse_project:fuse

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/fuse/Default
  TITLE:=FUSE
  URL:=https://github.com/libfuse/libfuse
endef

define Package/fuse/Default/description
 FUSE (Filesystem in UserSpacE)
endef

define Package/fuse-utils
$(call Package/fuse/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libfuse
  TITLE+= (utilities)
  SUBMENU:=Filesystem
  LICENSE:=GPL-2.0-only
  LICENSE_FILES:=COPYING
endef

define Package/fuse-utils/description
$(call Package/fuse/Default/description)
 This package contains the FUSE utilities.
 - fusermount
 - ulockmgr_server
endef

define Package/libfuse
$(call Package/fuse/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= (library)
  DEPENDS:=+kmod-fuse +libpthread
  SUBMENU:=Filesystem
  ABI_VERSION:=1
  LICENSE:=LGPL-2.1-only
  LICENSE_FILES:=COPYING.LIB
endef

define Package/libfuse/description
$(call Package/fuse/Default/description)
 This package contains the FUSE shared libraries, needed by other programs.
 - libfuse
 - libulockmgr
endef

# generic args
CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--enable-lib \
	--enable-util \
	--disable-rpath \
	--disable-example \
	--disable-mtab

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP)	$(PKG_INSTALL_DIR)/usr/include/{fuse,fuse.h,ulockmgr.h} $(1)/usr/include/
	$(INSTALL_DIR)  $(1)/usr/lib
	$(CP)	$(PKG_INSTALL_DIR)/usr/lib/lib{fuse,ulockmgr}.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR)  $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/fuse.pc $(1)/usr/lib/pkgconfig/
	$(SED) 's,-I$$$${includedir}/fuse,,g' $(1)/usr/lib/pkgconfig/fuse.pc
	$(SED) 's,-L$$$${libdir},,g' $(1)/usr/lib/pkgconfig/fuse.pc
endef

define Package/fuse-utils/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(PKG_INSTALL_DIR)/sbin/mount.fuse $(1)/sbin/
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/{fusermount,ulockmgr_server} $(1)/usr/bin/
endef

define Package/libfuse/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{fuse,ulockmgr}.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libfuse))
$(eval $(call BuildPackage,fuse-utils))
