#
# Copyright (C) 2021 ImmortalWrt
# (https://immortalwrt.org)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-rk3328
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/rockchip-linux/rkbin.git
PKG_SOURCE_DATE:=2020-10-30
PKG_SOURCE_VERSION:=0bb1c512492386a72a3a0b5a0e18e49c636577b9
PKG_MIRROR_HASH:=6dffe95b83f37a280f98c105fce529f45e39ce333b24709a9645aac1352457ce

PKG_MAINTAINER:=AmadeusGhost <amadeus@immortalwrt.org>

MAKE_PATH:=$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/arm-trusted-firmware-rk3328
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=ARM Trusted Firmware for Rockchip
    DEPENDS:=@TARGET_rockchip_armv8
endef

define Build/Configure
	$(SED) 's,$$$$(PKG_BUILD_DIR),$(PKG_BUILD_DIR),g' $(PKG_BUILD_DIR)/trust.ini
	$(call Build/Configure/Default)
endef

define Build/Compile
	mkimage -n rk3328 -T rksd -d $(PKG_BUILD_DIR)/bin/rk33/rk3328_ddr_333MHz_v1.16.bin $(PKG_BUILD_DIR)/idbloader.bin
	cat $(PKG_BUILD_DIR)/bin/rk33/rk322xh_miniloader_v2.50.bin >> $(PKG_BUILD_DIR)/idbloader.bin
	$(PKG_BUILD_DIR)/tools/trust_merger --replace bl31.elf $(PKG_BUILD_DIR)/bin/rk33/rk322xh_bl31_v1.44.elf $(PKG_BUILD_DIR)/trust.ini
endef

define Build/InstallDev
	$(INSTALL_DIR) -p $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/bin/rk33/rk322xh_bl31_v1.44.elf $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/tools/loaderimage $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/idbloader.bin $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/trust.bin $(STAGING_DIR_IMAGE)/
endef

define Package/arm-trusted-firmware-rk3328/install
endef

$(eval $(call BuildPackage,arm-trusted-firmware-rk3328))
