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
PKG_SOURCE_DATE:=2021-06-01
PKG_SOURCE_VERSION:=7d631e0d5b2d373b54d4533580d08fb9bd2eaad4
PKG_MIRROR_HASH:=73deb217d7f1dc87bb7a32a1b6855f957aeeb21dca86cdd5840c463683dc0f7d

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
	mkimage -n rk3328 -T rksd -d $(PKG_BUILD_DIR)/bin/rk33/rk3328_ddr_333MHz_v1.17.bin $(PKG_BUILD_DIR)/idbloader.bin
	cat $(PKG_BUILD_DIR)/bin/rk33/rk322xh_miniloader_v2.50.bin >> $(PKG_BUILD_DIR)/idbloader.bin
	$(PKG_BUILD_DIR)/tools/trust_merger --replace bl31.elf $(PKG_BUILD_DIR)/bin/rk33/rk322xh_bl31_v1.46.elf $(PKG_BUILD_DIR)/trust.ini
endef

define Build/InstallDev
	$(INSTALL_DIR) -p $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/bin/rk33/rk322xh_bl31_v1.46.elf $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/tools/loaderimage $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/idbloader.bin $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/trust.bin $(STAGING_DIR_IMAGE)/
endef

define Package/arm-trusted-firmware-rk3328/install
endef

$(eval $(call BuildPackage,arm-trusted-firmware-rk3328))
