# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-rockchip-vendor
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/rockchip-linux/rkbin.git
PKG_SOURCE_DATE:=2022-08-01
PKG_SOURCE_VERSION:=b0c100f1a260d807df450019774993c761beb79d
PKG_MIRROR_HASH:=17723ac8f6ec446c759444ee29ba4fe544cebb3785e26d8e10c91c54b9df3f1a

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

define Package/arm-trusted-firmware-rk3568
    $(Package/arm-trusted-firmware-rockchip-vendor)
    DEPENDS:=@TARGET_rockchip_armv8
    VARIANT:=rk3568
endef

define Package/arm-trusted-firmware-rk3588
    $(Package/arm-trusted-firmware-rockchip-vendor)
    DEPENDS:=@TARGET_rockchip_armv8
    VARIANT:=rk3588
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

define Package/arm-trusted-firmware-rk3568/install
endef

define Package/arm-trusted-firmware-rk3588/install
endef

$(eval $(call BuildPackage,arm-trusted-firmware-rk3328))
$(eval $(call BuildPackage,arm-trusted-firmware-rk3399))
$(eval $(call BuildPackage,arm-trusted-firmware-rk3568))
$(eval $(call BuildPackage,arm-trusted-firmware-rk3588))
