#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnetfilter_cttimeout
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://www.netfilter.org/projects/libnetfilter_cttimeout/files/ \
	ftp://ftp.netfilter.org/pub/libnetfilter_cttimeout/
PKG_HASH:=aeab12754f557cba3ce2950a2029963d817490df7edb49880008b34d7ff8feba

PKG_FIXUP:=autoreconf
PKG_LICENSE:=GPL-2.0+

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libnetfilter-cttimeout
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libmnl
  TITLE:=API to the in-kernel connection tracking timeout infrastructure
  URL:=http://www.netfilter.org/projects/libnetfilter_cttimeout/
endef

define Package/libnetfilter-cttimeout/description
 libnetfilter_cttimeout is a userspace library providing a programming
 interface (API) to the in-kernel connection tracking timeout handling.
 This library is currently used by conntrack-tools.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libnetfilter_cttimeout
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libnetfilter_cttimeout/*.h \
		$(1)/usr/include/libnetfilter_cttimeout/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnetfilter_cttimeout.{so*,a,la} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnetfilter_cttimeout.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libnetfilter-cttimeout/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnetfilter_cttimeout.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnetfilter-cttimeout))
