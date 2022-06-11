#
# Copyright (C) 2020 Tony Ambardar <itugrok@yahoo.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bpftools
PKG_VERSION:=5.11.16
PKG_RELEASE:=1

PKG_SOURCE:=linux-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/kernel/v5.x
PKG_HASH:=21163681d130cbce5a6be39019e2c69e44f284855ddd70b1a3bd039249540f43

PKG_MAINTAINER:=Tony Ambardar <itugrok@yahoo.com>

PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

LINUX_VERSION:=$(PKG_VERSION)
LINUX_TLD:=linux-$(LINUX_VERSION)

BPF_FILES:= \
	kernel/bpf scripts tools/Makefile tools/bpf tools/perf/perf-sys.h \
	tools/arch tools/build tools/include tools/lib tools/scripts
TAR_OPTIONS+= \
	--transform="s;$(LINUX_TLD)/;$(PKG_NAME)-$(PKG_VERSION)/;" \
	$(addprefix $(LINUX_TLD)/,$(BPF_FILES))

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/bpftool/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=bpftool - eBPF subsystem utility
  LICENSE:=GPL-2.0-only OR BSD-2-Clause
  URL:=http://www.kernel.org
  DEPENDS:=+libelf
endef

define Package/bpftool-minimal
  $(call Package/bpftool/Default)
  TITLE+= (Minimal)
  VARIANT:=minimal
  DEFAULT_VARIANT:=1
  PROVIDES:=bpftool
  ALTERNATIVES:=200:/usr/sbin/bpftool:/usr/libexec/bpftool-minimal
endef

define Package/bpftool-full
  $(call Package/bpftool/Default)
  TITLE+= (Full)
  VARIANT:=full
  PROVIDES:=bpftool
  ALTERNATIVES:=300:/usr/sbin/bpftool:/usr/libexec/bpftool-full
  DEPENDS+= +libbfd +libopcodes
endef

define Package/bpftool-minimal/description
  A tool for inspection and simple manipulation of eBPF programs and maps.
endef

define Package/bpftool-full/description
  A tool for inspection and simple manipulation of eBPF programs and maps.
  This full version uses libbfd and libopcodes to support disassembly of
  eBPF programs and jited code.
endef

define Package/libbpf
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libbpf - eBPF helper library
  VARIANT:=lib
  LICENSE:=LGPL-2.1 OR BSD-2-Clause
  ABI_VERSION:=0
  URL:=http://www.kernel.org
  DEPENDS:=+libelf
endef

define Package/libbpf/description
  libbpf is a library for loading eBPF programs and reading and manipulating eBPF objects from user-space.
endef


# LTO not compatible with DSO using PIC
ifneq ($(BUILD_VARIANT),lib)
  TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
  TARGET_LDFLAGS += -Wl,--gc-sections
endif

MAKE_VARS = \
	EXTRA_CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

MAKE_FLAGS += \
	BPFTOOL_VERSION="$(LINUX_VERSION)" \
	FEATURES_DUMP="$(PKG_BUILD_DIR)/FEATURE-DUMP.openwrt" \
	OUTPUT="$(PKG_BUILD_DIR)/" \
	prefix="/usr" \
	$(if $(findstring c,$(OPENWRT_VERBOSE)),V=1,V='')

ifeq ($(BUILD_VARIANT),full)
  HAVE_LIBBFD:=1
  HAVE_LIBCAP:=0
  HAVE_CLANG:=0
  MAKE_PATH:=tools/bpf/bpftool
else ifeq ($(BUILD_VARIANT),minimal)
  HAVE_LIBBFD:=0
  HAVE_LIBCAP:=0
  HAVE_CLANG:=0
  MAKE_PATH:=tools/bpf/bpftool
else ifeq ($(BUILD_VARIANT),lib)
  HAVE_LIBBFD:=0
  HAVE_LIBCAP:=0
  HAVE_CLANG:=0
  MAKE_PATH:=tools/lib/bpf
endif

# Perform a "throw-away" make to create a FEATURE-DUMP.* file to edit later.
# The "//" in the make target is actually needed, very unPOSIXly.
define Build/Configure
	+$(MAKE_VARS) $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/tools/bpf/bpftool \
		$(MAKE_FLAGS) FEATURES_DUMP= $(PKG_BUILD_DIR)//libbpf/libbpf.a
	(cd $(PKG_BUILD_DIR); cat FEATURE-DUMP.bpftool libbpf/FEATURE-DUMP.libbpf \
		| sort | uniq > FEATURE-DUMP.openwrt)
	$(SED) 's/feature-libbfd=1/feature-libbfd=$(HAVE_LIBBFD)/' \
		-e 's/feature-libcap=1/feature-libcap=$(HAVE_LIBCAP)/' \
		-e 's/feature-clang-bpf-co-re=1/feature-clang-bpf-co-re=$(HAVE_CLANG)/' \
		$(PKG_BUILD_DIR)/FEATURE-DUMP.openwrt
endef

define Build/InstallDev/libbpf
	$(INSTALL_DIR) $(1)/usr/include/bpf
	$(CP) $(PKG_INSTALL_DIR)/usr/include/bpf/*.h $(1)/usr/include/bpf/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib$(LIB_SUFFIX)/libbpf.{a,so*} \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib$(LIB_SUFFIX)/pkgconfig/libbpf.pc \
		$(1)/usr/lib/pkgconfig/
	$(SED) 's,/usr/include,$$$${prefix}/include,g' \
		$(1)/usr/lib/pkgconfig/libbpf.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' \
		$(1)/usr/lib/pkgconfig/libbpf.pc
endef

ifeq ($(BUILD_VARIANT),lib)
  Build/InstallDev=$(Build/InstallDev/libbpf)
endif

define Package/bpftool-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/bpftool \
		$(1)/usr/libexec/bpftool-$(BUILD_VARIANT)
endef

define Package/libbpf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib$(LIB_SUFFIX)/libbpf.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libbpf))
$(eval $(call BuildPackage,bpftool-full))
$(eval $(call BuildPackage,bpftool-minimal))
