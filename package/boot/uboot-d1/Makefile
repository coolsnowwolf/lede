# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023 OpenWrt.org
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2023.01
PKG_RELEASE:=1

PKG_HASH:=69423bad380f89a0916636e89e6dcbd2e4512d584308d922d1039d1e4331950f

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=d1
  BUILD_SUBTARGET:=generic
  UBOOT_IMAGE:=u-boot-sunxi-with-spl.bin
  UENV:=default
  DTS_DIR:=arch/riscv/dts
endef

define U-Boot/dongshan_nezha_stu
  NAME:=Dongshan Nezha STU
  OPENSBI:=generic
  DEPENDS:=+opensbi_generic
  UBOOT_DTS:=sun20i-d1-dongshan-nezha-stu.dtb
  BUILD_DEVICES:=100ask_dongshan-nezha-stu
endef

define U-Boot/lichee_rv_dock
  NAME:=LicheePi RV (dock)
  OPENSBI:=generic
  DEPENDS:=+opensbi_generic
  UBOOT_DTS:=sun20i-d1-lichee-rv-dock.dtb
  BUILD_DEVICES:=sipeed_lichee-rv-dock
endef

define U-Boot/mangopi_mq_pro
  NAME:=MangoPi MQ Pro
  OPENSBI:=generic
  DEPENDS:=+opensbi_generic
  UBOOT_DTS:=sun20i-d1-mangopi-mq-pro.dtb
  BUILD_DEVICES:=widora_mangopi-mq-pro
endef

define U-Boot/nezha
  NAME:=Nezha D1
  OPENSBI:=generic
  DEPENDS:=+opensbi_generic
  UBOOT_DTS:=sun20i-d1-nezha.dtb
  BUILD_DEVICES:=allwinner_d1-nezha
endef

UBOOT_TARGETS := \
	dongshan_nezha_stu \
	lichee_rv_dock \
	mangopi_mq_pro \
	nezha \

UBOOT_MAKE_FLAGS += \
	OPENSBI=$(STAGING_DIR_IMAGE)/fw_dynamic-${OPENSBI}.bin

define Build/Configure
	$(call Build/Configure/U-Boot)
	sed -i 's/CONFIG_TOOLS_LIBCRYPTO=y/# CONFIG_TOOLS_LIBCRYPTO is not set/' $(PKG_BUILD_DIR)/.config
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(DTS_DIR)/$(UBOOT_DTS) $(STAGING_DIR_IMAGE)/$(UBOOT_DTS)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-$(UBOOT_IMAGE)
	mkimage -C none -A riscv -T script -d uEnv-$(UENV).txt \
		$(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-boot.scr
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
