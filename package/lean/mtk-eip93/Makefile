#
# Copyright (C) 2006-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtk-eip93
PKG_RELEASE:=1.3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-12-10
PKG_SOURCE_URL:=https://github.com/vschagen/mtk-eip93.git
PKG_SOURCE_VERSION:=bb5fd1c88a8a74de06b70c281bceeb14d0d960ef
PKG_MIRROR_HASH:=d2a4c5da7fb695e575de4b4335b73dcbfea195ef76791133d36287065a66386b

MAKE_PATH:=src

include $(INCLUDE_DIR)/package.mk

define KernelPackage/crypto-hw-eip93
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Cryptographic API modules
  DEPENDS:= \
	@TARGET_ramips_mt7621 \
	+kmod-crypto-authenc \
	+kmod-crypto-des \
	+kmod-crypto-md5 \
	+kmod-crypto-sha1 \
	+kmod-crypto-sha256
  KCONFIG:=
  TITLE:=MTK EIP93 crypto module.
  FILES:=$(PKG_BUILD_DIR)/src/crypto-hw-eip93.ko
  AUTOLOAD:=$(call AutoProbe,crypto-hw-eip93)
endef

define KernelPackage/crypto-hw-eip93/description
Kernel module to enable EIP-93 Crypto engine as found
in the Mediatek MT7621 SoC.
It enables DES/3DES/AES ECB/CBC/CTR and
IPSEC offload with authenc(hmac(sha1/sha256), aes/cbc/rfc3686)
endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)/src"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,crypto-hw-eip93))
