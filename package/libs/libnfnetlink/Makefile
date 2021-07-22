#
# Copyright (C) 2007-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnfnetlink
PKG_VERSION:=1.0.1
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://www.netfilter.org/projects/libnfnetlink/files/ \
	ftp://ftp.netfilter.org/pub/libnfnetlink/
PKG_HASH:=f270e19de9127642d2a11589ef2ec97ef90a649a74f56cf9a96306b04817b51a
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0+

PKG_FIXUP:=autoreconf

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libnfnetlink
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A low-level library for netfilter related kernel/userspace communication
  URL:=http://netfilter.org/projects/libnfnetlink/
  ABI_VERSION:=0
endef

define Package/libnfnetlink/description
 libnfnetlink is is the low-level library for netfilter related kernel/userspace communication.
 It provides a generic messaging infrastructure for in-kernel netfilter subsystems
 (such as nfnetlink_log, nfnetlink_queue, nfnetlink_conntrack) and their respective users
 and/or management tools in userspace.
endef

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared

CONFIGURE_VARS += \
	lt_prog_compiler_pic="$(FPIC)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libnfnetlink
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libnfnetlink/{libnfnetlink,linux_nfnetlink,linux_nfnetlink_compat}.h \
		$(1)/usr/include/libnfnetlink/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnfnetlink.{a,so*} \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnfnetlink.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libnfnetlink/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libnfnetlink.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnfnetlink))
