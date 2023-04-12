#
# Copyright (C) 2020 Tobias Maedel <openwrt@tbspace.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-rockchip
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/ARM-software/arm-trusted-firmware
PKG_SOURCE_DATE:=2022-11-17
PKG_SOURCE_VERSION:=0fa7fe59f3a34cb16e390da9bdd290a52005d450
PKG_MIRROR_HASH:=94a92969c1ae06cbedb07d76bc7265692d9f73ec5a7263602902ef4b221886ad

PKG_MAINTAINER:=Tobias Maedel <openwrt@tbspace.de>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
	BUILD_TARGET:=rockchip
	BUILD_SUBTARGET:=armv8
endef

define Trusted-Firmware-A/rk3328
	NAME:=Rockchip RK3328
	PLAT:=rk3328
endef

define Trusted-Firmware-A/rk3399
	NAME:=Rockchip RK3399
	PLAT:=rk3399
endef

define Trusted-Firmware-A/rk3568
	NAME:=Rockchip RK3568
	PLAT:=rk3568
endef

TFA_TARGETS:= \
	rk3328 \
	rk3399 \
    rk3568

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl31/bl31.elf \
		$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)_bl31.elf
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
