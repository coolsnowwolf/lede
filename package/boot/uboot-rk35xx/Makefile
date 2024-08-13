#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=uboot-rockchip
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rockchip-linux/u-boot

PKG_SOURCE_DATE:=2023-07-25
PKG_SOURCE_VERSION:=32640b0ada9344f91e7a407576568782907161cd
PKG_MIRROR_HASH:=5aeaf01c7dd48c1f241ecc70de1c50f656e5f5f69b2f0a9618fd9aaf343d0433

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=rockchip
  UENV:=default
  HIDDEN:=1
  DEFAULT:=y
  PKG_BUILD_DEPENDS:=arm-trusted-firmware-rockchip-vendor
endef

# RK3528 boards
define U-Boot/rk3528-evb
  NAME:=RK3528 EVB Board
  DEPENDS:=+PACKAGE_u-boot-$(1):arm-trusted-firmware-rk3528
  ATF:=rk3528_bl31_v1.16.elf
  DDR:=rk3528_ddr_1056MHz_v1.06.bin
  SOC:=rk3528
endef

# RK3588 boards
define U-Boot/rk3588-evb
  NAME:=RK3588 EVB Board
  DEPENDS:=+PACKAGE_u-boot-$(1):arm-trusted-firmware-rk3588
  ATF:=rk3588_bl31_v1.40.elf
  DDR:=rk3588_ddr_lp4_2112MHz_lp5_2736MHz_v1.12.bin
  SOC:=rk3588
endef

UBOOT_TARGETS := rk3528-evb rk3588-evb

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

UBOOT_MAKE_FLAGS += \
  IDB_SOC=$(SOC) \
  TPL_BIN=$(STAGING_DIR_IMAGE)/$(DDR) \
  u-boot.itb idbloader.img

define Build/Configure
	$(call Build/Configure/U-Boot)

	$(SED) 's/CONFIG_TOOLS_LIBCRYPTO=y/# CONFIG_TOOLS_LIBCRYPTO is not set/' $(PKG_BUILD_DIR)/.config
	$(SED) 's#CONFIG_MKIMAGE_DTC_PATH=.*#CONFIG_MKIMAGE_DTC_PATH="$(PKG_BUILD_DIR)/scripts/dtc/dtc"#g' $(PKG_BUILD_DIR)/.config
	echo 'CONFIG_IDENT_STRING=" OpenWrt"' >> $(PKG_BUILD_DIR)/.config
	$(CP) $(STAGING_DIR_IMAGE)/$(ATF) $(PKG_BUILD_DIR)/bl31.elf
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/idbloader.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-idbloader.img
	$(CP) $(PKG_BUILD_DIR)/u-boot.itb $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.itb
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
