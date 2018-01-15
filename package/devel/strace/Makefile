#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=strace
PKG_VERSION:=4.20
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=5bf3148dd17306a42566f7da17368fdd781afa147db05ea63a4ca2b50f58c523

PKG_LICENSE:=BSD-3c
PKG_LICENSE_FILES:=COPYRIGHT
PKG_CPE_ID:=cpe:/a:paul_kranenburg:strace

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_FIXUP:=autoreconf
PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_strace_libunwind
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

HOST_CFLAGS += -I$(LINUX_DIR)/user_headers/include

CONFIGURE_VARS+= \
	LDFLAGS_FOR_BUILD="$(HOST_LDFLAGS)" \
	CPPFLAGS_FOR_BUILD="$(HOST_CPPFLAGS)" \
	CFLAGS_FOR_BUILD="$(HOST_CFLAGS)" \
	CC_FOR_BUILD="$(HOST_CC)"

define Package/strace
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=System call tracer
  DEPENDS:=+PACKAGE_strace_libunwind:libunwind
  URL:=http://strace.sourceforge.net/
endef

define Package/strace/description
A useful diagnostic, instructional, and debugging tool. Allows you to track what
system calls a program makes while it is running.
endef

define Package/strace/config
config PACKAGE_strace_libunwind
	bool "Enable stack tracing support using libunwind (experimental)"
	default n
endef

CONFIGURE_ARGS += --with-libunwind=$(if $(CONFIG_PACKAGE_strace_libunwind),yes,no)
MAKE_FLAGS := \
	CCOPT="$(TARGET_CFLAGS)"

define Package/strace/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/strace $(1)/usr/bin/
endef

$(eval $(call BuildPackage,strace))
