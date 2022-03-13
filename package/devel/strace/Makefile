#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=strace
PKG_VERSION:=5.16
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://strace.io/files/$(PKG_VERSION)
PKG_HASH:=dc7db230ff3e57c249830ba94acab2b862da1fcaac55417e9b85041a833ca285

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:paul_kranenburg:strace

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_CONFIG_DEPENDS := \
	CONFIG_STRACE_LIBDW \
	CONFIG_STRACE_LIBUNWIND

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
  URL:=https://strace.io/
  DEPENDS:=+STRACE_LIBDW:libdw +STRACE_LIBUNWIND:libunwind
endef

define Package/strace/description
  A useful diagnostic, instructional, and debugging tool. Allows you to track what
  system calls a program makes while it is running.
endef

define Package/strace/config
choice
	prompt "stack tracing support"
	default STRACE_NONE

	config STRACE_NONE
		bool "None"

	config STRACE_LIBDW
		bool "libdw"

	config STRACE_LIBUNWIND
		bool "libunwind (experimental)"
endchoice
endef

CONFIGURE_ARGS += \
	--with-libdw=$(if $(CONFIG_STRACE_LIBDW),yes,no) \
	--with-libunwind=$(if $(CONFIG_STRACE_LIBUNWIND),yes,no) \
	--enable-mpers=no \
	--without-libselinux

MAKE_FLAGS := \
	CCOPT="$(TARGET_CFLAGS)"

define Package/strace/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/strace $(1)/usr/bin/
endef

$(eval $(call BuildPackage,strace))
