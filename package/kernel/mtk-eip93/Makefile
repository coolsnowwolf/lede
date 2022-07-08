include $(TOPDIR)/rules.mk

PKG_NAME:=mtk-eip93
PKG_RELEASE:=1

PKG_LICENSE:=GPLv2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/vschagen/mtk-eip93.git
PKG_SOURCE_VERSION:=ca08387bf8352652129019bb19e2423ab313d5cb
PKG_MIRROR_HASH:=4505ee2a175a6cf77a1a3b4fa7bf188da8f8b47894e5afa62c8cb008fbfa15ed

MAKE_PATH:=crypto/mtk-eip93

include $(INCLUDE_DIR)/kernel.mk
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
  FILES:=$(PKG_BUILD_DIR)/$(MAKE_PATH)/crypto-hw-eip93.ko
  AUTOLOAD:=$(call AutoProbe,crypto-hw-eip93)
  MENU:=1
endef

define KernelPackage/crypto-hw-eip93/description
Kernel module to enable EIP-93 Crypto engine as found
in the Mediatek MT7621 SoC.
It enables DES/3DES/AES ECB/CBC/CTR and
IPSEC offload with authenc(hmac(sha1/sha256), aes/cbc/rfc3686)
endef

define KernelPackage/crypto-hw-eip93/config
	source "$(SOURCE)/Config.in"
endef

EIP93_EXTRA_CFLAGS:= \
	-D CONFIG_CRYPTO_DEV_EIP93_SKCIPHER=1 \
	-D CONFIG_CRYPTO_DEV_EIP93_HMAC=1 \
	-D CONFIG_CRYPTO_DEV_EIP93_AES=1 \
	-D CONFIG_CRYPTO_DEV_EIP93_DES=1 \
	-D CONFIG_CRYPTO_DEV_EIP93_AEAD=1 \
	-D CONFIG_MTK_EIP93_GENERIC_SW_MAX_LEN=$(CONFIG_MTK_EIP93_GENERIC_SW_MAX_LEN) \
	-D CONFIG_MTK_EIP93_AES_128_SW_MAX_LEN=$(CONFIG_MTK_EIP93_AES_128_SW_MAX_LEN)
EIP93_MAKE_FLAGS:= \
	EXTRA_CFLAGS="$(EIP93_EXTRA_CFLAGS)" \
	CONFIG_CRYPTO_DEV_EIP93=m \
	CONFIG_CRYPTO_DEV_EIP93_SKCIPHER=y \
	CONFIG_CRYPTO_DEV_EIP93_AEAD=y

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		$(EIP93_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)/$(MAKE_PATH)" \
		modules
endef

$(eval $(call KernelPackage,crypto-hw-eip93))
