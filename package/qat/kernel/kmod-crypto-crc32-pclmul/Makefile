
#
# Copyright (C) 2019 OpenWrt.org and dl12345@github.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

KMOD_NAME:=crypto-crc32-pclmul
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

define KernelPackage/crypto-crc32-pclmul
  SUBMENU:=$(CRYPTO_MENU)
  TITLE:=CRC32 hardware acceleration (PCLMULDQ)
  DEPENDS:=+kmod-crypto-hash +kmod-crypto-crc32 @TARGET_x86_64
  KCONFIG:=CONFIG_CRYPTO_CRC32_PCLMUL
  FILES:=$(LINUX_DIR)/arch/x86/crypto/crc32-pclmul.ko
  AUTOLOAD:=$(call AutoLoad,09,crc32-pclmul)
endef

define KernelPackage/$(KMOD_NAME)/description
 From Intel Westmere and AMD Bulldozer processor with SSE4.2 and PCLMULQDQ supported, the processor will support CRC32 PCLMULQDQ 
 implementation using hardware accelerated PCLMULQDQ instruction. This option will create 'crc32-pclmul' module, which will enable 
 any routine to use the CRC-32-IEEE 802.3 checksum and gain better performance as compared with the table implementation
endef

$(eval $(call KernelPackage,$(KMOD_NAME)))	

