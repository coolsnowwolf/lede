include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qca-nss-ecm
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://source.codeaurora.org/quic/qsdk/oss/lklm/qca-nss-ecm
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=9228212b4238c0d8c296f795948ede8f2ca0242e
PKG_MIRROR_HASH:=02fe4c86c8c88fb15704b1b253ab756a2658f24ce5db64a7909cb60bf9c1cdff

LOCAL_VARIANT=$(patsubst qca-nss-ecm-%,%,$(patsubst qca-nss-ecm-%,%,$(BUILD_VARIANT)))
include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_QCA_NSS_ECM_EXAMPLES_PCC),y)
       FILES_EXAMPLES=$(PKG_BUILD_DIR)/examples/ecm_pcc_test.ko
endif

ifeq ($(CONFIG_QCA_NSS_ECM_EXAMPLES_MARK),y)
       FILES_EXAMPLES+=$(PKG_BUILD_DIR)/examples/ecm_mark_test.ko
endif

#Explicitly enable OVS external module, if ovsmgr is enabled.
ifneq ($(CONFIG_PACKAGE_kmod-qca-ovsmgr),)
CONFIG_QCA_NSS_ECM_OVS=y
endif

ifeq ($(CONFIG_QCA_NSS_ECM_OVS),y)
       FILES_EXAMPLES+=$(PKG_BUILD_DIR)/examples/ecm_ovs.ko
endif

