# SPDX-License-Identifier: GPL-2.0-only
#

include $(TOPDIR)/rules.mk

PKG_NAME := dpdk
PKG_VERSION := 21.05

PKG_SOURCE := dpdk-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL := http://static.dpdk.org/rel
PKG_HASH := 1e12499b4c5fcdb57c83e5fe184ea6becddf7cf092893c2c5ef2efb0eec12f0b

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/meson.mk

TARGET_CFLAGS += -D_GNU_SOURCE -Wno-unused-result -Wno-format-nonliteral

define Package/dpdk
  SECTION    := net
  CATEGORY   := Network
  TITLE      := Data Plane Development Kit (DPDK)
  DEPENDS    := @(x86_64||arm||aarch64) +librt +libpthread +libbpf +libpcap +libopenssl
endef

define Package/dpdk/description
  DPDK consists of libraries to accelerate packet processing
  workloads running on a wide variety of CPU architectures.
endef

define Package/dpdk/install
	$(INSTALL_DIR) $(1)/usr/{bin,lib}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{dpdk-testpmd,dpdk-proc-info,dpdk-test-flow-perf} $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,dpdk))