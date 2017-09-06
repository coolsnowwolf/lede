#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnetfilter_queue
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=http://git.netfilter.org/libnetfilter_queue
PKG_SOURCE_DATE:=2016-07-03
PKG_SOURCE_VERSION:=981025e103d887fb6a9c9bb49c74ec323108d098
PKG_MIRROR_HASH:=85c124be0e19162e1ebd1aba6e7e86bb4106e9dab4267baaf66d3ccb48d56e3f

PKG_FIXUP:=autoreconf
PKG_LICENSE:=GPL-2.0+

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libnetfilter-queue
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libmnl +libnfnetlink
  TITLE:=API to the in-kernel connection tracking queue infrastructure
  URL:=http://www.netfilter.org/projects/libnetfilter_queue/
endef

define Package/libnetfilter-queue/description
 libnetfilter_queue is a userspace library providing a programming
 interface (API) to the in-kernel connection tracking state table.
 This library is currently used by conntrack-tools.
endef

TARGET_CFLAGS += $(FPIC) -D_GNU_SOURCE=1

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libnetfilter_queue
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libnetfilter_queue/*.h \
		$(1)/usr/include/libnetfilter_queue/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnetfilter_queue.{so*,a,la} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnetfilter_queue.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libnetfilter-queue/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnetfilter_queue.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnetfilter-queue))
