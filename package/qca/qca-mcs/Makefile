include $(TOPDIR)/rules.mk

PKG_NAME:=qca-mcs
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2023-04-21
PKG_SOURCE_URL:=https://git.codelinaro.org/clo/qsdk/oss/lklm/qca-mcs.git
PKG_SOURCE_VERSION:=8797823e392ac3d9098c090964afd46805a0eb2b
PKG_MIRROR_HASH:=f0fa76af4545842ebf8b4f0743e1079e190dfa5f3f0d464c063063521d3d30df

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-mcs
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  URL:=http://www.qca.qualcomm.com
  MAINTAINER:=Qualcomm Atheros, Inc.
  TITLE:=QCA Multicast Snooping Support
  DEPENDS:=@TARGET_qualcommax \
	+@KERNEL_IPV6_MROUTE +@KERNEL_IP_MROUTE
  KCONFIG:=CONFIG_NETFILTER=y \
	   CONFIG_BRIDGE_NETFILTER=y
  FILES:=$(PKG_BUILD_DIR)/qca-mcs.ko
  AUTOLOAD:=$(call AutoLoad,41,qca-mcs)
endef

define KernelPackage/qca-mcs/Description
  This package installs the IGMP/MLD Snooping Module
endef

QCA_MC_SNOOPING_HEADERS= \
			$(PKG_BUILD_DIR)/mc_api.h \
			$(PKG_BUILD_DIR)/mc_ecm.h \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/qca-mcs
	$(foreach header_file,$(QCA_MC_SNOOPING_HEADERS), $(CP) $(header_file) $(1)/usr/include/qca-mcs;)
	$(foreach header_file,$(QCA_MC_SNOOPING_HEADERS), $(CP) $(header_file) $(1)/usr/include/;)
endef

EXTRA_CFLAGS+=-Wno-implicit-fallthrough

QCA_MC_SNOOPING_MAKE_OPTS:= \
        $(KERNEL_MAKE_FLAGS) \
        CONFIG_SUPPORT_MLD=y \
        MDIR=$(PKG_BUILD_DIR) \
        KBUILDPATH=$(LINUX_DIR) \
        KERNELPATH=$(LINUX_SRC_DIR) \
        KERNELRELEASE=$(LINUX_RELEASE)

define Build/Compile
	+$(MAKE) -C $(LINUX_DIR) \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_JOBS) \
		KBUILDPATH=$(LINUX_DIR) \
		$(PKG_MAKE_FLAGS) \
		M=$(PKG_BUILD_DIR) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		$(strip $(QCA_MC_SNOOPING_MAKE_OPTS)) \
		modules
endef

$(eval $(call KernelPackage,qca-mcs))
