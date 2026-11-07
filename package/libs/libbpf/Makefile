#
# Copyright (C) 2020-2024 Tony Ambardar <itugrok@yahoo.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libbpf
PKG_VERSION:=1.7.0

SR1010_UAPI_FIX:=$(if $(and $(filter y,$(CONFIG_TARGET_zte_zx279133)),\
	$(filter y,$(CONFIG_TARGET_zte_zx279133_DEVICE_zte_zxslc-sr1010))),y)
PKG_CONFIG_DEPENDS += CONFIG_TARGET_zte_zx279133 \
	CONFIG_TARGET_zte_zx279133_DEVICE_zte_zxslc-sr1010
PKG_PREPARED_DEPENDS += $(PKG_CONFIG_DEPENDS)
PKG_RELEASE:=$(if $(SR1010_UAPI_FIX),2,1)

PKG_SOURCE_URL:=https://github.com/libbpf/libbpf
PKG_MIRROR_HASH:=c5ffaa91d22172cffb75439919876c1db8a75a82c5a2b8182db34f8867b8cab5
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_ABI_VERSION:=$(firstword $(subst .,$(space),$(PKG_VERSION)))

PKG_MAINTAINER:=Tony Ambardar <itugrok@yahoo.com>
PKG_CPE_ID:=cpe:/a:libbpf_project:libbpf

PKG_BUILD_FLAGS:=no-mips16 no-gc-sections no-lto
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

# Quilt's generic PatchDir fallback treats every top-level entry as a patch.
# Stage only generic *.patch files so SR1010 subdirectories are not included.
ifneq ($(strip $(QUILT)),)
define Build/Patch
	@rm -rf $(PKG_BUILD_DIR)/patches $(PKG_BUILD_DIR)/.patch-source
	@mkdir -p $(PKG_BUILD_DIR)/.patch-source
	@for patch in $(CURDIR)/patches/*.patch; do \
		[ -f "$$$$patch" ] || continue; \
		cp "$$$$patch" $(PKG_BUILD_DIR)/.patch-source/; \
	done
	$(call PatchDir,$(PKG_BUILD_DIR),$(PKG_BUILD_DIR)/.patch-source,)
	@rm -rf $(PKG_BUILD_DIR)/.patch-source
	@touch $(PKG_BUILD_DIR)/.quilt_used
endef
endif

ifneq ($(SR1010_UAPI_FIX),)
define Build/Prepare
	$(call Build/Prepare/Default)
	$(call PatchDir,$(PKG_BUILD_DIR),$(CURDIR)/patches/sr1010,sr1010/)
endef
endif

define Package/libbpf
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libbpf - eBPF helper library
  LICENSE:=LGPL-2.1 OR BSD-2-Clause
  ABI_VERSION:=$(PKG_ABI_VERSION)
  URL:=https://www.kernel.org
  DEPENDS:=+libelf
endef

define Package/libbpf/description
  libbpf is a library for loading eBPF programs and reading and manipulating eBPF objects from user-space.
endef

MAKE_FLAGS += \
	$(if $(findstring c,$(OPENWRT_VERBOSE)),V=1,V='') \
	LIBSUBDIR=lib

MAKE_PATH = src

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

Build/InstallDev=$(Build/InstallDev/libbpf)

define Package/libbpf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbpf.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libbpf))
