#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2022.01
PKG_RELEASE:=$(AUTORELEASE)

PKG_HASH:=81b4543227db228c03f8a1bf5ddbc813b0bb8f6555ce46064ef721a6fc680413

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

define U-Boot/omnia
  NAME:=Turris Omnia
  BUILD_DEVICES:=cznic_turris-omnia
  BUILD_SUBTARGET:=cortexa9
  UBOOT_CONFIG:=turris_omnia
  UBOOT_IMAGE:=u-boot-spl.kwb
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

UBOOT_TARGETS:= \
	clearfog \
	helios4 \
	omnia \
	espressobin \
	uDPU

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
