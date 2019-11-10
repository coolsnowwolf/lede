#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-layerscape-armv8_32b
PKG_VERSION:=lsdk-1806
PKG_RELEASE:=1

# Layerscape ARMv8 platforms use 64-bit u-boot to support both 32-bit and 64-bit
# kernel/rootfs. Since OpenWrt could only provide 32-bit toolchain for 32-bit targets,
# 64-bit u-boot images git tree is provided here whose source code actually is
# uboot-layerscape's source code.
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/yangbolu1991/u-boot-lede.git
PKG_SOURCE_VERSION:=a2a01facee0918bf724b4f0aec6746e2f1271519
PKG_MIRROR_HASH:=5fcb58c14cdc934793ff315e178ad1d9d2ff55fcaac394b48ec116d1b11ca324

include $(INCLUDE_DIR)/package.mk

define Package/uboot-layerscape-armv8_32b/Config
  define Package/u-boot-$(1)-image
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=$(2)
    DEPENDS:=@TARGET_layerscape_armv8_32b
    CONFIG:=$(3)
    ENV_SIZE:=$(4)
  endef
endef

define Package/uboot-layerscape-armv8_32b/ls1012ardb
  TITLE:=U-Boot image for NXP LS1012ARDB
  CONFIG:=ls1012ardb-uboot.bin
  ENV_SIZE:=0x40000
endef

define Package/uboot-layerscape-armv8_32b/ls1012afrwy
  TITLE:=U-Boot image for NXP LS1012FRWY
  CONFIG:=ls1012afrwy-uboot.bin
  ENV_SIZE:=0x10000
endef

define Package/uboot-layerscape-armv8_32b/ls1043ardb
  TITLE:=U-Boot image for NXP LS1043ARDB
  CONFIG:=ls1043ardb-uboot.bin
  ENV_SIZE:=0x20000
endef

define Package/uboot-layerscape-armv8_32b/ls1046ardb
  TITLE:=U-Boot image for NXP LS1046ARDB
  CONFIG:=ls1046ardb-uboot.bin
  ENV_SIZE:=0x2000
endef

define Build/Compile
endef

define Package/uboot-layerscape-armv8_32b/Install
  define Package/u-boot-$(1)-image/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(CONFIG) $(STAGING_DIR_IMAGE)/
	$(PKG_BUILD_DIR)/tools/mkenvimage -s $(ENV_SIZE) \
		-o $(STAGING_DIR_IMAGE)/$(1)-uboot-env.bin \
		../uboot-layerscape/files/$(1)-uEnv.txt
  endef
endef

UBOOTS := \
  ls1012ardb \
  ls1012afrwy \
  ls1043ardb \
  ls1046ardb

$(foreach u,$(UBOOTS), \
	$(eval $(Package/uboot-layerscape-armv8_32b/$(u))) \
	$(eval $(call Package/uboot-layerscape-armv8_32b/Config,$(u),$(TITLE),$(CONFIG),$(ENV_SIZE))) \
	$(eval $(call Package/uboot-layerscape-armv8_32b/Install,$(u))) \
	$(eval $(call BuildPackage,u-boot-$(u)-image)) \
)
