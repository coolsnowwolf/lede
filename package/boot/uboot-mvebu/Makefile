#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2019.04
PKG_RELEASE:=1

PKG_HASH:=76b7772d156b3ddd7644c8a1736081e55b78828537ff714065d21dbade229bef

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define U-Boot/Default
  BUILD_TARGET:=mvebu
  HIDDEN:=1
endef

define U-Boot/clearfog
  NAME:=SolidRun ClearFog A1
  BUILD_DEVICES:=solidrun_clearfog-base-a1 solidrun_clearfog-pro-a1
  BUILD_SUBTARGET:=cortexa9
  UBOOT_IMAGE:=u-boot-spl.kwb
endef

define U-Boot/uDPU
  NAME:=Methode uDPU
  BUILD_DEVICES:=methode_udpu
  BUILD_SUBTARGET:=cortexa53
endef

UBOOT_TARGETS:= \
	clearfog \
	uDPU

Build/Exports:=$(Host/Exports)

define Build/Configure
	# enable additional options beyond <device>_defconfig
	echo CONFIG_NET_RANDOM_ETHADDR=y >> $(PKG_BUILD_DIR)/configs/$(UBOOT_CONFIG)_defconfig
	echo CONFIG_CMD_SETEXPR=y >> $(PKG_BUILD_DIR)/configs/$(UBOOT_CONFIG)_defconfig

	$(call Build/Configure/U-Boot)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
