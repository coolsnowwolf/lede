#
# Copyright (C) 2011-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=perf
PKG_VERSION:=$(LINUX_VERSION)
PKG_RELEASE:=2

PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_FLAGS:=nonshared

# Perf's makefile and headers are not relocatable and must be built from the
# Linux sources directory
PKG_BUILD_DIR:=$(LINUX_DIR)/tools/perf-$(TARGET_DIR_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/perf
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:= +libelf1 +libdw +(mips||mipsel||powerpc||i386||x86_64||arm||aarch64):libunwind +libpthread +librt +objdump @!LINUX_3_18 @!IN_SDK @!TARGET_arc770 @KERNEL_PERF_EVENTS
  TITLE:=Linux performance monitoring tool
  VERSION:=$(LINUX_VERSION)-$(PKG_RELEASE)
  URL:=http://www.kernel.org
endef

define Package/perf/description
  perf is the Linux performance monitoring tool
endef

define Build/Prepare
	$(CP) $(LINUX_DIR)/tools/perf/* $(PKG_BUILD_DIR)/
endef

MAKE_FLAGS = \
	ARCH="$(LINUX_KARCH)" \
	NO_LIBPERL=1 \
	NO_LIBPYTHON=1 \
	NO_NEWT=1 \
	NO_LZMA=1 \
	NO_BACKTRACE=1 \
	NO_LIBNUMA=1 \
	NO_GTK2=1 \
	NO_LIBAUDIT=1 \
	NO_LIBCRYPTO=1 \
	NO_LIBUNWIND=1 \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	CC="$(TARGET_CC)" \
	LD="$(TARGET_CROSS)ld" \
	CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(if $(findstring c,$(OPENWRT_VERBOSE)),V=1,V='') \
	WERROR=0 \
	prefix=/usr

ifdef CONFIG_LINUX_4_4
ifdef CONFIG_USE_MUSL
 MAKE_FLAGS += EXTRA_CFLAGS="-I$(CURDIR)/musl-include -include $(CURDIR)/musl-compat.h -D__UCLIBC__"
endif
endif

define Build/Compile
	+$(MAKE_FLAGS) $(MAKE) $(PKG_JOBS) \
		-C $(PKG_BUILD_DIR) \
		-f Makefile.perf \
		--no-print-directory
endef

define Package/perf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/perf $(1)/usr/bin/
endef

$(eval $(call BuildPackage,perf))
