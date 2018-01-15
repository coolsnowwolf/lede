#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=uboot-layerscape
PKG_SOURCE_DATE:=2017-08-24
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/qoriq-open-source/u-boot.git
PKG_SOURCE_VERSION:=fbedf04c1bad675eadbac86febdcf759441a02af
PKG_MIRROR_HASH:=29922f83ce3e8dde163eafcfd07f3595e2779b7a3e8eb43640f058f58248718d

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=layerscape
  BUILD_SUBTARGET:=armv8_64b
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
endef

define U-Boot/ls1088ardb
  NAME:=NXP ls1088ardb 64b Dev Board
  UBOOT_CONFIG:=ls1088ardb_qspi
endef

define U-Boot/ls2088ardb
  NAME:=NXP ls2088ardb 64b Dev Board
  UBOOT_CONFIG:=ls2080ardb
endef

define U-Boot/ls1012afrdm
  NAME:=NXP ls1012afrdm 64b Dev Board
  UBOOT_CONFIG:=ls1012afrdm_qspi
endef

UBOOT_TARGETS := \
	ls1043ardb \
	ls1046ardb \
	ls1012ardb \
	ls1088ardb \
	ls2088ardb \
	ls1012afrdm

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(BUILD_SUBTARGET)-uboot.bin
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
