#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnftnl
PKG_VERSION:=1.1.8
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://netfilter.org/projects/$(PKG_NAME)/files
PKG_HASH:=04a3fa5b08b736268f7e65836b9f05d9d5f438181467bee3c76c3c4a4f3ab711

PKG_MAINTAINER:=Steven Barth <steven@midlink.org>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

DISABLE_NLS:=

define Package/libnftnl
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libmnl
  TITLE:=Low-level netlink library for the nf_tables subsystem
  URL:=http://www.netfilter.org/projects/libnftnl
  ABI_VERSION:=11
endef

define Package/libnftnl/description
 libnftnl is a userspace library providing a low-level netlink
 programming interface (API) to the in-kernel nf_tables subsystem.
endef

TARGET_CFLAGS += $(FPIC) -flto
TARGET_LDFLAGS += -flto

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libnftnl
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libnftnl/*.h \
		$(1)/usr/include/libnftnl/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnftnl.{so*,a,la} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnftnl.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libnftnl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnftnl.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnftnl))
