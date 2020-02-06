#
# Copyright (C) 2017 Hauke Mehrtens
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-sunxi
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/ARM-software/arm-trusted-firmware
PKG_SOURCE_DATE:=2018-10-02
PKG_SOURCE_VERSION:=dbc8d9496ead9ecdd7c2a276b542a4fbbbf64027
PKG_MIRROR_HASH:=c81521a27b86f606e927b4e346286540b862828c8d49292eae1f5c6adfc24001

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=license.md

PKG_MAINTAINER:=Hauke Mehrtens <hauke@hauke-m.de>

include $(INCLUDE_DIR)/package.mk


define Package/arm-trusted-firmware-sunxi
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=ARM Trusted Firmware for Allwinner
    DEPENDS:=@TARGET_sunxi_cortexa53
endef

export GCC_HONOUR_COPTS=s

MAKE_VARS = \
	CROSS_COMPILE="$(TARGET_CROSS)"

MAKE_FLAGS += \
	PLAT=sun50i_a64 \
	bl31

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/build/sun50i_a64/release/bl31.bin $(STAGING_DIR_IMAGE)/bl31.bin
endef

define Package/arm-trusted-firmware-sunxi/install
endef

$(eval $(call BuildPackage,arm-trusted-firmware-sunxi))
