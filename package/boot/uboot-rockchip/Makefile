#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2020.04

PKG_HASH:=fe732aaf037d9cc3c0909bad8362af366ae964bbdac6913a34081ff4ad565372

PKG_MAINTAINER:=Tobias Maedel <openwrt@tbspace.de>

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip

define U-Boot/Default
  BUILD_TARGET:=rockchip
  UENV:=default
  HIDDEN:=1
endef

define U-Boot/nanopi-r2s-rk3328
  BUILD_SUBTARGET:=armv8
  NAME:=NanoPi R2S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2s
  BL31=$(STAGING_DIR_IMAGE)/rk3328_bl31.elf
endef

define U-Boot/rock-pi-e-rk3328
  BUILD_SUBTARGET:=armv8
  NAME:=Rock Pi E
  BUILD_DEVICES:= \
    radxa_rock-pi-e
  BL31=$(STAGING_DIR_IMAGE)/rk3328_bl31.elf
endef

define U-Boot/rockpro64-rk3399
  BUILD_SUBTARGET:=armv8
  NAME:=RockPro64
  BUILD_DEVICES:= \
    pine64_rockpro64
  BL31=$(STAGING_DIR_IMAGE)/rk3399_bl31.elf
endef

UBOOT_TARGETS := \
  nanopi-r2s-rk3328 \
  rock-pi-e-rk3328 \
  rockpro64-rk3399

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

UBOOT_MAKE_FLAGS += \
  BL31=$(BL31)

define Build/Configure
	echo CONFIG_CMD_SETEXPR=y >> $(PKG_BUILD_DIR)/configs/$(UBOOT_CONFIG)_defconfig
	$(call Build/Configure/U-Boot)
	$(SED) 's#CONFIG_MKIMAGE_DTC_PATH=.*#CONFIG_MKIMAGE_DTC_PATH="$(PKG_BUILD_DIR)/scripts/dtc/dtc"#g' $(PKG_BUILD_DIR)/.config
	echo 'CONFIG_IDENT_STRING=" OpenWrt"' >> $(PKG_BUILD_DIR)/.config
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/idbloader.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-idbloader.img
	$(CP) $(PKG_BUILD_DIR)/u-boot.itb $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.itb
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
