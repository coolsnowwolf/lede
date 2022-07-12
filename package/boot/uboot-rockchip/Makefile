#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information. 
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2022.07
PKG_RELEASE:=$(AUTORELEASE)

PKG_HASH:=92b08eb49c24da14c1adbf70a71ae8f37cc53eeb4230e859ad8b6733d13dcf5e

PKG_MAINTAINER:=Tobias Maedel <openwrt@tbspace.de>

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=rockchip
  UENV:=default
  HIDDEN:=1
endef


# RK3328 boards

define U-Boot/nanopi-r2c-rk3328
  BUILD_SUBTARGET:=armv8
  NAME:=NanoPi R2C
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2c
  DEPENDS:=+PACKAGE_u-boot-nanopi-r2c-rk3328:arm-trusted-firmware-rk3328
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
  ATF:=rk322xh_bl31_v1.46.elf
  OF_PLATDATA:=$(1)
  USE_RKBIN:=1
endef

define U-Boot/nanopi-r2s-rk3328
  BUILD_SUBTARGET:=armv8
  NAME:=NanoPi R2S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2s \
    friendlyarm_nanopi-neo3
  DEPENDS:=+PACKAGE_u-boot-nanopi-r2s-rk3328:arm-trusted-firmware-rk3328
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
  ATF:=rk322xh_bl31_v1.46.elf
  OF_PLATDATA:=$(1)
  USE_RKBIN:=1
endef

define U-Boot/orangepi-r1-plus-rk3328
  BUILD_SUBTARGET:=armv8
  NAME:=Orange Pi R1 Plus
  BUILD_DEVICES:= \
    xunlong_orangepi-r1-plus
  DEPENDS:=+PACKAGE_u-boot-orangepi-r1-plus-rk3328:arm-trusted-firmware-rk3328
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
  ATF:=rk322xh_bl31_v1.46.elf
  OF_PLATDATA:=$(1)
  USE_RKBIN:=1
endef

define U-Boot/orangepi-r1-plus-lts-rk3328
  BUILD_SUBTARGET:=armv8
  NAME:=Orange Pi R1 Plus LTS
  BUILD_DEVICES:= \
    xunlong_orangepi-r1-plus-lts
  DEPENDS:=+PACKAGE_u-boot-orangepi-r1-plus-lts-rk3328:arm-trusted-firmware-rk3328
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
  ATF:=rk322xh_bl31_v1.46.elf
  OF_PLATDATA:=$(1)
  USE_RKBIN:=1
endef

define U-Boot/doornet1-rk3328
  BUILD_SUBTARGET:=armv8
  NAME:=DoorNet1
  BUILD_DEVICES:= \
    embedfire_doornet1
  DEPENDS:=+PACKAGE_u-boot-doornet1-rk3328:arm-trusted-firmware-rk3328
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
  ATF:=rk322xh_bl31_v1.46.elf
  OF_PLATDATA:=$(1)
  USE_RKBIN:=1
endef

# RK3399 boards

define U-Boot/doornet2-rk3399
  BUILD_SUBTARGET:=armv8
  NAME:=DoorNet2
  BUILD_DEVICES:= \
    embedfire_doornet2
  DEPENDS:=+PACKAGE_u-boot-doornet2-rk3399:arm-trusted-firmware-rk3399
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
  ATF:=rk3399_bl31_v1.35.elf
  USE_RKBIN:=1
endef

define U-Boot/guangmiao-g4c-rk3399
  BUILD_SUBTARGET:=armv8
  NAME:=GuangMiao G4C
  BUILD_DEVICES:= \
    sharevdi_guangmiao-g4c
  DEPENDS:=+PACKAGE_u-boot-guangmiao-g4c-rk3399:arm-trusted-firmware-rockchip
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip
  ATF:=rk3399_bl31.elf
endef

