#
# Copyright (C) 2019 OpenWrt.org and dl12345@github.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

KMOD_NAME:=crypto-crct10dif-pclmu
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

define KernelPackage/$(KMOD_NAME)
  SUBMENU:=$(CRYPTO_MENU)
  TITLE:=CRCT10DIF hardware acceleration
  DEPENDS:=+kmod-crypto-hash @TARGET_x86_64
  KCONFIG:=CONFIG_CRYPTO_CRCT10DIF_PCLMUL
  FILES:=$(LINUX_DIR)/arch/x86/crypto/crct10dif-pclmul.ko
  AUTOLOAD:=$(call AutoLoad,09,crct10dif-pclmul)
endef

define KernelPackage/$(KMOD_NAME)/description
 CRC T10 Data Integrity Field computation is being cast as a crypto transform. 
 This allows for faster crc t10 diff transforms to be used if they are available.
endef

$(eval $(call KernelPackage,$(KMOD_NAME)))	

