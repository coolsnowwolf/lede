#
# Copyright (C) 2021 TDT AG <development@tdt.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See https://www.gnu.org/licenses/gpl-2.0.txt for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libatasmart
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.0pointer.net/libatasmart.git
PKG_SOURCE_DATE:=2012-05-21
PKG_SOURCE_VERSION:=de6258940960443038b4c1651dfda3620075e870
PKG_MIRROR_HASH:=6d2a8782d16e4c1b909e5e836c43c6d58d65b0e1698a53a463a8694a396eb0d7

PKG_MAINTAINER:=Florian Eckert <fe@dev.tdt.de>
PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=LGPL

PKG_REMOVE_FILES:=autogen.sh
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=libatasmart/host

# Do not do autoconf FIXUP for host.
# We only need Host Compiled strpool binary.
HOST_FIXUP:=

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/libatasmart
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=S.M.A.R.T. Reading and Parsing Library
  URL:=https://git.0pointer.net/libatasmart.git
  DEPENDS:= +libudev
endef

define Package/libatasmart/description
  This library is supposed to be lean and small and thus
  supports only a subset of the S.M.A.R.T. functionality.
  However, I claim that it implements the relevant part of it.
  If you need full control over all S.M.A.R.T. functionality of
  your hardware please refer to smartmontools.
endef

define Host/Configure
endef

define Host/Compile
	$(RM) -rf $(HOST_BUILD_DIR)/strpool
	$(HOSTCC) $(HOST_CFLAGS) $(HOST_LDFLAGS) \
		-o $(HOST_BUILD_DIR)/strpool \
		$(HOST_BUILD_DIR)/strpool.c
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/strpool $(STAGING_DIR_HOSTPKG)/bin
endef

define Build/Configure
	$(RM) -rf $(PKG_BUILD_DIR)/strpool
	$(RM) $(PKG_BUILD_DIR)/strpool.c
	$(Build/Configure/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.la \
		$(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h \
		$(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libatasmart/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libatasmart))
