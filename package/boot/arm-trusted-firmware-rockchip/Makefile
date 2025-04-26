#
# Copyright (C) 2020 Sarah Maedel <openwrt@tbspace.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2.13
PKG_RELEASE:=1

PKG_HASH:=afb5c408392fcec840bd30de9b02a236b0108142024f9853b542b596b0d894e3

PKG_MAINTAINER:=Sarah Maedel <openwrt@tbspace.de>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  NAME:=Rockchip $(1) SoCs
  BUILD_TARGET:=rockchip
endef

define Trusted-Firmware-A/rk3328
  BUILD_SUBTARGET:=armv8
  PLAT=rk3328
endef

define Trusted-Firmware-A/rk3399
  BUILD_SUBTARGET:=armv8
  PLAT:=rk3399
endef

define Trusted-Firmware-A/rk3568
  BUILD_SUBTARGET:=armv8
  PLAT:=rk3568
endef

define Trusted-Firmware-A/rk3576
  BUILD_SUBTARGET:=armv8
  PLAT:=rk3576
endef

define Trusted-Firmware-A/rk3588
  BUILD_SUBTARGET:=armv8
  PLAT:=rk3588
endef

TFA_TARGETS:= \
	rk3328 \
	rk3399 \
	rk3568 \
	rk3576 \
	rk3588

ifeq ($(BUILD_VARIANT),rk3399)
  M0_GCC_NAME:=arm-gnu-toolchain
  M0_GCC_RELEASE:=12.3.rel1
  M0_GCC_VERSION:=$(HOST_ARCH)-arm-none-eabi
  M0_GCC_SOURCE:=$(M0_GCC_NAME)-$(M0_GCC_RELEASE)-$(M0_GCC_VERSION).tar.xz

  define Download/m0-gcc
    FILE:=$(M0_GCC_SOURCE)
    URL:=https://developer.arm.com/-/media/Files/downloads/gnu/$(M0_GCC_RELEASE)/binrel
  ifeq ($(HOST_ARCH),aarch64)
    HASH:=14c0487d5753f6071d24e568881f7c7e67f80dd83165dec5164b3731394af431
  else
    HASH:=12a2815644318ebcceaf84beabb665d0924b6e79e21048452c5331a56332b309
  endif
  endef

  define Build/Prepare
	$(eval $(call Download,m0-gcc))
	$(call Build/Prepare/Default)

	xzcat $(DL_DIR)/$(M0_GCC_SOURCE) | $(HOST_TAR) -C $(PKG_BUILD_DIR)/ $(TAR_OPTIONS)
  endef

  TFA_MAKE_FLAGS+= \
    M0_CROSS_COMPILE=$(PKG_BUILD_DIR)/$(M0_GCC_NAME)-$(M0_GCC_RELEASE)-$(M0_GCC_VERSION)/bin/arm-none-eabi-
endif

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		OPENSSL_DIR=$(STAGING_DIR_HOST) \
		$(if $(DTC),DTC="$(DTC)") \
		PLAT=$(PLAT) \
		BUILD_STRING="OpenWrt ($(VARIANT))" \
		$(TFA_MAKE_FLAGS)
endef

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl31/bl31.elf $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)_bl31.elf
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