define U-Boot/nanopi-r4s-rk3399
  BUILD_SUBTARGET:=armv8
  NAME:=NanoPi R4S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4s
  DEPENDS:=+PACKAGE_u-boot-nanopi-r4s-rk3399:arm-trusted-firmware-rk3399
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
  ATF:=rk3399_bl31_v1.35.elf
  USE_RKBIN:=1
endef

define U-Boot/nanopi-r4se-rk3399
  BUILD_SUBTARGET:=armv8
  NAME:=NanoPi R4SE
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4se
  DEPENDS:=+PACKAGE_u-boot-nanopi-r4se-rk3399:arm-trusted-firmware-rk3399
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
  ATF:=rk3399_bl31_v1.35.elf
  USE_RKBIN:=1
endef

define U-Boot/rock-pi-4-rk3399
  BUILD_SUBTARGET:=armv8
  NAME:=Rock Pi 4
  BUILD_DEVICES:= \
    radxa_rock-pi-4
  DEPENDS:=+PACKAGE_u-boot-rock-pi-4-rk3399:arm-trusted-firmware-rockchip
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip
  ATF:=rk3399_bl31.elf
endef

define U-Boot/rockpro64-rk3399
  BUILD_SUBTARGET:=armv8
  NAME:=RockPro64
  BUILD_DEVICES:= \
    pine64_rockpro64
  DEPENDS:=+PACKAGE_u-boot-rockpro64-rk3399:arm-trusted-firmware-rockchip
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip
  ATF:=rk3399_bl31.elf
endef

UBOOT_TARGETS := \
  doornet2-rk3399 \
  guangmiao-g4c-rk3399 \
  nanopi-r4s-rk3399 \
  nanopi-r4se-rk3399 \
  rock-pi-4-rk3399 \
  rockpro64-rk3399 \
  doornet1-rk3328 \
  nanopi-r2c-rk3328 \
  nanopi-r2s-rk3328 \
  orangepi-r1-plus-rk3328 \
  orangepi-r1-plus-lts-rk3328

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

UBOOT_MAKE_FLAGS += \
  BL31=$(STAGING_DIR_IMAGE)/$(ATF)

define Build/Configure
	$(call Build/Configure/U-Boot)

ifneq ($(OF_PLATDATA),)
	mkdir -p $(PKG_BUILD_DIR)/tpl/dts
	mkdir -p $(PKG_BUILD_DIR)/include/generated

	$(CP) $(PKG_BUILD_DIR)/of-platdata/$(OF_PLATDATA)/dt-plat.c $(PKG_BUILD_DIR)/tpl/dts/dt-plat.c
	$(CP) $(PKG_BUILD_DIR)/of-platdata/$(OF_PLATDATA)/dt-structs-gen.h $(PKG_BUILD_DIR)/include/generated/dt-structs-gen.h
	$(CP) $(PKG_BUILD_DIR)/of-platdata/$(OF_PLATDATA)/dt-decl.h $(PKG_BUILD_DIR)/include/generated/dt-decl.h
endif

	$(SED) 's#CONFIG_MKIMAGE_DTC_PATH=.*#CONFIG_MKIMAGE_DTC_PATH="$(PKG_BUILD_DIR)/scripts/dtc/dtc"#g' $(PKG_BUILD_DIR)/.config
	echo 'CONFIG_IDENT_STRING=" OpenWrt"' >> $(PKG_BUILD_DIR)/.config
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
ifneq ($(USE_RKBIN),)
	$(STAGING_DIR_IMAGE)/loaderimage --pack --uboot $(PKG_BUILD_DIR)/u-boot-dtb.bin $(PKG_BUILD_DIR)/uboot.img 0x200000
	$(CP) $(PKG_BUILD_DIR)/uboot.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot.img
else
	$(CP) $(PKG_BUILD_DIR)/idbloader.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-idbloader.img
	$(CP) $(PKG_BUILD_DIR)/u-boot.itb $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.itb
endif
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
