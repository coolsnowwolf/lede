#
# Copyright (C) 2019 OpenWrt.org and dl12345@github.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

KMOD_NAME:=crypto-ghash-clmulni
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

define KernelPackage/crypto-ghash-clmulni
  SUBMENU:=$(CRYPTO_MENU)
  TITLE:=GHASH digest hardware acceleration
  DEPENDS:=+kmod-crypto-ghash @TARGET_x86_64
  KCONFIG:=CONFIG_CRYPTO_GHASH_CLMUL_NI_INTEL
  FILES:=$(LINUX_DIR)/arch/x86/crypto/ghash-clmulni-intel.ko
  AUTOLOAD:=$(call AutoLoad,09,ghash-clmulni-intel)
endef

define KernelPackage/$(KMOD_NAME)/description
 GHASH is message digest algorithm for GCM (Galois/Counter Mode). The implementation is accelerated by CLMUL-NI of Intel. 
endef

$(eval $(call KernelPackage,$(KMOD_NAME)))	

