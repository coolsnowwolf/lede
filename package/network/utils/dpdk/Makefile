# SPDX-License-Identifier: GPL-2.0-only
#

include $(TOPDIR)/rules.mk

PKG_NAME := dpdk
PKG_VERSION := 21.05

PKG_SOURCE := dpdk-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL := http://static.dpdk.org/rel
PKG_MD5SUM := a78bba290b11d9717d1272cc6bfaf7c3

PKG_BUILD_DEPENDS:= meson/host ninja/host
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../../../feeds/packages/meson/meson.mk
include ../../../feeds/packages/ninja/ninja.mk

ifeq ($(CONFIG_LIBC),"glibc")
	PKG_CONFIG := $(PKG_BUILD_DIR)/config/arm/arm_armv7a_linux_glibc_gcc
else
	PKG_CONFIG := $(PKG_BUILD_DIR)/config/arm/arm_armv7a_linux_musl_gcc
endif

TARGET_CFLAGS += -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral

define Package/dpdk
  SECTION    := net
  CATEGORY   := Network
  TITLE      := Data Plane Development Kit (DPDK) 
  DEPENDS    := @(arm) +librt +libpthread +libbpf +libpcap	
endef

define Package/dpdk/description
  DPDK consists of libraries to accelerate packet processing 
  workloads running on a wide variety of CPU architectures. 
endef

define Build/Configure
	$(call Build/Configure/Meson) --cross-file $(PKG_CONFIG)
endef

define Package/dpdk/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dpdk-testpmd $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dpdk-proc-info $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dpdk-test-flow-perf $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,dpdk))
