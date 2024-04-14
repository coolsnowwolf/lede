include $(TOPDIR)/rules.mk

PKG_NAME:=qca-ssdk
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://git.codelinaro.org/clo/qsdk/oss/lklm/qca-ssdk.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2023-06-06
PKG_SOURCE_VERSION:=74caf88aa3b6793c300f676e4fb1c62da7507be9
PKG_MIRROR_HASH:=6bdb90919b773f5fb432c8b374c9419feac32ba6583ad82dfec5e41628a32dd9

PKG_FLAGS:=nonshared
PKG_BUILD_FLAGS:=no-lto

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-ssdk
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Qualcom SSDK switch driver
  DEPENDS:=@(TARGET_qualcommax)
  FILES:=$(PKG_BUILD_DIR)/build/bin/qca-ssdk.ko
  AUTOLOAD:=$(call AutoLoad,30,qca-ssdk)
endef

define KernelPackage/qca-ssdk/Description
  Driver for Qualcomm Atheros switches.
endef

GCC_VERSION=$(shell echo "$(CONFIG_GCC_VERSION)" | sed 's/[^0-9.]*\([0-9.]*\).*/\1/')

LNX_CONFIG_OPTS = LNX_MAKEOPTS='$(KERNEL_MAKEOPTS)' MODULE_TYPE=KSLIB modules

MAKE_FLAGS+= \
	TARGET_NAME=$(CONFIG_TARGET_NAME) \
	TOOL_PATH=$(TOOLCHAIN_DIR)/bin \
	SYS_PATH=$(LINUX_DIR) \
	TOOLPREFIX=$(TARGET_CROSS) \
	KVER=$(LINUX_VERSION) \
	ARCH=$(LINUX_KARCH) \
	TARGET_SUFFIX=$(CONFIG_TARGET_SUFFIX) \
	GCC_VERSION=$(GCC_VERSION) \
	EXTRA_CFLAGS=-fno-stack-protector -I$(STAGING_DIR)/usr/include \
	PTP_FEATURE=disable SWCONFIG_FEATURE=disable \
	$(LNX_CONFIG_OPTS)

ifeq ($(CONFIG_TARGET_SUBTARGET), "ipq60xx")
    MAKE_FLAGS+= CHIP_TYPE=CPPE
else ifeq ($(CONFIG_TARGET_SUBTARGET), "ipq807x")
    MAKE_FLAGS+= CHIP_TYPE=HPPE
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/api
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/ref
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/fal
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/sal
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/init
	$(CP) -rf $(PKG_BUILD_DIR)/include/api/sw_ioctl.h $(1)/usr/include/qca-ssdk/api
	if [ -f $(PKG_BUILD_DIR)/include/ref/ref_vsi.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/ref/ref_vsi.h $(1)/usr/include/qca-ssdk/ref/; \
	fi
	if [ -f $(PKG_BUILD_DIR)/include/ref/ref_fdb.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/ref/ref_fdb.h $(1)/usr/include/qca-ssdk/ref/; \
	fi
	if [ -f $(PKG_BUILD_DIR)/include/ref/ref_port_ctrl.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/ref/ref_port_ctrl.h $(1)/usr/include/qca-ssdk/ref/; \
	fi
	if [ -f $(PKG_BUILD_DIR)/include/init/ssdk_init.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/init/ssdk_init.h $(1)/usr/include/qca-ssdk/init/; \
	fi
	if [ -f $(PKG_BUILD_DIR)/include/init/ssdk_netlink.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/init/ssdk_netlink.h $(1)/usr/include/qca-ssdk/init/; \
	fi
	$(CP) -rf $(PKG_BUILD_DIR)/include/fal $(1)/usr/include/qca-ssdk
	$(CP) -rf $(PKG_BUILD_DIR)/include/common/*.h $(1)/usr/include/qca-ssdk
	$(CP) -rf $(PKG_BUILD_DIR)/include/sal/os/linux/*.h $(1)/usr/include/qca-ssdk
	$(CP) -rf $(PKG_BUILD_DIR)/include/sal/os/*.h $(1)/usr/include/qca-ssdk

endef

$(eval $(call KernelPackage,qca-ssdk))
