include $(TOPDIR)/rules.mk

PKG_NAME:=qca-nss-crypto
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-12-15
PKG_SOURCE_URL:=https://git.codelinaro.org/clo/qsdk/oss/lklm/nss-crypto.git
PKG_SOURCE_VERSION:=3c5a574ce99d7f0b9f892002020f1bf9bfc57a81
PKG_MIRROR_HASH:=ff487c5574481f548eef7b61129fa7be1d83ae285dcc3356a06be237440d8782

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

# v1.0 is for Akronite
# v2.0 is for Hawkeye/Cypress/Maple
ifneq (, $(findstring $(CONFIG_TARGET_SUBTARGET), "ipq807x" "ipq60xx"))
NSS_CRYPTO_DIR:=v2.0
else
NSS_CRYPTO_DIR:=v1.0
endif

define KernelPackage/qca-nss-crypto
	SECTION:=kernel
	CATEGORY:=Kernel modules
	SUBMENU:=Cryptographic API modules
	DEPENDS:=@TARGET_qualcommax +kmod-qca-nss-drv
	TITLE:=Kernel driver for NSS crypto driver
	FILES:=$(PKG_BUILD_DIR)/$(NSS_CRYPTO_DIR)/src/qca-nss-crypto.ko \
	$(PKG_BUILD_DIR)/$(NSS_CRYPTO_DIR)/tool/qca-nss-crypto-tool.ko
	AUTOLOAD:=$(call AutoProbe,qca-nss-crypto)
endef

define KernelPackage/qca-nss-crypto/Description
This package contains a NSS crypto driver for QCA chipset
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/qca-nss-crypto
	$(CP) $(PKG_BUILD_DIR)/$(NSS_CRYPTO_DIR)/include/* $(1)/usr/include/qca-nss-crypto
endef

EXTRA_CFLAGS+= \
	-DCONFIG_NSS_DEBUG_LEVEL=4 \
	-I$(STAGING_DIR)/usr/include/qca-nss-crypto \
	-I$(STAGING_DIR)/usr/include/qca-nss-drv \
	-I$(PKG_BUILD_DIR)/$(NSS_CRYPTO_DIR)/include \
	-I$(PKG_BUILD_DIR)/$(NSS_CRYPTO_DIR)/src

ifeq ($(CONFIG_TARGET_BOARD), "qualcommax")
	SOC:=$(CONFIG_TARGET_SUBTARGET)
endif

define Build/Compile
	+$(MAKE) -C "$(LINUX_DIR)" \
		CC="$(TARGET_CC)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) \
		SoC=$(SOC) \
		$(PKG_JOBS) \
		modules
endef

$(eval $(call KernelPackage,qca-nss-crypto))
