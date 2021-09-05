#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libseccomp
PKG_VERSION:=2.5.1
PKG_RELEASE:=1
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/seccomp/libseccomp/releases/download/v$(PKG_VERSION)/
PKG_HASH:=ee307e383c77aa7995abc5ada544d51c9723ae399768a97667d4cdb3c3a30d55

PKG_MAINTAINER:=Nikos Mavrogiannopoulos <nmav@gnutls.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:libseccomp_project:libseccomp

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=gperf/host
PKG_LIBTOOL_PATHS:=. lib

PKG_CONFIG_DEPENDS:= \
	CONFIG_KERNEL_SECCOMP

include $(INCLUDE_DIR)/package.mk

# This is done instead of DEPENDS:=@!arc to avoid a recursive dependency when
# the library is conditionally selected by util/lxc.
define Package/libseccomp/config
  depends on !arc
endef

define Package/libseccomp/Default
  SUBMENU:=
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=seccomp
  URL:=https://github.com/seccomp/libseccomp/wiki
endef

define Package/libseccomp/Default/description
 The libseccomp library provides an easy to use, platform independent, interface
 to the Linux Kernel's syscall filtering mechanism.  The libseccomp API is
 designed to abstract away the underlying BPF based syscall filter language and
 present a more conventional function-call based filtering interface that should
 be familiar to, and easily adopted by, application developers.
endef

define Package/libseccomp
$(call Package/libseccomp/Default)
  TITLE+= (library)
endef

define Package/scmp_sys_resolver
$(call Package/libseccomp/Default)
  TITLE+= scmp_sys_resolver
  DEPENDS+= libseccomp
endef

define Package/libseccomp/description
 This package contains the seccomp library.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/seccomp*.h \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libseccomp.{a,so*} \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libseccomp.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libseccomp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libseccomp.so.* $(1)/usr/lib/
endef

define Package/scmp_sys_resolver/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/scmp_sys_resolver $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libseccomp))
$(eval $(call BuildPackage,scmp_sys_resolver))
