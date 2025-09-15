#
# Copyright (C) 2019 OpenWrt.org and dl12345@github.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

KMOD_NAME:=crypto-sha-cxxxx
PKG_NAME:=kmod-$(KMOD_NAME)
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=dl12345 <dl12345@github.com>
PKG_LICENSE:=GPL v3

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

CRYPTO_MENU:=Cryptographic API modules
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

define Build/Prepare
    mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define KernelPackage/crypto-sha1-cxxxx
  SUBMENU:=$(CRYPTO_MENU)
  TITLE:=SHA1 x86_64 versions for C2000/C3000 target
  HIDDEN:=1
  DEPENDS:=kmod-crypto-sha1 @(TARGET_x86_c2xxx||TARGET_x86_c3xxx)
  DEFAULT:=y if ((TARGET_x86_c2xxx||TARGET_x86_c3xxx) && PACKAGE_kmod-crypto-sha1)
  FILES:=$(LINUX_DIR)/arch/x86/crypto/sha1-ssse3.ko
  AUTOLOAD+=$(call AutoLoad,09,sha1-ssse3)
endef

define KernelPackage/crypto-sha256-cxxxx
  SUBMENU:=$(CRYPTO_MENU)
  TITLE:=SHA256 x86_64 versions for C2000/C3000 target
  HIDDEN:=1
  DEPENDS:=kmod-crypto-sha256 @(TARGET_x86_c2xxx||TARGET_x86_c3xxx)
  DEFAULT:=y if ((TARGET_x86_c2xxx||TARGET_x86_c3xxx) && PACKAGE_kmod-crypto-sha256)
  FILES:=$(LINUX_DIR)/arch/x86/crypto/sha256-ssse3.ko
  AUTOLOAD+=$(call AutoLoad,09,sha256-ssse3)
endef

define KernelPackage/crypto-sha512-cxxxx
  SUBMENU:=$(CRYPTO_MENU)
  TITLE:=SHA512 x86_64 versions for C2000/C3000 target
  HIDDEN:=1
  DEPENDS:=kmod-crypto-sha512 @(TARGET_x86_c2xxx||TARGET_x86_c3xxx)
  DEFAULT:=y if ((TARGET_x86_c2xxx||TARGET_x86_c3xxx) && PACKAGE_kmod-crypto-sha512)
  FILES:=$(LINUX_DIR)/arch/x86/crypto/sha512-ssse3.ko
  AUTOLOAD+=$(call AutoLoad,09,sha512-ssse3)
endef

$(eval $(call KernelPackage,crypto-sha1-cxxxx))	
$(eval $(call KernelPackage,crypto-sha256-cxxxx))	
$(eval $(call KernelPackage,crypto-sha512-cxxxx))	

