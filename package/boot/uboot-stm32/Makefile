#
# Copyright (C) 2024 Bootlin
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2025.04
PKG_RELEASE:=1

PKG_HASH:=439d3bef296effd54130be6a731c5b118be7fddd7fcc663ccbc5fb18294d8718
PKG_MAINTAINER:=Thomas Richard <thomas.richard@bootlin.com>

UBOOT_USE_INTREE_DTC:=1

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=stm32
  UBOOT_IMAGE:=u-boot.bin u-boot.dtb
endef

define U-Boot/stm32mp13
  BUILD_SUBTARGET:=stm32mp1
  UBOOT_CONFIG:=stm32mp13
endef

define U-Boot/stm32mp15
  BUILD_SUBTARGET:=stm32mp1
  UBOOT_CONFIG:=stm32mp15
endef

define U-Boot/stm32mp135f-dk
  $(call U-Boot/stm32mp13)
  NAME:=STM32MP135F-DK
  DEVICE_TREE:=stm32mp135f-dk
endef

define U-Boot/stm32mp157c-dk2
  $(call U-Boot/stm32mp15)
  NAME:=STM32MP157C-DK2
  DEVICE_TREE:=stm32mp157c-dk2
endef

define U-Boot/stm32mp157c-dk2-scmi
  $(call U-Boot/stm32mp15)
  NAME:=STM32MP157C-DK2 (SCMI)
  DEVICE_TREE:=stm32mp157c-dk2-scmi
endef

UBOOT_TARGETS := \
		 stm32mp135f-dk \
		 stm32mp157c-dk2 \
		 stm32mp157c-dk2-scmi

UBOOT_MAKE_FLAGS += DEVICE_TREE=$(DEVICE_TREE)

UBOOT_CUSTOMIZE_CONFIG := \
	--disable TOOLS_MKEFICAPSULE

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(foreach img,$(UBOOT_IMAGE), \
		$(CP) $(PKG_BUILD_DIR)/$(img) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(img); \
	)
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
