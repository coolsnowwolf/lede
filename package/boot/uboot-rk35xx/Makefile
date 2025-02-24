#
# Copyright (C) 2022 ImmortalWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-rk35xx
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/radxa/u-boot

PKG_SOURCE_DATE:=2024-10-29
PKG_SOURCE_VERSION:=27398f1e19628407fabb279034653d23c9369f12
PKG_MIRROR_HASH:=d0469f6c1f0d561d1495844fff2b50bc42af16fa4e33f5f42f092770a2bb4967

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=rockchip
  UENV:=default
  HIDDEN:=1
endef

define U-Boot/evb-rk3528
  BUILD_SUBTARGET:=armv8
  NAME:=RK3528 Evaluation
  BUILD_DEVICES:= \
    armsom_sige1 \
    hinlink_opc-h28k \
    hinlink_opc-h29k \
    hinlink_opc-ht2 \
    radxa_e20c \
    widora_mangopi-m28c \
    widora_mangopi-m28k \
    widora_mangopi-m28k-pro
  DEPENDS:=+PACKAGE_u-boot-evb-rk3528:rkbin-rk3528
  ATF:=rk3528_bl31_v1.17.elf
  DDR:=rk3528_ddr_1056MHz_v1.10.bin
  UBOOT_CONFIG:=rk3528
  SOC:=rk3528
endef

define U-Boot/evb-rk3576
  BUILD_SUBTARGET:=armv8
  NAME:=RK3576 Evaluation
  BUILD_DEVICES:= \
    armsom_sige5
  DEPENDS:=+PACKAGE_u-boot-evb-rk3576:rkbin-rk3576
  ATF:=rk3576_bl31_v1.12.elf
  DDR:=rk3576_ddr_lp4_2112MHz_lp5_2736MHz_v1.08.bin
  UBOOT_CONFIG:=rk3576
  SOC:=rk3576
endef

UBOOT_TARGETS := \
  evb-rk3528 \
  evb-rk3576

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

UBOOT_MAKE_FLAGS += \
  BL31=$(STAGING_DIR_IMAGE)/$(ATF) spl/u-boot-spl.bin u-boot.dtb u-boot.itb

define Build/Configure
	$(call Build/Configure/U-Boot)

	$(SED) 's#CONFIG_MKIMAGE_DTC_PATH=.*#CONFIG_MKIMAGE_DTC_PATH="$(PKG_BUILD_DIR)/scripts/dtc/dtc"#g' $(PKG_BUILD_DIR)/.config
	echo 'CONFIG_IDENT_STRING=" OpenWrt"' >> $(PKG_BUILD_DIR)/.config
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(PKG_BUILD_DIR)/tools/mkimage -n $(SOC) -T rksd -d \
		$(STAGING_DIR_IMAGE)/$(DDR):$(PKG_BUILD_DIR)/spl/u-boot-spl.bin $(PKG_BUILD_DIR)/idbloader.img
	$(CP) $(PKG_BUILD_DIR)/idbloader.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-idbloader.img
	$(CP) $(PKG_BUILD_DIR)/u-boot.itb $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.itb
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
