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
PKG_SOURCE_URL=https://github.com/apritzel/arm-trusted-firmware
PKG_SOURCE_DATE:=2016-07-12
PKG_SOURCE_VERSION:=87e8aedd80e6448a55b2328768d956fcb5f5d410
PKG_MIRROR_HASH:=4e71a7d4bc0613533854c646b0fa574b18503c0ab28621aac67c70b9827562d8

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
	PLAT=sun50iw1p1 \
	DEBUG=1 \
	bl31

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/build/sun50iw1p1/debug/bl31.bin $(STAGING_DIR_IMAGE)/bl31.bin
endef

define Package/arm-trusted-firmware-sunxi/install
endef

$(eval $(call BuildPackage,arm-trusted-firmware-sunxi))
