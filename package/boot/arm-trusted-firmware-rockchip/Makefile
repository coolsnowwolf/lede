#
# Copyright (C) 2020 Tobias Maedel <openwrt@tbspace.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2.12
PKG_RELEASE:=1

PKG_HASH:=b4c047493cac1152203e1ba121ae57267e4899b7bf56eb365e22a933342d31c9

PKG_MAINTAINER:=Tobias Maedel <openwrt@tbspace.de>

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

define Trusted-Firmware-A/rk3588
  BUILD_SUBTARGET:=armv8
  PLAT:=rk3588
endef

TFA_TARGETS:= \
	rk3328 \
	rk3399 \
	rk3568 \
	rk3588

ifeq ($(BUILD_VARIANT),rk3399)
  M0_GCC_NAME:=gcc-arm
  M0_GCC_RELEASE:=11.2-2022.02
  M0_GCC_VERSION:=$(HOST_ARCH)-arm-none-eabi
  M0_GCC_SOURCE:=$(M0_GCC_NAME)-$(M0_GCC_RELEASE)-$(M0_GCC_VERSION).tar.xz

  define Download/m0-gcc
    FILE:=$(M0_GCC_SOURCE)
    URL:=https://developer.arm.com/-/media/Files/downloads/gnu/$(M0_GCC_RELEASE)/binrel
  ifeq ($(HOST_ARCH),aarch64)
    HASH:=ef1d82e5894e3908cb7ed49c5485b5b95deefa32872f79c2b5f6f5447cabf55f
  else
    HASH:=8c5acd5ae567c0100245b0556941c237369f210bceb196edfe5a2e7532c60326
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
