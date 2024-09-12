#
# Copyright (C) 2020-2024 Tony Ambardar <itugrok@yahoo.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bpftool
PKG_VERSION:=7.4.0
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/libbpf/bpftool
PKG_MIRROR_HASH:=18e22f72e67ff402b5ecaf314445f25c40bfe23299cb783b5834a496297c51ed
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=v7.4.0

PKG_MAINTAINER:=Tony Ambardar <itugrok@yahoo.com>

PKG_BUILD_FLAGS:=no-mips16 gc-sections lto
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/host-build.mk

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

ifeq ($(BUILD_VARIANT),full)
  full:=1
else
  full:=0
endif

MAKE_FLAGS += \
	OUTPUT="$(PKG_BUILD_DIR)/" \
	prefix="/usr" \
	$(if $(findstring c,$(OPENWRT_VERBOSE)),V=1,V='') \
	check_feat=0 \
	feature-clang-bpf-co-re=0 \
	feature-libbfd=$(full) \
	feature-llvm=0 \
	feature-libcap=0 \
	feature-disassembler-four-args=1 \
	feature-disassembler-init-styled=1

MAKE_PATH = src

define Package/bpftool-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/bpftool \
		$(1)/usr/libexec/bpftool-$(BUILD_VARIANT)
endef

HOST_MAKE_FLAGS += \
	OUTPUT="$(HOST_BUILD_DIR)/" \
	prefix="/usr" \
	$(if $(findstring c,$(OPENWRT_VERBOSE)),V=1,V='') \
	check_feat=0 \
	feature-clang-bpf-co-re=0 \
	feature-libbfd=1 \
	feature-llvm=0 \
	feature-libcap=0 \
	feature-disassembler-four-args=1 \
	feature-disassembler-init-styled=1

HOST_MAKE_PATH = src

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/usr/sbin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bpftool \
		$(STAGING_DIR_HOST)/usr/sbin/bpftool
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/usr/sbin/bpftool
endef

$(eval $(call BuildPackage,bpftool-full))
$(eval $(call BuildPackage,bpftool-minimal))
$(eval $(call HostBuild))
