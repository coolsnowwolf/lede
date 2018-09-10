#
# Copyright (C) 2012-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2017.01
PKG_RELEASE:=3

PKG_HASH:=6c425175f93a4bcf2ec9faf5658ef279633dbd7856a293d95bd1ff516528ecf2

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=omap
  UBOOT_IMAGE:=u-boot.img MLO
  UENV:=default
endef

define U-Boot/omap4_panda
  NAME:=Pandaboard
  BUILD_DEVICES:=ti_omap4-panda
endef

define U-Boot/am335x_boneblack
  NAME:=TI AM335x BeagleBone Black
  BUILD_DEVICES:=ti_am335x-bone-black
endef

define U-Boot/am335x_evm
  NAME:=AM335x EVM
  BUILD_DEVICES:=ti_am335x-evm
endef

define U-Boot/omap3_overo
  NAME:=Gumstix Overo
endef

define U-Boot/omap3_beagle
  NAME:=BeagleBoard
  BUILD_DEVICES:=ti_omap3-beagle
endef

UBOOT_TARGETS:=omap4_panda am335x_evm omap3_overo omap3_beagle am335x_boneblack

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)
	$(CP) $(patsubst %,$(PKG_BUILD_DIR)/%,$(UBOOT_IMAGE)) $(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)/
	$(CP) ./files/uEnv-$(UENV).txt $(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)/uEnv.txt
endef

$(eval $(call BuildPackage/U-Boot))
