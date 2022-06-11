#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-layerscape
PKG_VERSION:=LSDK-20.04-update-290520
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/u-boot
PKG_SOURCE_VERSION:=1e55b2f9e7f56b76569089b9e950f49c1579580e
PKG_MIRROR_HASH:=46aace27e1367f40b424a64215d524a99c3bd62f49057550039e72d5a1ab7edb

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=layerscape
  BUILD_SUBTARGET:=armv8_64b
  BUILD_DEVICES:=$(1)
  UBOOT_IMAGE:=u-boot-dtb.bin
  ENV_SIZE:=0x2000
endef

define U-Boot/fsl_ls1012a-frdm
  NAME:=NXP LS1012AFRDM
  UBOOT_CONFIG:=ls1012afrdm_tfa
  ENV_SIZE:=0x40000
endef

define U-Boot/fsl_ls1012a-rdb
  NAME:=NXP LS1012ARDB
  UBOOT_CONFIG:=ls1012ardb_tfa
  ENV_SIZE:=0x40000
endef

define U-Boot/fsl_ls1012a-frwy-sdboot
  NAME:=NXP LS1012AFRWY
  UBOOT_CONFIG:=ls1012afrwy_tfa
  ENV_SIZE:=0x10000
endef

define U-Boot/fsl_ls1043a-rdb
  NAME:=NXP LS1043ARDB
  UBOOT_CONFIG:=ls1043ardb_tfa
endef

define U-Boot/fsl_ls1043a-rdb-sdboot
  NAME:=NXP LS1043ARDB SD Card Boot
  UBOOT_CONFIG:=ls1043ardb_tfa
endef

define U-Boot/fsl_ls1046a-frwy
  NAME:=NXP LS1046AFRWY
  UBOOT_CONFIG:=ls1046afrwy_tfa
endef

define U-Boot/fsl_ls1046a-frwy-sdboot
  NAME:=NXP LS1046AFRWY SD Card Boot
  UBOOT_CONFIG:=ls1046afrwy_tfa
endef

define U-Boot/fsl_ls1046a-rdb
  NAME:=NXP LS1046ARDB
  UBOOT_CONFIG:=ls1046ardb_tfa
endef

define U-Boot/fsl_ls1046a-rdb-sdboot
  NAME:=NXP LS1046ARDB SD Card Boot
  UBOOT_CONFIG:=ls1046ardb_tfa
endef

define U-Boot/fsl_ls1088a-rdb
  NAME:=NXP LS1088ARDB
  UBOOT_CONFIG:=ls1088ardb_tfa
endef

define U-Boot/fsl_ls1088a-rdb-sdboot
  NAME:=NXP LS1088ARDB SD Card Boot
  UBOOT_CONFIG:=ls1088ardb_tfa
endef

define U-Boot/fsl_ls2088a-rdb
  NAME:=NXP LS2088ARDB
  UBOOT_CONFIG:=ls2088ardb_tfa
endef

define U-Boot/fsl_lx2160a-rdb
  NAME:=NXP LX2160ARDB
  UBOOT_CONFIG:=lx2160ardb_tfa
endef

define U-Boot/fsl_lx2160a-rdb-sdboot
  NAME:=NXP LX2160ARDB SD Card Boot
  UBOOT_CONFIG:=lx2160ardb_tfa
endef

define U-Boot/fsl_ls1021a-twr
  NAME:=NXP LS1021ATWR
  BUILD_SUBTARGET:=armv7
  UBOOT_CONFIG:=ls1021atwr_nor
  ENV_SIZE:=0x20000
endef

define U-Boot/fsl_ls1021a-twr-sdboot
  NAME:=NXP LS1021ATWR SD Card Boot
  BUILD_SUBTARGET:=armv7
  UBOOT_CONFIG:=ls1021atwr_sdcard_ifc
  UBOOT_IMAGE:=u-boot-with-spl-pbl.bin
  ENV_SIZE:=0x20000
endef

define U-Boot/fsl_ls1021a-iot-sdboot
  NAME:=NXP LS1021AIOT SD Card Boot
  BUILD_SUBTARGET:=armv7
  UBOOT_CONFIG:=ls1021aiot_sdcard
  UBOOT_IMAGE:=u-boot-with-spl-pbl.bin
  ENV_SIZE:=0x2000
endef

UBOOT_TARGETS := \
  fsl_ls1012a-frdm \
  fsl_ls1012a-rdb \
  fsl_ls1012a-frwy-sdboot \
  fsl_ls1043a-rdb \
  fsl_ls1043a-rdb-sdboot \
  fsl_ls1046a-frwy \
  fsl_ls1046a-frwy-sdboot \
  fsl_ls1046a-rdb \
  fsl_ls1046a-rdb-sdboot \
  fsl_ls1088a-rdb \
  fsl_ls1088a-rdb-sdboot \
  fsl_ls2088a-rdb \
  fsl_lx2160a-rdb \
  fsl_lx2160a-rdb-sdboot \
  fsl_ls1021a-twr \
  fsl_ls1021a-twr-sdboot \
  fsl_ls1021a-iot-sdboot

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) \
		$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot.bin
	$(PKG_BUILD_DIR)/tools/mkenvimage -s $(ENV_SIZE) \
		-o $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot-env.bin \
		files/$(BUILD_VARIANT)-uEnv.txt
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