define KernelPackage/qca-nss-ecm/Default
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=+TARGET_ipq806x:kmod-qca-nss-drv \
	+TARGET_ipq_ipq806x:kmod-qca-nss-drv \
	+TARGET_ipq_ipq807x:kmod-qca-nss-drv \
	+TARGET_ipq_ipq807x_64:kmod-qca-nss-drv \
	+TARGET_ipq807x:kmod-qca-nss-drv \
	+TARGET_ipq807x_64:kmod-qca-nss-drv \
	+TARGET_ipq_ipq60xx:kmod-qca-nss-drv \
	+TARGET_ipq_ipq60xx_64:kmod-qca-nss-drv \
	+TARGET_ipq_ipq50xx:kmod-qca-nss-drv \
	+TARGET_ipq_ipq50xx_64:kmod-qca-nss-drv \
	+iptables-mod-extra +kmod-ipt-conntrack \
	+kmod-pppoe @!LINUX_3_18 \
	+kmod-ipsec
  TITLE:=QCA NSS Enhanced Connection Manager (ECM)
  FILES:=$(PKG_BUILD_DIR)/*.ko $(FILES_EXAMPLES)
  KCONFIG:=CONFIG_BRIDGE_NETFILTER=y \
	CONFIG_NF_CONNTRACK_EVENTS=y \
	CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y \
	CONFIG_NF_CONNTRACK_DSCPREMARK_EXT=y
  MENU:=1
  PROVIDES:=kmod-qca-nss-ecm
endef

define KernelPackage/qca-nss-ecm/Description/Default
This package contains the QCA NSS Enhanced Connection Manager
endef

define KernelPackage/qca-nss-ecm/Default/install
	$(INSTALL_DIR) $(1)/etc/firewall.d $(1)/etc/init.d $(1)/usr/bin $(1)/lib/netifd/offload $(1)/etc/config $(1)/etc/uci-defaults $(1)/etc/sysctl.d
	$(INSTALL_DATA) ./files/qca-nss-ecm.firewall $(1)/etc/firewall.d/qca-nss-ecm
	$(INSTALL_BIN) ./files/qca-nss-ecm.init $(1)/etc/init.d/qca-nss-ecm
	$(INSTALL_BIN) ./files/ecm_dump.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/on-demand-down $(1)/lib/netifd/offload/on-demand-down
	$(INSTALL_DATA) ./files/qca-nss-ecm.uci $(1)/etc/config/ecm
	$(INSTALL_DATA) ./files/qca-nss-ecm.defaults $(1)/etc/uci-defaults/99-qca-nss-ecm
	$(INSTALL_BIN) ./files/qca-nss-ecm.sysctl $(1)/etc/sysctl.d/qca-nss-ecm.conf
ifeq ($(CONFIG_KERNEL_IPQ_MEM_PROFILE),256)
	echo 'net.netfilter.nf_conntrack_max=2048' >> $(1)/etc/sysctl.d/qca-nss-ecm.conf
endif
ifeq ($(CONFIG_KERNEL_IPQ_MEM_PROFILE),512)
	echo 'net.netfilter.nf_conntrack_max=8192' >> $(1)/etc/sysctl.d/qca-nss-ecm.conf
endif
endef

define KernelPackage/qca-nss-ecm-standard
  $(call KernelPackage/qca-nss-ecm/Default)
  VARIANT:=standard
endef

define KernelPackage/qca-nss-ecm-standard/Description
  $(call KernelPackage/qca-nss-ecm/Description/Default)
endef

define KernelPackage/qca-nss-ecm-standard/install
$(call KernelPackage/qca-nss-ecm/Default/install, $(1))
endef

# Variant with additional features enabled for premium profile
define KernelPackage/qca-nss-ecm-premium/Default
$(call KernelPackage/qca-nss-ecm/Default)
  TITLE+= (with premium features)
  VARIANT:=premium
  DEPENDS+=+kmod-nat46 \
	+kmod-l2tp +kmod-pppol2tp +kmod-pptp \
	+kmod-bonding
endef

define KernelPackage/qca-nss-ecm-premium/Description/Default
$(call KernelPackage/qca-nss-ecm/Description/Default)
with the premium features enabled
endef

define KernelPackage/qca-nss-ecm-premium/Default/install
$(call KernelPackage/qca-nss-ecm/install)
endef

define KernelPackage/qca-nss-ecm-premium
$(call KernelPackage/qca-nss-ecm-premium/Default)
endef

define KernelPackage/qca-nss-ecm-premium/Description
$(call KernelPackage/qca-nss-ecm-premium/Description/Default)
endef

define KernelPackage/qca-nss-ecm-premium/install
$(call KernelPackage/qca-nss-ecm-standard/install, $(1))
endef

# Variant with additional features enabled for noload profile
define KernelPackage/qca-nss-ecm-noload
  $(call KernelPackage/qca-nss-ecm/Default)
  TITLE+= (with noload features)
  PROVIDES:=kmod-qca-nss-ecm
  VARIANT:=noload
  DEPENDS+=+kmod-l2tp +kmod-pppol2tp +kmod-pptp \
	   +kmod-bonding
endef

define KernelPackage/qca-nss-ecm-noload/Description
  $(call KernelPackage/qca-nss-ecm/Description/Default)
  When selected, this package installs the driver but does not load it at init.
endef

define KernelPackage/qca-nss-ecm-noload/install
$(call KernelPackage/qca-nss-ecm/Default/install, $(1))
	#
	# Remove the START line from the init script, so that the symlink
	# in the /etc/rc.d directory is not created.
	#
	sed -i '/START=/d' $(1)/etc/init.d/qca-nss-ecm
endef

define KernelPackage/qca-nss-ecm-premium-noload
  $(call KernelPackage/qca-nss-ecm-premium/Default)
  PROVIDES:=kmod-qca-nss-ecm-premium
endef

define KernelPackage/qca-nss-ecm-premium-noload/Description
  $(call KernelPackage/qca-nss-ecm-premium/Description/Default)
  When selected, this package installs the driver but does not load it at init.
endef

define KernelPackage/qca-nss-ecm-premium-noload/install
$(call KernelPackage/qca-nss-ecm-premium/Default/install, $(1))
endef

define Build/InstallDev/qca-nss-ecm
	$(INSTALL_DIR) $(1)/usr/include/qca-nss-ecm
	$(CP) $(PKG_BUILD_DIR)/exports/* $(1)/usr/include/qca-nss-ecm/
endef

define Build/InstallDev
	$(call Build/InstallDev/qca-nss-ecm,$(1))
endef

EXTRA_CFLAGS+= \
	-I$(STAGING_DIR)/usr/include/hyfibr \
	-I$(STAGING_DIR)/usr/include/qca-mcs \
	-I$(STAGING_DIR)/usr/include/qca-nss-drv \
	-I$(STAGING_DIR)/usr/include/shortcut-fe \
	-I$(STAGING_DIR)/usr/include/nat46

ECM_MAKE_OPTS:=ECM_CLASSIFIER_HYFI_ENABLE=y
ifneq ($(LOCAL_VARIANT),standard)
ECM_MAKE_OPTS+=ECM_MULTICAST_ENABLE=y \
		ECM_INTERFACE_IPSEC_ENABLE=y \
		# ECM_INTERFACE_PPTP_ENABLE=y \
		ECM_INTERFACE_L2TPV2_ENABLE=y \
		ECM_INTERFACE_GRE_TAP_ENABLE=y \
		ECM_INTERFACE_GRE_TUN_ENABLE=y \
		ECM_INTERFACE_SIT_ENABLE=y \
		ECM_INTERFACE_TUNIPIP6_ENABLE=y \
		ECM_INTERFACE_RAWIP_ENABLE=y

ifeq ($(CONFIG_TARGET_ipq_ipq40xx)$(CONFIG_TARGET_ipq40xx),)
ECM_MAKE_OPTS+=ECM_INTERFACE_BOND_ENABLE=y
endif
endif

ifeq ($(filter $(CONFIG_KERNEL_IPQ_MEM_PROFILE), 256),)
ECM_MAKE_OPTS+=ECM_XFRM_ENABLE=y
endif

# ifneq ($(CONFIG_PACKAGE_kmod-nat46),)
# ECM_MAKE_OPTS+=ECM_INTERFACE_MAP_T_ENABLE=y
# endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-ovpn-link),)
ECM_MAKE_OPTS+=ECM_INTERFACE_OVPN_ENABLE=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-vxlanmgr),)
ECM_MAKE_OPTS+=ECM_INTERFACE_VXLAN_ENABLE=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-ovsmgr),)
ECM_MAKE_OPTS+=ECM_INTERFACE_OVS_BRIDGE_ENABLE=y \
		ECM_CLASSIFIER_OVS_ENABLE=y
EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/qca-ovsmgr
endif

# Keeping default as ipq806x for branches that does not have subtarget framework
ifeq ($(CONFIG_TARGET_ipq),y)
subtarget:=$(SUBTARGET)
else
subtarget:=$(CONFIG_TARGET_BOARD)
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/qca-nss-ecm
	$(CP) $(PKG_BUILD_DIR)/exports/* $(1)/usr/include/qca-nss-ecm
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" $(strip $(ECM_MAKE_OPTS)) \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" SoC="$(subtarget)" \
		EXAMPLES_BUILD_PCC="$(CONFIG_QCA_NSS_ECM_EXAMPLES_PCC)" \
		EXAMPLES_BUILD_MARK="$(CONFIG_QCA_NSS_ECM_EXAMPLES_MARK)" \
		EXAMPLES_BUILD_OVS="$(CONFIG_QCA_NSS_ECM_OVS)" \
		ECM_FRONT_END_SFE_ENABLE="$(CONFIG_QCA_ECM_SFE_SUPPORT)" \
		modules
endef

define KernelPackage/qca-nss-ecm-premium/config
menu "ECM Configuration"
	config QCA_NSS_ECM_EXAMPLES_PCC
		bool "Build PCC usage example"
		help
			Selecting this will build the PCC classifier usage example module.
		default n

	config QCA_NSS_ECM_EXAMPLES_MARK
		bool "Build Mark classifier usage example"
		help
			Selecting this will build the Mark classifier usage example module.
		default n

	config QCA_NSS_ECM_OVS
		bool "Build OVS classifier external module"
		help
			Selecting this will build the OVS classifier external module.
		default n

	config QCA_ECM_SFE_SUPPORT
		bool "Add SFE support to ECM driver"
		default n
endmenu
endef

$(eval $(call KernelPackage,qca-nss-ecm-noload))
$(eval $(call KernelPackage,qca-nss-ecm-standard))
$(eval $(call KernelPackage,qca-nss-ecm-premium-noload))
$(eval $(call KernelPackage,qca-nss-ecm-premium))
