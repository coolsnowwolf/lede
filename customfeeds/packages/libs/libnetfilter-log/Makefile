#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnetfilter_log
PKG_VERSION:=1.0.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://www.netfilter.org/projects/libnetfilter_log/files/ \
	ftp://ftp.netfilter.org/pub/libnetfilter_log/
PKG_HASH:=74e0fe75753dba3ac114531b5e73240452c789a3f3adccf5c51217da1d933b21
PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

PKG_FIXUP:=autoreconf
PKG_LICENSE:=GPL-2.0+

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libnetfilter-log
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libnfnetlink +kmod-nfnetlink-log +libmnl
  TITLE:=API to receive to-be-logged packets from the kernel nfnetlink_log subsystem
  URL:=http://www.netfilter.org/projects/libnetfilter_log/
  ABI_VERSION:=1
endef

define Package/libnetfilter-log/description
 libnetfilter_log is a userspace library providing interface to packets that
 have been logged by the kernel packet filter. It is is part of a system that
 deprecates the old syslog/dmesg based packet logging. This library has been
 previously known as libnfnetlink_log.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared \
	--without-ipulog \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libnetfilter_log
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libnetfilter_log/*.h \
		$(1)/usr/include/libnetfilter_log/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnetfilter_log.{so*,a,la} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnetfilter_log.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libnetfilter-log/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnetfilter_log.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnetfilter-log))
