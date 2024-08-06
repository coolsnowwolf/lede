include $(TOPDIR)/rules.mk

PKG_NAME:=qca-nss-cfi
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-12-15
PKG_SOURCE_URL:=https://git.codelinaro.org/clo/qsdk/oss/lklm/nss-cfi.git
PKG_SOURCE_VERSION:=5cd07ce299ee3ce62dbe4f6783ad36361e57583b
PKG_MIRROR_HASH:=e449eee24fccc09b1cf0f1367bb54cedadcc46a30423934744e78272443197e7

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

ifneq (, $(findstring $(CONFIG_TARGET_SUBTARGET), "ipq807x" "ipq60xx"))
  CFI_OCF_DIR:=ocf/v2.0
  CFI_CRYPTOAPI_DIR:=cryptoapi/v2.0
else
  CFI_CRYPTOAPI_DIR:=cryptoapi/v1.1
  CFI_OCF_DIR:=ocf/v1.0
  CFI_IPSEC_DIR:=ipsec/v1.0
endif

define KernelPackage/qca-nss-cfi-cryptoapi
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Cryptographic API modules
  DEPENDS:=@TARGET_qualcommax +kmod-qca-nss-crypto +kmod-crypto-authenc
  TITLE:=Kernel driver for NSS cfi
  FILES:=$(PKG_BUILD_DIR)/$(CFI_CRYPTOAPI_DIR)/qca-nss-cfi-cryptoapi.ko
  AUTOLOAD:=$(call AutoLoad,59,qca-nss-cfi-cryptoapi)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/qca-nss-cfi
	$(CP) $(PKG_BUILD_DIR)/$(CFI_CRYPTOAPI_DIR)/../exports/* $(1)/usr/include/qca-nss-cfi
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/qca-nss-cfi
endef

define KernelPackage/qca-nss-cfi/Description
This package contains a NSS cfi driver for QCA chipset
endef

EXTRA_CFLAGS+= \
	-DCONFIG_NSS_DEBUG_LEVEL=4 \
	-I$(LINUX_DIR)/crypto/ocf \
	-I$(STAGING_DIR)/usr/include/qca-nss-crypto \
	-I$(STAGING_DIR)/usr/include/crypto \
	-I$(STAGING_DIR)/usr/include/qca-nss-drv

ifneq (, $(findstring $(CONFIG_TARGET_SUBTARGET), "ipq807x"))
EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/qca-nss-clients
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-cfi-cryptoapi),)
MAKE_OPTS+= \
	cryptoapi=y \
	NSS_CRYPTOAPI_ABLK=n \
	NSS_CRYPTOAPI_SKCIPHER=y
endif

ifeq ($(CONFIG_TARGET_BOARD), "qualcommax")
	SOC:=$(CONFIG_TARGET_SUBTARGET)
endif

define Build/Compile
	+$(MAKE) -C "$(LINUX_DIR)" $(strip $(MAKE_OPTS)) \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		CC="$(TARGET_CC)" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) \
		SoC=$(SOC) \
		$(PKG_JOBS) \
		modules
endef

$(eval $(call KernelPackage,qca-nss-cfi-cryptoapi))
