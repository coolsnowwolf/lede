include $(TOPDIR)/rules.mk

PKG_NAME:=qca-ssdk
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/quic/qsdk/oss/lklm/qca-ssdk
PKG_SOURCE_DATE:=2021-04-28
PKG_SOURCE_VERSION:=c9bc3bc34eaaac78083573524097356e2dcc1b66
PKG_MIRROR_HASH:=29db78529be32427b8b96fcbfec22a016a243676781ec96d9d65b810944fa405

LOCAL_VARIANT=$(patsubst qca-ssdk-%,%,$(patsubst qca-ssdk-%,%,$(BUILD_VARIANT)))

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-ssdk/default-nohnat
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for QCA SSDK
  FILES:=$(PKG_BUILD_DIR)/build/bin/qca-ssdk.ko
  AUTOLOAD:=$(call AutoLoad,30,qca-ssdk)
  PROVIDES:=qca-ssdk
endef

define KernelPackage/qca-ssdk-nohnat
$(call KernelPackage/qca-ssdk/default-nohnat)
  DEPENDS:=@(TARGET_ipq806x||TARGET_ipq807x||TARGET_ipq60xx)
  VARIANT:=nohnat
endef

define KernelPackage/qca-ssdk-nohnat/Description
This package contains a qca-ssdk driver for QCA chipset
endef

define KernelPackage/qca-ssdk-hnat
$(call KernelPackage/qca-ssdk/default-nohnat)
  DEPENDS:=@TARGET_ipq40xx +kmod-ipt-extra +kmod-ipt-filter \
	+kmod-ppp +TARGET_ipq40xx:kmod-qca-rfs
  TITLE+= (hnat)
  VARIANT:=hnat
endef

define KernelPackage/qca-ssdk-hnat/Description
This package contains a qca-ssdk-hnat driver for QCA chipset
endef

ifdef CONFIG_TOOLCHAIN_BIN_PATH
TOOLCHAIN_BIN_PATH=$(CONFIG_TOOLCHAIN_BIN_PATH)
else
TOOLCHAIN_BIN_PATH=$(TOOLCHAIN_DIR)/bin
endif

QCASSDK_CONFIG_OPTS+= \
	$(KERNEL_MAKE_FLAGS) \
	KVER=$(LINUX_VERSION) \
	SYS_PATH=$(LINUX_DIR) \
	GCC_VERSION=$(GCC_VERSION) \
	TOOLPREFIX=$(TARGET_CROSS) \
	TOOL_PATH=$(TOOLCHAIN_BIN_PATH) \
	TARGET_SUFFIX=$(CONFIG_TARGET_SUFFIX) \
	EXTRA_CFLAGS=-fno-stack-protector -I$(STAGING_DIR)/usr/include

ifeq ($(LOCAL_VARIANT),hnat)
    QCASSDK_CONFIG_OPTS+= HNAT_FEATURE=enable
ifeq ($(BOARD),ipq40xx)
    QCASSDK_CONFIG_OPTS+= RFS_FEATURE=enable
endif
endif

ifeq ($(BOARD),ipq60xx)
    QCASSDK_CONFIG_OPTS+= CHIP_TYPE=CPPE
else ifeq ($(BOARD),ipq807x)
    QCASSDK_CONFIG_OPTS+= CHIP_TYPE=HPPE
else
    QCASSDK_CONFIG_OPTS+= HK_CHIP=enable
endif

ifneq (, $(findstring $(BOARD), ipq60xx ipq807x))
    QCASSDK_CONFIG_OPTS+= PTP_FEATURE=disable SWCONFIG_FEATURE=disable
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(strip $(QCASSDK_CONFIG_OPTS))
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
	$(CP) -rf $(PKG_BUILD_DIR)/include/fal $(1)/usr/include/qca-ssdk
	$(CP) -rf $(PKG_BUILD_DIR)/include/common/*.h $(1)/usr/include/qca-ssdk
	$(CP) -rf $(PKG_BUILD_DIR)/include/sal/os/linux/*.h $(1)/usr/include/qca-ssdk
	$(CP) -rf $(PKG_BUILD_DIR)/include/sal/os/*.h $(1)/usr/include/qca-ssdk
endef

define KernelPackage/qca-ssdk-nohnat/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/qca-ssdk $(1)/etc/init.d/qca-ssdk
endef

define KernelPackage/qca-ssdk-hnat/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/qca-ssdk $(1)/etc/init.d/qca-ssdk
endef

$(eval $(call KernelPackage,qca-ssdk-nohnat))
$(eval $(call KernelPackage,qca-ssdk-hnat))
