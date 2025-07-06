include $(TOPDIR)/rules.mk

PKG_NAME:=qca-nss-dp
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/openwrt/qca-nss-dp.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2025-05-12
PKG_SOURCE_VERSION:=07b87bf513ffe58ce8dd00eb2edf68b5b9bde518
PKG_MIRROR_HASH:=04f2af2dd30db529b5f016da89405cc405ffb0108c945c4b415a794f0036597f

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
