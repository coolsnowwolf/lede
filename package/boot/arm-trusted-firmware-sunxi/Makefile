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
PKG_SOURCE_DATE:=2020-11-17
PKG_SOURCE_VERSION:=e2c509a39c6cc4dda8734e6509cdbe6e3603cdfc
PKG_MIRROR_HASH:=b212d369a5286ebbf6a5616486efa05fa54d4294fd6e9ba2e54fdfae9eda918d

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=license.md

PKG_MAINTAINER:=Hauke Mehrtens <hauke@hauke-m.de>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
	BUILD_TARGET:=sunxi
	BUILD_SUBTARGET:=cortexa53
endef

define Trusted-Firmware-A/sunxi-a64
	NAME:=Allwinner A64
	PLAT:=sun50i_a64
endef

define Trusted-Firmware-A/sunxi-h6
	NAME:=Allwinner H6
	PLAT:=sun50i_h6
endef

TFA_TARGETS:= \
	sunxi-a64 \
	sunxi-h6

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl31.bin $(STAGING_DIR_IMAGE)/bl31_$(BUILD_VARIANT).bin
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
