include $(TOPDIR)/rules.mk

PKG_NAME:=qca-ssdk
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/openwrt/qca-ssdk.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2025-05-12
PKG_SOURCE_VERSION:=709382c7aefbd07643db24edb514322f078c5181
PKG_MIRROR_HASH:=8422416edbc40b1cd37c824b834edadca7ec480b5b41fb3188481b0580ebef50

PKG_FLAGS:=nonshared
PKG_BUILD_PARALLEL:=1
PKG_BUILD_FLAGS:=no-lto

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-ssdk
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Qualcom SSDK switch driver
  DEPENDS:=@(TARGET_qualcommax)
  FILES:=$(PKG_BUILD_DIR)/qca-ssdk.ko
  AUTOLOAD:=$(call AutoLoad,30,qca-ssdk)
endef

define KernelPackage/qca-ssdk/Description
  Driver for Qualcomm Atheros switches.
endef

GCC_VERSION=$(shell echo "$(CONFIG_GCC_VERSION)" | sed 's/[^0-9.]*\([0-9.]*\).*/\1/')

LNX_CONFIG_OPTS = LNX_MAKEOPTS='$(KERNEL_MAKEOPTS)' PRJ_PATH=$(PKG_BUILD_DIR) MODULE_TYPE=KSLIB modules

EXTRA_CFLAGS:= \
	-fno-stack-protector \
	-Wno-missing-prototypes \
	-Wno-missing-declarations \
	-I$(STAGING_DIR)/usr/include

MAKE_FLAGS+= \
	TARGET_NAME=$(CONFIG_TARGET_NAME) \
	TOOL_PATH=$(firstword $(TOOLCHAIN_BIN_DIRS)) \
	SYS_PATH=$(LINUX_DIR) \
	TOOLPREFIX=$(TARGET_CROSS) \
	KVER=$(LINUX_VERSION) \
	ARCH=$(LINUX_KARCH) \
	TARGET_SUFFIX=$(CONFIG_TARGET_SUFFIX) \
	GCC_VERSION=$(GCC_VERSION) \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	SoC=$(CONFIG_TARGET_SUBTARGET) \
	SHELL="$(BASH)" \
	PTP_FEATURE=disable SWCONFIG_FEATURE=disable \
	ISISC_ENABLE=disable MHT_ENABLE=disable \
	IN_QCA803X_PHY=FALSE IN_QCA808X_PHY=FALSE \
	IN_MALIBU_PHY=FALSE IN_MP_PHY=FALSE \
	$(LNX_CONFIG_OPTS)

ifeq ($(CONFIG_TARGET_SUBTARGET), "ipq807x")
    MAKE_FLAGS+= CHIP_TYPE=HPPE
endif

ifeq ($(CONFIG_TARGET_SUBTARGET), "ipq60xx")
    MAKE_FLAGS+= CHIP_TYPE=CPPE \
                 IN_AQUANTIA_PHY=TRUE
endif

ifeq ($(CONFIG_TARGET_SUBTARGET), "ipq50xx")
    MAKE_FLAGS+= CHIP_TYPE=MP
endif

define Build/Compile
	+$(MAKE) $(PKG_JOBS) $(MAKE_FLAGS) -C $(PKG_BUILD_DIR) $(LNX_CONFIG_OPTS)
endef

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
