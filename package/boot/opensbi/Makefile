# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=opensbi
PKG_RELEASE:=1.6

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/riscv/opensbi
PKG_SOURCE_DATE:=2024-12-24
PKG_SOURCE_VERSION:=bd613dd92113f683052acfb23d9dc8ba60029e0a
PKG_MIRROR_HASH:=7897ee3c77c7d205278cbb87eabc92056ea73d3ea45ca974cf3500ef33399d96

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_TARGETS:=bin
PKG_FLAGS:=nonshared
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=COPYING.BSD
PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>

include $(INCLUDE_DIR)/package.mk

define Package/opensbi
    SECTION:=boot
    CATEGORY:=Boot Loaders
    DEPENDS:=@(TARGET_sifiveu||TARGET_allwinner)
    URL:=https://github.com/riscv/opensbi/blob/master/README.md
    VARIANT:=$(subst _,/,$(subst opensbi_,,$(1)))
    TITLE:=OpenSBI generic
    OPENSBI_IMAGE:=
    PLAT:=
endef

define Package/opensbi_generic
  $(Package/opensbi)
  TITLE:=OpenSBI generic
  OPENSBI_IMAGE:=fw_dynamic.bin
  PLAT:=generic
endef

export GCC_HONOUR_COPTS=s

MAKE_VARS = \
	CROSS_COMPILE="$(TARGET_CROSS)"

define Build/Compile
	$(eval $(Package/opensbi_$(BUILD_VARIANT))) \
		+$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR) \
		PLATFORM=$(PLAT)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/build/platform/$(PLAT)/firmware/fw_dynamic.bin $(STAGING_DIR_IMAGE)/fw_dynamic-${BUILD_VARIANT}.bin
endef

$(eval $(call BuildPackage,opensbi_generic))
