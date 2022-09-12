include $(TOPDIR)/rules.mk

PKG_NAME:=qca-nss-ecm-64
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_URL:=https://git.codelinaro.org/clo/qsdk/oss/lklm/qca-nss-ecm.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-06-15
PKG_SOURCE_VERSION:=a4854db8d70ed0367556d817faa45e43b533a164
PKG_MIRROR_HASH:=7661c6c3b137c127b9ec96aec3a05433d341c3a1db231e812d10380216b87858

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-nss-ecm-64
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=@(TARGET_ipq807x||TARGET_ipq60xx) \
  	+kmod-qca-nss-drv \
	+iptables-mod-extra \
	+kmod-ipt-conntrack \
	+kmod-ipt-physdev \
	+iptables-mod-physdev \
	+kmod-ppp \
	+kmod-pppoe
  TITLE:=QCA NSS Enhanced Connection Manager (ECM)
  FILES:=$(PKG_BUILD_DIR)/ecm.ko
  KCONFIG:=CONFIG_BRIDGE_NETFILTER=y \
	CONFIG_NF_CONNTRACK_EVENTS=y \
	CONFIG_NF_CONNTRACK_CHAIN_EVENTS=n \
	CONFIG_NF_CONNTRACK_DSCPREMARK_EXT=n
endef

define KernelPackage/qca-nss-ecm-64/Description
This package contains the QCA NSS Enhanced Connection Manager
endef

define KernelPackage/qca-nss-ecm-64/install
	$(INSTALL_DIR) $(1)/etc/firewall.d $(1)/etc/init.d $(1)/usr/bin $(1)/lib/netifd/offload $(1)/etc/config $(1)/etc/uci-defaults $(1)/etc/sysctl.d
	$(INSTALL_DATA) ./files/qca-nss-ecm.firewall $(1)/etc/firewall.d/qca-nss-ecm
	$(INSTALL_BIN) ./files/qca-nss-ecm.init $(1)/etc/init.d/qca-nss-ecm
	$(INSTALL_BIN) ./files/ecm_dump.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/on-demand-down $(1)/lib/netifd/offload/on-demand-down
	$(INSTALL_DATA) ./files/qca-nss-ecm.uci $(1)/etc/config/ecm
	$(INSTALL_DATA) ./files/qca-nss-ecm.defaults $(1)/etc/uci-defaults/99-qca-nss-ecm
	$(INSTALL_BIN) ./files/qca-nss-ecm.sysctl $(1)/etc/sysctl.d/qca-nss-ecm.conf
	echo 'net.netfilter.nf_conntrack_max=8192' >> $(1)/etc/sysctl.d/qca-nss-ecm.conf
endef

EXTRA_CFLAGS+=-I$(STAGING_DIR)/usr/include/qca-nss-drv

ifneq (, $(findstring $(CONFIG_TARGET_BOARD), "ipq807x" "ipq60xx"))
ECM_MAKE_OPTS+= ECM_FRONT_END_NSS_ENABLE=y \
		ECM_CLASSIFIER_HYFI_ENABLE=n \
		ECM_MULTICAST_ENABLE=n \
		ECM_INTERFACE_IPSEC_ENABLE=n \
		ECM_INTERFACE_PPTP_ENABLE=n \
		ECM_INTERFACE_L2TPV2_ENABLE=n \
		ECM_INTERFACE_GRE_TAP_ENABLE=n \
		ECM_INTERFACE_GRE_TUN_ENABLE=n \
		ECM_INTERFACE_SIT_ENABLE=n \
		ECM_INTERFACE_TUNIPIP6_ENABLE=n \
		ECM_INTERFACE_RAWIP_ENABLE=n \
		ECM_INTERFACE_VLAN_ENABLE=n \
		ECM_CLASSIFIER_MARK_ENABLE=n \
		ECM_CLASSIFIER_DSCP_ENABLE=n \
		ECM_CLASSIFIER_PCC_ENABLE=n \
		ECM_BAND_STEERING_ENABLE=n \
		ECM_INTERFACE_PPPOE_ENABLE=n
endif

define Build/InstallDev
	mkdir -p $(1)/usr/include/qca-nss-ecm
	$(CP) $(PKG_BUILD_DIR)/exports/* $(1)/usr/include/qca-nss-ecm
endef

define Build/Compile
	+$(MAKE) -C "$(LINUX_DIR)" $(strip $(ECM_MAKE_OPTS)) \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" SoC=$(CONFIG_TARGET_BOARD) \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_JOBS) \
		modules
endef

$(eval $(call KernelPackage,qca-nss-ecm-64))
