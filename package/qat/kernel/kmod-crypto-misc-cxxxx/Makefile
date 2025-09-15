#
# Copyright (C) 2019 OpenWrt.org and dl12345@github.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

KMOD_NAME:=crypto-misc-cxxxx
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

define KernelPackage/crypto-misc-cxxxx
  SUBMENU:=$(CRYPTO_MENU)
  HIDDEN:=1
  TITLE:=crypto-misc-cxxxx
  DEPENDS:=kmod-crypto-misc @(TARGET_x86_c2xxx||TARGET_x86_c3xxx)
  DEFAULT:=y if ((TARGET_x86_c2xxx||TARGET_x86_c3xxx) && PACKAGE_kmod-crypto-misc)
  FILES:= \
	$(LINUX_DIR)/arch/x86/crypto/camellia-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/blowfish-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/twofish-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/twofish-x86_64-3way.ko \
	$(LINUX_DIR)/arch/x86/crypto/serpent-sse2-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/camellia-aesni-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/cast5-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/cast6-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/twofish-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/serpent-avx-x86_64.ko \
	$(LINUX_DIR)/arch/x86/crypto/camellia-aesni-avx2.ko \
	$(LINUX_DIR)/arch/x86/crypto/serpent-avx2.ko \
	$(LINUX_DIR)/crypto/ablk_helper.ko@lt4.17
  	AUTOLOAD+= $(call AutoLoad,10,camellia-x86_64 \
	camellia-aesni-avx-x86_64 camellia-aesni-avx2 cast5-avx-x86_64 \
	cast6-avx-x86_64 twofish-x86_64 twofish-x86_64-3way \
	twofish-avx-x86_64 blowfish-x86_64 serpent-avx-x86_64 serpent-avx2)
endef

define KernelPackage/$(KMOD_NAME)/description
 Camellia, Camellia-AESNI, Blowfish, Twofish, Serpent, Serpent SSE2, Serpent AVX2, Cast5, Cast6
endef

$(eval $(call KernelPackage,$(KMOD_NAME)))	

