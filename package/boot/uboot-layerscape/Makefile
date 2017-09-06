#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=uboot-layerscape
PKG_VERSION:=v1.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=462fea9bdf03ec4bc23f2f1bb3917c2ec57017eca1e11b8a609bf60cd51a94e7
PKG_SOURCE_URL:=https://github.com/fsl-jyt/u-boot.git
PKG_SOURCE_VERSION:=69b5ea576a2b0448391a910114d6c05859bf78d0

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=layerscape
  BUILD_SUBTARGET:=64b
  BUILD_DEVICES:=$(1)
  UBOOT_IMAGE:=u-boot-dtb.bin
  HIDDEN:=y
endef

define U-Boot/ls1043ardb
  NAME:=NXP ls1043ardb 64b Dev Board
endef

define U-Boot/ls1046ardb
  NAME:=NXP ls1046ardb 64b Dev Board
  UBOOT_CONFIG:=ls1046ardb_qspi
endef

define U-Boot/ls1012ardb
  NAME:=NXP ls1012ardb 64b Dev Board
  UBOOT_CONFIG:=ls1012ardb_qspi
  UBOOT_IMAGE:=u-boot-swap.bin
endef

UBOOT_TARGETS := \
	ls1043ardb \
	ls1046ardb \
	ls1012ardb

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-64b-uboot.bin
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
