#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnetfilter_cthelper
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://www.netfilter.org/projects/libnetfilter_cthelper/files/ \
	ftp://ftp.netfilter.org/pub/libnetfilter_cthelper/
PKG_HASH:=07618e71c4d9a6b6b3dc1986540486ee310a9838ba754926c7d14a17d8fccf3d

PKG_FIXUP:=autoreconf
PKG_LICENSE:=GPL-2.0+

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libnetfilter-cthelper
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libmnl
  TITLE:=API to the in-kernel connection tracking helper infrastructure
  URL:=http://www.netfilter.org/projects/libnetfilter_cthelper/
endef

define Package/libnetfilter-cthelper/description
 libnetfilter_cthelper is a userspace library providing a programming
 interface (API) to the in-kernel connection tracking helpers. 
 This library is currently used by conntrack-tools.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libnetfilter_cthelper
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libnetfilter_cthelper/*.h \
		$(1)/usr/include/libnetfilter_cthelper/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnetfilter_cthelper.{so*,a,la} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnetfilter_cthelper.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libnetfilter-cthelper/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnetfilter_cthelper.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnetfilter-cthelper))
