include $(TOPDIR)/rules.mk

PKG_NAME:=qca-nss-dp
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/openwrt/qca-nss-dp.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2025-11-22
PKG_SOURCE_VERSION:=cdedfd891bdf7edd919e51fc3b35c68c860b6815
PKG_MIRROR_HASH:=3045f32c90544a1f020ddd0466dfb2d22fbb830b7ab4b9b6d7a2124c3c77611e

PKG_BUILD_PARALLEL:=1
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-nss-dp
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  DEPENDS:=@TARGET_qualcommax +kmod-qca-ssdk
  TITLE:=Qualcom NSS dataplane ethernet driver
  FILES:=$(PKG_BUILD_DIR)/qca-nss-dp.ko
  AUTOLOAD:=$(call AutoLoad,31,qca-nss-dp,1)
endef

define KernelPackage/qca-nss-dp/Description
  NSS dataplane ethernet driver for Qualcom SoCs.
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include/qca-nss-dp
	$(CP) $(PKG_BUILD_DIR)/exports/* $(1)/usr/include/qca-nss-dp/
endef

EXTRA_CFLAGS+= \
	-I$(STAGING_DIR)/usr/include/qca-ssdk

NSS_DP_HAL_DIR:=$(PKG_BUILD_DIR)/hal
define Build/Configure
	$(CP) $(NSS_DP_HAL_DIR)/soc_ops/$(CONFIG_TARGET_SUBTARGET)/nss_$(CONFIG_TARGET_SUBTARGET).h \
		$(PKG_BUILD_DIR)/exports/nss_dp_arch.h
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		-C "$(LINUX_DIR)" \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		SoC="$(CONFIG_TARGET_SUBTARGET)" \
		modules
endef

$(eval $(call KernelPackage,qca-nss-dp))
