#
# Copyright (C) 2020 Tony Ambardar <itugrok@yahoo.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bpftools
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/libbpf/bpftool
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-03-08
PKG_SOURCE_VERSION:=04c465fd1f561f67796dc68bbfe1aa7cfa956c3c
PKG_MIRROR_HASH:=e22a954cd186f43228a96586bbdc120b11e6c87360ab88ae96ba37afb9c7cb58
PKG_ABI_VERSION:=$(call abi_version_str,$(PKG_SOURCE_DATE))

PKG_MAINTAINER:=Tony Ambardar <itugrok@yahoo.com>

PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

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
  ABI_VERSION:=$(PKG_ABI_VERSION)
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

ifeq ($(BUILD_VARIANT),full)
  full:=1
else
  full:=0
endif

MAKE_VARS = \
	EXTRA_CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"

MAKE_FLAGS += \
	OUTPUT="$(PKG_BUILD_DIR)/" \
	prefix="/usr" \
	$(if $(findstring c,$(OPENWRT_VERBOSE)),V=1,V='') \
	LIBSUBDIR=lib \
	check_feat=0 \
	feature-clang-bpf-co-re=0 \
	feature-reallocarray=1 \
	feature-zlib=1 \
	feature-libbfd=$(full) \
	feature-libcap=0 \
	feature-disassembler-four-args=$(full)

ifeq ($(BUILD_VARIANT),lib)
  MAKE_PATH = libbpf/src
else
  MAKE_PATH = src
endif

define Build/InstallDev/libbpf
	$(INSTALL_DIR) $(1)/usr/include/bpf
	$(CP) $(PKG_INSTALL_DIR)/usr/include/bpf/*.h $(1)/usr/include/bpf/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbpf.{a,so*} \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libbpf.pc \
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
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbpf.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libbpf))
$(eval $(call BuildPackage,bpftool-full))
$(eval $(call BuildPackage,bpftool-minimal))
