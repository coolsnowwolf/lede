# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-rockchip-vendor
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/rockchip-linux/rkbin.git
PKG_SOURCE_DATE:=2023-07-26
PKG_SOURCE_VERSION:=b4558da0860ca48bf1a571dd33ccba580b9abe23
PKG_MIRROR_HASH:=039f0f72d0dd179487b5d4b135d13684b220f3d81fa7308a34431a86701f69c6

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

MAKE_PATH:=$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/arm-trusted-firmware-rockchip-vendor
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=ARM Trusted Firmware for Rockchip
endef

define Package/arm-trusted-firmware-rk3328
    $(Package/arm-trusted-firmware-rockchip-vendor)
    DEPENDS:=@TARGET_rockchip_armv8
    VARIANT:=rk3328
endef

define Package/arm-trusted-firmware-rk3399
    $(Package/arm-trusted-firmware-rockchip-vendor)
    DEPENDS:=@TARGET_rockchip_armv8
    VARIANT:=rk3399
endef

define Package/arm-trusted-firmware-rk3566
    $(Package/arm-trusted-firmware-rockchip-vendor)
    DEPENDS:=@TARGET_rockchip_armv8
    VARIANT:=rk3566
endef

define Package/arm-trusted-firmware-rk3568
    $(Package/arm-trusted-firmware-rockchip-vendor)
    DEPENDS:=@TARGET_rockchip_armv8
    VARIANT:=rk3568
endef

define Build/Configure
	$(SED) 's,$$$$(PKG_BUILD_DIR),$(PKG_BUILD_DIR),g' $(PKG_BUILD_DIR)/trust.ini
	$(SED) 's,$$$$(VARIANT),$(BUILD_VARIANT),g' $(PKG_BUILD_DIR)/trust.ini
	$(call Build/Configure/Default)
endef

define Build/Compile
	$(CURDIR)/pack-firmware.sh build $(BUILD_VARIANT) '$(PKG_BUILD_DIR)'
endef

define Build/InstallDev
	$(CURDIR)/pack-firmware.sh install $(BUILD_VARIANT) '$(PKG_BUILD_DIR)' '$(STAGING_DIR_IMAGE)'
endef

define Package/arm-trusted-firmware-rk3328/install
endef

define Package/arm-trusted-firmware-rk3399/install
endef

define Package/arm-trusted-firmware-rk3566/install
endef

define Package/arm-trusted-firmware-rk3568/install
endef

$(eval $(call BuildPackage,arm-trusted-firmware-rk3328))
$(eval $(call BuildPackage,arm-trusted-firmware-rk3399))
$(eval $(call BuildPackage,arm-trusted-firmware-rk3566))
$(eval $(call BuildPackage,arm-trusted-firmware-rk3568))
