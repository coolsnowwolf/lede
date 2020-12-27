#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2020.10
PKG_RELEASE:=1

PKG_HASH:=0d481bbdc05c0ee74908ec2f56a6daa53166cc6a78a0e4fac2ac5d025770a622

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

define U-Boot/helios4
  NAME:=Kobol Helios 4
  BUILD_DEVICES:=kobol_helios4
  BUILD_SUBTARGET:=cortexa9
  UBOOT_IMAGE:=u-boot-spl.kwb
endef

define U-Boot/espressobin
  NAME:=Marvell ESPRESSObin
  BUILD_SUBTARGET:=cortexa53
  UBOOT_CONFIG:=mvebu_espressobin-88f3720
endef

define U-Boot/espressobin-emmc
  NAME:=Marvell ESPRESSObin
  BUILD_SUBTARGET:=cortexa53
  UBOOT_CONFIG:=mvebu_espressobin-88f3720
  UBOOT_MAKE_FLAGS+=DEVICE_TREE=armada-3720-espressobin-emmc
endef

define U-Boot/uDPU
  NAME:=Methode uDPU
  BUILD_SUBTARGET:=cortexa53
endef

UBOOT_TARGETS:= \
	clearfog \
	helios4 \
	espressobin \
	espressobin-emmc \
	uDPU

Build/Exports:=$(Host/Exports)

define Build/Configure
	# enable additional options beyond <device>_defconfig
	echo CONFIG_CMD_SETEXPR=y >> $(PKG_BUILD_DIR)/configs/$(UBOOT_CONFIG)_defconfig

	$(call Build/Configure/U-Boot)
endef

define Package/u-boot/install
	$(if $(findstring cortexa53,$(BUILD_SUBTARGET)),,$(Package/u-boot/install/default))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
