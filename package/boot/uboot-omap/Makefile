#
# Copyright (C) 2012-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2019.10
PKG_RELEASE:=1

PKG_HASH:=8d6d6070739522dd236cba7055b8736bfe92b4fac0ea18ad809829ca79667014

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=omap
  UBOOT_IMAGE:=u-boot.img MLO
endef

define U-Boot/omap4_panda
  NAME:=Pandaboard
  BUILD_DEVICES:=ti_omap4-panda
endef

define U-Boot/am335x_evm
  NAME:=AM335x EVM
  BUILD_DEVICES:=ti_am335x-evm ti_am335x-bone-black
endef

define U-Boot/omap3_overo
  NAME:=Gumstix Overo
endef

define U-Boot/omap3_beagle
  NAME:=BeagleBoard
  BUILD_DEVICES:=ti_omap3-beagle
endef

UBOOT_TARGETS:=omap4_panda am335x_evm omap3_overo omap3_beagle

define Build/InstallDev
	$(foreach device,$(BUILD_DEVICES), \
		$(INSTALL_DIR) $(STAGING_DIR_IMAGE)/$(device)
	)
	$(foreach device,$(BUILD_DEVICES), \
		$(CP) $(patsubst %,$(PKG_BUILD_DIR)/%,$(UBOOT_IMAGE)) $(STAGING_DIR_IMAGE)/$(device)/
	)
	$(foreach device,$(BUILD_DEVICES), \
		mkimage -C none -A arm -T script -d ./files/boot.scr.txt \
			$(STAGING_DIR_IMAGE)/$(device)/boot.scr
	)
endef

$(eval $(call BuildPackage/U-Boot))
