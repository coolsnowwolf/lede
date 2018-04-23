#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-layerscape-armv8_32b
PKG_SOURCE_DATE:=2018-01-22
PKG_RELEASE:=1

# Layerscape ARMv8 platforms use 64-bit u-boot to support both 32-bit and 64-bit
# kernel/rootfs. Since OpenWrt could only provide 32-bit toolchain for 32-bit targets,
# 64-bit u-boot images git tree is provided here whose source code actually is
# uboot-layerscape's source code.
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/yangbolu1991/u-boot-lede.git
PKG_SOURCE_VERSION:=40a40dfd8136fcc314dd442a3b24a41b1e5652dd
PKG_MIRROR_HASH:=3dde5a33d19c573cbdce74f5d7edb6c5ca2a26a39d503c3efe5d5ad06f01e9f5

PKG_MAINTAINER:=Yangbo Lu <yangbo.lu@nxp.com>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/uboot-layerscape-armv8_32b
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_layerscape_armv8_32b
  TITLE:=U-Boot for ARMv8 32-bit based Layerscape boards
  DEFAULT:=y
  HIDDEN:=1
endef

define Build/Configure

endef

define Build/Compile

endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/ls1043ardb-uboot.bin $(STAGING_DIR_IMAGE)/ls1043ardb-armv8_32b-uboot.bin
	$(CP) $(PKG_BUILD_DIR)/ls1046ardb-uboot.bin $(STAGING_DIR_IMAGE)/ls1046ardb-armv8_32b-uboot.bin
	$(CP) $(PKG_BUILD_DIR)/ls1012ardb-uboot.bin $(STAGING_DIR_IMAGE)/ls1012ardb-armv8_32b-uboot.bin
	$(CP) $(PKG_BUILD_DIR)/ls1012afrdm-uboot.bin $(STAGING_DIR_IMAGE)/ls1012afrdm-armv8_32b-uboot.bin
endef

$(eval $(call BuildPackage,uboot-layerscape-armv8_32b))
