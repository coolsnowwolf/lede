#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2026.04
PKG_RELEASE:=1

PKG_HASH:=ac7c04b8b7004923b00a4e5d6699c5df4d21233bac9fda690d8cfbc209fff2fd

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define U-Boot/Default
  BUILD_TARGET:=mvebu
  HIDDEN:=1
endef

define U-Boot/mox
  NAME:=CZ.NIC Turris Mox
  BUILD_DEVICES:=cznic_turris-mox
  BUILD_SUBTARGET:=cortexa53
  UBOOT_CONFIG:=turris_mox
endef

define U-Boot/clearfog
  NAME:=SolidRun ClearFog A1
  BUILD_DEVICES:=solidrun_clearfog-base-a1 solidrun_clearfog-pro-a1
  BUILD_SUBTARGET:=cortexa9
  UBOOT_IMAGE:=u-boot-with-spl.kwb
endef

define U-Boot/helios4
  NAME:=Kobol Helios 4
  BUILD_DEVICES:=kobol_helios4
  BUILD_SUBTARGET:=cortexa9
  UBOOT_IMAGE:=u-boot-with-spl.kwb
endef

define U-Boot/omnia
  NAME:=Turris Omnia
  BUILD_DEVICES:=cznic_turris-omnia
  BUILD_SUBTARGET:=cortexa9
  UBOOT_CONFIG:=turris_omnia
  UBOOT_IMAGE:=u-boot-with-spl.kwb
endef

define U-Boot/espressobin
  NAME:=Marvell ESPRESSObin
  BUILD_SUBTARGET:=cortexa53
  UBOOT_CONFIG:=mvebu_espressobin-88f3720
endef

define U-Boot/uDPU
  NAME:=Methode uDPU
  BUILD_SUBTARGET:=cortexa53
endef

define U-Boot/eDPU
  NAME:=Methode eDPU
  BUILD_SUBTARGET:=cortexa53
endef

define U-Boot/rb5009
  NAME:=MikroTik RB5009
  BUILD_SUBTARGET:=cortexa72
  BUILD_DEVICES:=mikrotik_rb5009ug mikrotik_rb5009upr
  UBOOT_CONFIG:=mvebu_rb5009
  UBOOT_IMAGE:=u-boot.elf
endef

define U-Boot/atlas
  NAME:=RIPE Atlas Probe v5
  BUILD_DEVICES:=ripe_atlas-v5
  BUILD_SUBTARGET:=cortexa53
  UBOOT_CONFIG:=turris_mox
endef

UBOOT_TARGETS:= \
	mox \
	clearfog \
	helios4 \
	omnia \
	espressobin \
	atlas \
	uDPU \
	eDPU \
	rb5009

UBOOT_CUSTOMIZE_CONFIG := \
	--disable TOOLS_MKEFICAPSULE

define Package/u-boot/install
	$(if $(findstring cortexa53,$(BUILD_SUBTARGET)),,$(Package/u-boot/install/default))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
