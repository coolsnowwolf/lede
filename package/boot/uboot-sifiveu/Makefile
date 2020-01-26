# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 OpenWrt.org
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_RELEASE:=1
PKG_VERSION:=2022.10
PKG_HASH:=50b4482a505bc281ba8470c399a3c26e145e29b23500bc35c50debd7fa46bdf8

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=sifiveu
  BUILD_DEVICES=$(1)
  UBOOT_IMAGE:=u-boot.itb
  DTS_DIR:=arch/riscv/dts
  UENV:=default
  DEFAULT:=y
endef

define U-Boot/sifive_unleashed
  NAME:=SiFive Unleashed
  OPENSBI:=generic
  DEPENDS:=+opensbi_generic
  UBOOT_DTS:=hifive-unleashed-a00.dtb
  BUILD_DEVICES:=sifive_unleashed
endef

define U-Boot/sifive_unmatched
  NAME:=SiFive Unmatched
  OPENSBI:=generic
  DEPENDS:=+opensbi_generic
  UBOOT_DTS:=hifive-unmatched-a00.dtb
  BUILD_DEVICES:=sifive_unmatched
endef

UBOOT_TARGETS := \
	sifive_unleashed \
	sifive_unmatched

UBOOT_MAKE_FLAGS += \
        OPENSBI=$(STAGING_DIR_IMAGE)/fw_dynamic-${OPENSBI}.bin

define Build/Configure
	$(call Build/Configure/U-Boot)
	sed -i 's/CONFIG_TOOLS_LIBCRYPTO=y/# CONFIG_TOOLS_LIBCRYPTO is not set/' $(PKG_BUILD_DIR)/.config
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spl/u-boot-spl.bin $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)-spl
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(DTS_DIR)/$(UBOOT_DTS) $(STAGING_DIR_IMAGE)/$(UBOOT_DTS)

	mkimage -C none -A arm -T script -d uEnv-$(UENV).txt \
		$(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-boot.scr
endef

$(eval $(call BuildPackage/U-Boot))
