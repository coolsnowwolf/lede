include $(TOPDIR)/rules.mk

PKG_NAME:=qca-nss-clients-64
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_URL:=https://source.codeaurora.org/quic/cc-qrdk/oss/lklm/nss-clients
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2021-04-29
PKG_SOURCE_VERSION:=b93c72c1b72c591c2ddc2f0b24f0e2b457720118
PKG_MIRROR_HASH:=fbfba64a364b095ea7c9a24cd7af96b63ab0bc29c179e1628c675aa223c0d063

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-nss-drv-pppoe-64
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - PPPoE
  DEPENDS:=@TARGET_ipq807x +kmod-qca-nss-drv-64 +kmod-ppp +kmod-pppoe
  FILES:=$(PKG_BUILD_DIR)/pppoe/qca-nss-pppoe.ko
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-pppoe)
endef

define KernelPackage/qca-nss-drv-pppoe-64/Description
Kernel modules for NSS connection manager - Support for PPPoE
endef

EXTRA_CFLAGS+= \
	-I$(STAGING_DIR)/usr/include/qca-nss-drv \
	-I$(STAGING_DIR)/usr/include/qca-nss-crypto \
	-I$(STAGING_DIR)/usr/include/qca-nss-cfi \
	-I$(STAGING_DIR)/usr/include/qca-nss-gmac \
	-I$(STAGING_DIR)/usr/include/qca-ssdk \
	-I$(STAGING_DIR)/usr/include/qca-ssdk/fal \
	-I$(STAGING_DIR)/usr/include/nat46

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-pppoe-64),)
NSS_CLIENTS_MAKE_OPTS+=pppoe=y
endif

ifeq ($(CONFIG_TARGET_BOARD), "ipq807x")
    SOC="ipq807x_64"
else ifeq ($(CONFIG_TARGET_BOARD), "ipq60xx")
    SOC="ipq60xx_64"
endif

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" $(strip $(NSS_CLIENTS_MAKE_OPTS)) \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		SoC=$(SOC) \
		$(KERNEL_MAKE_FLAGS) \
		modules
endef

$(eval $(call KernelPackage,qca-nss-drv-pppoe-64))
