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

PKG_SOURCE_DATE:=2024-03-01
PKG_SOURCE_VERSION:=3b2035860d51a75fc70941acf9ae934140991b21
PKG_MIRROR_HASH:=4cba13e74fff830f946ee52d55c6a94e2de691d45db6cf69c481f91a8063e23f

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
    widora_mangopi-m28k \
    widora_mangopi-m28k-pro
  DEPENDS:=+PACKAGE_u-boot-evb-rk3528:rkbin-rk3528
  ATF:=rk3528_bl31_v1.17.elf
  DDR:=rk3528_ddr_1056MHz_v1.09.bin
  UBOOT_CONFIG:=rk3528
  SOC:=rk3528
endef

UBOOT_TARGETS := \
  evb-rk3528

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
