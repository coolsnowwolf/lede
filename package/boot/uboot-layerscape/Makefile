#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-layerscape
PKG_VERSION:=LSDK-20.04
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/u-boot
PKG_SOURCE_VERSION:=b0833bd0e26c059277f0673c9887b967f1ea5837
PKG_MIRROR_HASH:=3cc01d4853ba56d9bcfe5366eddb5f664ae110741146d04f03434d38b4b5b0a0

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=layerscape
  BUILD_SUBTARGET:=armv8_64b
  BUILD_DEVICES:=$(1)
  UBOOT_IMAGE:=u-boot-dtb.bin
  ENV_SIZE:=0x2000
endef

define U-Boot/ls1012afrdm
  NAME:=NXP LS1012AFRDM
  UBOOT_CONFIG:=ls1012afrdm_tfa
  ENV_SIZE:=0x40000
endef

define U-Boot/ls1012ardb
  NAME:=NXP LS1012ARDB
  UBOOT_CONFIG:=ls1012ardb_tfa
  ENV_SIZE:=0x40000
endef

define U-Boot/ls1012afrwy
  NAME:=NXP LS1012AFRWY
  UBOOT_CONFIG:=ls1012afrwy_tfa
  ENV_SIZE:=0x10000
endef

define U-Boot/ls1043ardb
  NAME:=NXP LS1043ARDB
  UBOOT_CONFIG:=ls1043ardb_tfa
endef

define U-Boot/ls1043ardb-sdboot
  NAME:=NXP LS1043ARDB SD Card Boot
  UBOOT_CONFIG:=ls1043ardb_tfa
endef

define U-Boot/ls1046ardb
  NAME:=NXP LS1046ARDB
  UBOOT_CONFIG:=ls1046ardb_tfa
endef

define U-Boot/ls1046ardb-sdboot
  NAME:=NXP LS1046ARDB SD Card Boot
  UBOOT_CONFIG:=ls1046ardb_tfa
endef

define U-Boot/ls1088ardb
  NAME:=NXP LS1088ARDB
  UBOOT_CONFIG:=ls1088ardb_tfa
endef

define U-Boot/ls1088ardb-sdboot
  NAME:=NXP LS1088ARDB SD Card Boot
  UBOOT_CONFIG:=ls1088ardb_tfa
endef

define U-Boot/ls2088ardb
  NAME:=NXP LS2088ARDB
  UBOOT_CONFIG:=ls2088ardb_tfa
endef

define U-Boot/ls1021atwr
  NAME:=NXP LS1021ATWR
  BUILD_SUBTARGET:=armv7
  UBOOT_CONFIG:=ls1021atwr_nor
  ENV_SIZE:=0x20000
endef

define U-Boot/ls1021atwr-sdboot
  NAME:=NXP LS1021ATWR SD Card Boot
  BUILD_SUBTARGET:=armv7
  UBOOT_CONFIG:=ls1021atwr_sdcard_ifc
  UBOOT_IMAGE:=u-boot-with-spl-pbl.bin
  ENV_SIZE:=0x20000
endef

define U-Boot/ls1021aiot-sdboot
  NAME:=NXP LS1021AIOT SD Card Boot
  BUILD_SUBTARGET:=armv7
  UBOOT_CONFIG:=ls1021aiot_sdcard
  UBOOT_IMAGE:=u-boot-with-spl-pbl.bin
  ENV_SIZE:=0x2000
endef

UBOOT_TARGETS := \
  ls1012afrdm \
  ls1012ardb \
  ls1012afrwy \
  ls1043ardb \
  ls1043ardb-sdboot \
  ls1046ardb \
  ls1046ardb-sdboot \
  ls1088ardb \
  ls1088ardb-sdboot \
  ls2088ardb \
  ls1021atwr \
  ls1021atwr-sdboot \
  ls1021aiot-sdboot

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
