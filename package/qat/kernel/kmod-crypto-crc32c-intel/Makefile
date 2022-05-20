#
# Copyright (C) 2019 OpenWrt.org and dl12345@github.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

KMOD_NAME:=crypto-crc32c-intel
PKG_NAME:=kmod-$(KMOD_NAME)
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=dl12345 <dl12345@github.com>
PKG_LICENSE:=GPL v3

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

CRYPTO_MENU:=Cryptographic API modules

define Build/Prepare
    mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define KernelPackage/crypto-crc32c-intel
  SUBMENU:=$(CRYPTO_MENU)
  TITLE:=CRC32c hardware acceleration (SSE4.2)
  DEPENDS:=+kmod-crypto-hash @TARGET_x86_64
  KCONFIG:=CONFIG_CRYPTO_CRC32C_INTEL
  FILES:=$(LINUX_DIR)/arch/x86/crypto/crc32c-intel.ko
  AUTOLOAD:=$(call AutoLoad,09,crc32-intel)  
endef

define KernelPackage/$(KMOD_NAME)/description
 In Intel processor with SSE4.2 supported, the processor will support CRC32C implementation using hardware accelerated CRC32 instruction. 
 This option will create 'crc32c-intel' module, which will enable any routine to use the hardware accelerated CRC32 instruction
 Module will be crc32c-intel
endef

$(eval $(call KernelPackage,$(KMOD_NAME)))	

