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
PKG_SOURCE_URL:=https://github.com/vschagen/mtk-eip93.git
PKG_SOURCE_VERSION:=ca08387bf8352652129019bb19e2423ab313d5cb
PKG_MIRROR_HASH:=4505ee2a175a6cf77a1a3b4fa7bf188da8f8b47894e5afa62c8cb008fbfa15ed

MAKE_PATH:=crypto/mtk-eip93

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
  TITLE:=MTK EIP93 crypto module.
  FILES:=$(PKG_BUILD_DIR)/crypto/mtk-eip93/crypto-hw-eip93.ko
  AUTOLOAD:=$(call AutoProbe,crypto-hw-eip93)
endef

define KernelPackage/crypto-hw-eip93/description
Kernel module to enable EIP-93 Crypto engine as found
in the Mediatek MT7621 SoC.
It enables DES/3DES/AES ECB/CBC/CTR and
IPSEC offload with authenc(hmac(sha1/sha256), aes/cbc/rfc3686)
endef

PKG_EXTRA_KCONFIG:= \
	CONFIG_CRYPTO_DEV_EIP93=m \
	CONFIG_CRYPTO_DEV_EIP93_AEAD=y \
	CONFIG_CRYPTO_DEV_EIP93_AES=y \
	CONFIG_CRYPTO_DEV_EIP93_DES=y \
	CONFIG_CRYPTO_DEV_EIP93_HMAC=y \
	CONFIG_CRYPTO_DEV_EIP93_SKCIPHER=y

PKG_EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(PKG_EXTRA_KCONFIG))))

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)/crypto/mtk-eip93" \
	$(PKG_EXTRA_KCONFIG) \
	EXTRA_CFLAGS="$(PKG_EXTRA_CFLAGS)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,crypto-hw-eip93))
