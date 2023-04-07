include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qca-nss-clients
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://git.codelinaro.org/clo/qsdk/oss/lklm/nss-clients.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=740d0102c518cd49f30c5580982b218b480006b1
PKG_MIRROR_HASH:=2f427d01dba69b1b89d3a081daf08b36fb345d55b9c9462eb358e5b071e2a171

include $(INCLUDE_DIR)/package.mk

# Keep default as ipq806x for branches that does not have subtarget framework
ifeq ($(CONFIG_TARGET_ipq),y)
subtarget:=$(SUBTARGET)
else
subtarget:=$(CONFIG_TARGET_BOARD)
endif

ifneq (, $(findstring $(subtarget), "ipq807x" "ipq807x_64" "ipq60xx" "ipq60xx_64"))
# DTLS Manager v2.0 for Hawkeye/Cypress
  DTLSMGR_DIR:=v2.0
# IPsec Manager v2.0 for Hawkeye/Cypress
  IPSECMGR_DIR:=v2.0
# KLIPS plugin
  IPSECMGR_KLIPS:= $(PKG_BUILD_DIR)/ipsecmgr/$(IPSECMGR_DIR)/plugins/klips/qca-nss-ipsec-klips.ko
else
# DTLS Manager v1.0 for Akronite.
  DTLSMGR_DIR:=v1.0
# IPsec Manager v1.0 for Akronite.
  IPSECMGR_DIR:=v1.0
# KLIPS plugin not needed
  IPSECMGR_KLIPS:=
endif

define KernelPackage/qca-nss-drv-tun6rd
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - tun6rd
  DEPENDS:=+kmod-qca-nss-drv +kmod-sit +6rd @!LINUX_3_18
  FILES:=$(PKG_BUILD_DIR)/qca-nss-tun6rd.ko
  AUTOLOAD:=$(call AutoLoad,60,qca-nss-tun6rd)
endef

define KernelPackage/qca-nss-drv-tun6rd/Description
Kernel modules for NSS connection manager - Support for 6rd tunnel
endef

define KernelPackage/qca-nss-drv-dtlsmgr
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - dtlsmgr
  DEPENDS:=+kmod-qca-nss-drv +kmod-qca-nss-cfi-cryptoapi @!LINUX_3_18
  FILES:=$(PKG_BUILD_DIR)/dtls/$(DTLSMGR_DIR)/qca-nss-dtlsmgr.ko
endef

define KernelPackage/qca-nss-drv-dtls/Description
Kernel modules for NSS connection manager - Support for DTLS sessions
endef

define KernelPackage/qca-nss-drv-l2tpv2
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - l2tp
  DEPENDS:=+kmod-qca-nss-drv +kmod-ppp +kmod-l2tp @!LINUX_3_18
  FILES:=$(PKG_BUILD_DIR)/l2tp/l2tpv2/qca-nss-l2tpv2.ko
  KCONFIG:=CONFIG_L2TP=y
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-l2tpv2)
endef

define KernelPackage/qca-nss-drv-l2tpv2/Description
Kernel modules for NSS connection manager - Support for l2tp tunnel
endef

define KernelPackage/qca-nss-drv-pptp
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - PPTP
  DEPENDS:=+kmod-qca-nss-drv +kmod-pptp @!LINUX_3_18
  FILES:=$(PKG_BUILD_DIR)/pptp/qca-nss-pptp.ko
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-pptp)
endef

define KernelPackage/qca-nss-drv-pptp/Description
Kernel modules for NSS connection manager - Support for PPTP tunnel
endef

define KernelPackage/qca-nss-drv-pppoe
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - PPPoE
  DEPENDS:=+kmod-qca-nss-drv +kmod-pppoe @!LINUX_3_18 \
		+!(TARGET_ipq_ipq807x_QSDK_256||TARGET_ipq_ipq60xx_QSDK_256):kmod-bonding
  FILES:=$(PKG_BUILD_DIR)/pppoe/qca-nss-pppoe.ko
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-pppoe)
endef

define KernelPackage/qca-nss-drv-pppoe/Description
Kernel modules for NSS connection manager - Support for PPPoE
endef

define KernelPackage/qca-nss-drv-map-t
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - MAP-T
  DEPENDS:=+kmod-qca-nss-drv +kmod-nat46 @!LINUX_3_18
  FILES:=$(PKG_BUILD_DIR)/map/map-t/qca-nss-map-t.ko
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-map-t)
endef

define KernelPackage/qca-nss-drv-map-t/Description
Kernel modules for NSS connection manager - Support for MAP-T
endef

define KernelPackage/qca-nss-drv-gre
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - GRE
  DEPENDS:=@TARGET_ipq_ipq806x||TARGET_ipq806x||TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq807x||TARGET_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64||TARGET_ipq_ipq50xx||TARGET_ipq_ipq50xx_64 \
	   +kmod-qca-nss-drv @!LINUX_3_18 +kmod-gre6
  FILES:=$(PKG_BUILD_DIR)/gre/qca-nss-gre.ko $(PKG_BUILD_DIR)/gre/test/qca-nss-gre-test.ko
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-gre)
endef

define KernelPackage/qca-nss-drv-gre/Description
Kernel modules for NSS connection manager - Support for GRE
endef

define KernelPackage/qca-nss-drv-tunipip6
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (connection manager) - DS-lite and ipip6 Tunnel
  DEPENDS:=+kmod-qca-nss-drv +kmod-iptunnel6 +kmod-ip6-tunnel @!LINUX_3_18
  FILES:=$(PKG_BUILD_DIR)/qca-nss-tunipip6.ko
  AUTOLOAD:=$(call AutoLoad,60,qca-nss-tunipip6)
endef

define KernelPackage/qca-nss-drv-tunipip6/Description
Kernel modules for NSS connection manager
Add support for DS-lite and ipip6 tunnel
endef

define KernelPackage/qca-nss-drv-profile
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  DEPENDS:=+kmod-qca-nss-drv @!LINUX_3_18
  TITLE:=Profiler for QCA NSS driver (IPQ806x)
  FILES:=$(PKG_BUILD_DIR)/profiler/qca-nss-profile-drv.ko
endef

define KernelPackage/qca-nss-drv-profile/Description
This package contains a NSS driver profiler for QCA chipset
endef

define KernelPackage/qca-nss-drv-ipsecmgr
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS (ipsec manager) - ipsecmgr
  DEPENDS:=@TARGET_ipq806x||TARGET_ipq_ipq806x||TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq807x||TARGET_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64 \
		+kmod-qca-nss-drv +kmod-qca-nss-ecm-standard +kmod-qca-nss-cfi-cryptoapi @!LINUX_3_18
ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-l2tpv2),)
  DEPENDS+=+kmod-qca-nss-drv-l2tpv2
endif
  FILES:=$(PKG_BUILD_DIR)/ipsecmgr/$(IPSECMGR_DIR)/qca-nss-ipsecmgr.ko	$(IPSECMGR_KLIPS)
  AUTOLOAD:=$(call AutoLoad,60,qca-nss-ipsecmgr)
endef

define KernelPackage/qca-nss-drv-ipsecmgr/Description
Kernel module for NSS IPsec offload manager
endef

define KernelPackage/qca-nss-drv-capwapmgr
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  DEPENDS:=+kmod-qca-nss-drv +kmod-qca-nss-drv-dtlsmgr @!LINUX_3_18
  TITLE:=NSS CAPWAP Manager for QCA NSS driver (IPQ806x)
  FILES:=$(PKG_BUILD_DIR)/capwapmgr/qca-nss-capwapmgr.ko
endef

define KernelPackage/qca-nss-drv-capwapmgr/Description
This package contains a NSS CAPWAP Manager
endef

define KernelPackage/qca-nss-drv-bridge-mgr
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS bridge manager
  DEPENDS:=@TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq807x||TARGET_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64 \
		+TARGET_ipq_ipq807x:kmod-qca-nss-drv-vlan-mgr \
		+TARGET_ipq_ipq807x_64:kmod-qca-nss-drv-vlan-mgr \
		+TARGET_ipq807x:kmod-qca-nss-drv-vlan-mgr \
		+TARGET_ipq807x_64:kmod-qca-nss-drv-vlan-mgr \
		+TARGET_ipq_ipq60xx:kmod-qca-nss-drv-vlan-mgr \
		+TARGET_ipq_ipq60xx_64:kmod-qca-nss-drv-vlan-mgr @!LINUX_3_18 \
		+!(TARGET_ipq_ipq807x_QSDK_256||TARGET_ipq_ipq60xx_QSDK_256):kmod-bonding
  FILES:=$(PKG_BUILD_DIR)/bridge/qca-nss-bridge-mgr.ko
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-bridge-mgr)
endef

define KernelPackage/qca-nss-drv-bridge-mgr/Description
Kernel modules for NSS bridge manager
endef

define KernelPackage/qca-nss-drv-vlan-mgr
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS vlan manager
  DEPENDS:=@TARGET_ipq806x||TARGET_ipq807x +kmod-qca-nss-drv @!LINUX_3_18 \
		+!(TARGET_ipq_ipq807x_QSDK_256||TARGET_ipq_ipq60xx_QSDK_256):kmod-bonding
  FILES:=$(PKG_BUILD_DIR)/vlan/qca-nss-vlan.ko
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-vlan)
endef

define KernelPackage/qca-nss-drv-vlan-mgr/Description
Kernel modules for NSS vlan manager
endef

define KernelPackage/qca-nss-drv-qdisc
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  TITLE:=Qdisc for configuring shapers in NSS
  DEPENDS:=+kmod-qca-nss-drv @!LINUX_3_18
  FILES:=$(PKG_BUILD_DIR)/nss_qdisc/qca-nss-qdisc.ko
  KCONFIG:=CONFIG_NET_CLS_ACT=y
  AUTOLOAD:=$(call AutoLoad,58,qca-nss-qdisc)
endef

define KernelPackage/qca-nss-drv-qdisc/Description
Linux qdisc that aids in configuring shapers in the NSS
endef

define KernelPackage/qca-nss-drv-lag-mgr
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS LAG manager
  DEPENDS:=+kmod-qca-nss-drv  @!LINUX_3_18 \
	   +TARGET_ipq_ipq807x:kmod-qca-nss-drv-vlan-mgr \
	   +TARGET_ipq_ipq807x_64:kmod-qca-nss-drv-vlan-mgr @!LINUX_3_18 \
	   +TARGET_ipq807x:kmod-qca-nss-drv-vlan-mgr \
	   +TARGET_ipq807x_64:kmod-qca-nss-drv-vlan-mgr @!LINUX_3_18 \
	   +TARGET_ipq_ipq60xx:kmod-qca-nss-drv-vlan-mgr @!LINUX_3_18 \
	   +TARGET_ipq_ipq60xx_64:kmod-qca-nss-drv-vlan-mgr @!LINUX_3_18 \
	   +kmod-bonding
  FILES:=$(PKG_BUILD_DIR)/lag/qca-nss-lag-mgr.ko
  AUTOLOAD:=$(call AutoLoad,51,qca-nss-lag-mgr)
endef

define KernelPackage/qca-nss-drv-lag-mgr/Description
Kernel modules for NSS LAG manager
endef

define KernelPackage/qca-nss-drv-netlink
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  DEPENDS:=@TARGET_ipq807x||TARGET_ipq_ipq807x||TARGET_ipq807x_64||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64||TARGET_ipq_ipq50xx||TARGET_ipq_ipq50xx_64 \
		+kmod-qca-nss-drv @!LINUX_3_18 \
		+PACKAGE_kmod-qca-nss-drv-ipsecmgr:kmod-qca-nss-drv-ipsecmgr \
		+PACKAGE_kmod-qca-nss-drv-dtlsmgr:kmod-qca-nss-drv-dtlsmgr \
		+PACKAGE_kmod-qca-nss-drv-capwapmgr:kmod-qca-nss-drv-capwapmgr @!LINUX_3_18
  TITLE:=NSS NETLINK Manager for QCA NSS driver
  FILES:=$(PKG_BUILD_DIR)/netlink/qca-nss-netlink.ko
endef

define KernelPackage/qca-nss-drv-netlink/Description
Kernel module for NSS netlink manager
endef

define KernelPackage/qca-nss-drv-ovpn-mgr
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for NSS OpenVPN manager
  DEPENDS:=+kmod-qca-nss-drv +kmod-qca-nss-cfi-cryptoapi +kmod-tun +kmod-ipt-conntrack @!LINUX_3_18 \
	  @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64
  FILES:=$(PKG_BUILD_DIR)/openvpn/src/qca-nss-ovpn-mgr.ko
endef

define KernelPackage/qca-nss-drv-ovpn-mgr/Description
Kernel module for NSS OpenVPN manager
endef

define KernelPackage/qca-nss-drv-ovpn-link
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for interfacing NSS OpenVPN manager with ECM
  DEPENDS:=+kmod-qca-nss-drv-ovpn-mgr +@PACKAGE_kmod-qca-nss-ecm-premium @!LINUX_3_18 \
	  @TARGET_ipq_ipq807x||TARGET_ipq_ipq807x_64||TARGET_ipq_ipq60xx||TARGET_ipq_ipq60xx_64
  FILES:=$(PKG_BUILD_DIR)/openvpn/plugins/qca-nss-ovpn-link.ko
endef

define KernelPackage/qca-nss-drv-ovpn-link/Description
This module registers with ECM and communicates with NSS OpenVPN manager for supporting OpenVPN offload.
endef

define KernelPackage/qca-nss-drv-pvxlanmgr
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  DEPENDS:=+kmod-qca-nss-drv @!LINUX_3_18
  TITLE:=NSS PVXLAN Manager for QCA NSS driver
  FILES:=$(PKG_BUILD_DIR)/pvxlanmgr/qca-nss-pvxlanmgr.ko
endef

define KernelPackage/qca-nss-drv-pvxlanmgr/Description
Kernel module for managing NSS PVxLAN
endef

define Build/InstallDev/qca-nss-clients
	$(INSTALL_DIR) $(1)/usr/include/qca-nss-clients
	$(CP) $(PKG_BUILD_DIR)/netlink/include/* $(1)/usr/include/qca-nss-clients/
	$(CP) $(PKG_BUILD_DIR)/exports/* $(1)/usr/include/qca-nss-clients/
endef

define Build/InstallDev
	$(call Build/InstallDev/qca-nss-clients,$(1))
endef

define KernelPackage/qca-nss-drv-ovpn-mgr/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/qca-nss-ovpn.init $(1)/etc/init.d/qca-nss-ovpn
endef

define KernelPackage/qca-nss-drv-ipsecmgr/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/qca-nss-ipsec $(1)/etc/init.d/qca-nss-ipsec
endef


EXTRA_CFLAGS+= \
	-I$(STAGING_DIR)/usr/include/qca-nss-drv \
	-I$(STAGING_DIR)/usr/include/qca-nss-crypto \
	-I$(STAGING_DIR)/usr/include/qca-nss-cfi \
	-I$(STAGING_DIR)/usr/include/qca-nss-gmac \
	-I$(STAGING_DIR)/usr/include/qca-nss-ecm \
	-I$(STAGING_DIR)/usr/include/qca-ssdk \
	-I$(STAGING_DIR)/usr/include/qca-ssdk/fal \
	-I$(STAGING_DIR)/usr/include/nat46

# Build individual packages if selected
ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-profile),)
MAKE_OPTS+=profile=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-capwapmgr),)
MAKE_OPTS+=capwapmgr=y
EXTRA_CFLAGS += -DNSS_CAPWAPMGR_ONE_NETDEV
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-tun6rd),)
MAKE_OPTS+=tun6rd=m
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-dtlsmgr),)
MAKE_OPTS+=dtlsmgr=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-l2tpv2),)
MAKE_OPTS+=l2tpv2=y
EXTRA_CFLAGS += -DNSS_L2TPV2_ENABLED
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-pptp),)
MAKE_OPTS+=pptp=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-map-t),)
MAKE_OPTS+=map-t=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-tunipip6),)
MAKE_OPTS+=tunipip6=m
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-qdisc),)
MAKE_OPTS+=qdisc=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-ipsecmgr),)
EXTRA_CFLAGS+= -I$(PKG_BUILD_DIR)/exports \
		-I$(STAGING_DIR)/usr/include/qca-nss-ecm
MAKE_OPTS+=ipsecmgr=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-bridge-mgr),)
MAKE_OPTS+=bridge-mgr=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-vlan-mgr),)
MAKE_OPTS+=vlan-mgr=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-lag-mgr),)
MAKE_OPTS+=lag-mgr=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-gre),)
EXTRA_CFLAGS+= -I$(PKG_BUILD_DIR)/exports
MAKE_OPTS+=gre=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-pppoe),)
MAKE_OPTS+=pppoe=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-netlink),)
MAKE_OPTS+=netlink=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-ovpn-mgr),)
MAKE_OPTS+=ovpn-mgr=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-ovpn-link),)
MAKE_OPTS+=ovpn-link=y
endif

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-drv-pvxlanmgr),)
MAKE_OPTS+=pvxlanmgr=y
endif

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" $(strip $(MAKE_OPTS)) \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		SoC="$(subtarget)" \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" \
		modules
endef

$(eval $(call KernelPackage,qca-nss-drv-profile))
$(eval $(call KernelPackage,qca-nss-drv-capwapmgr))
$(eval $(call KernelPackage,qca-nss-drv-tun6rd))
$(eval $(call KernelPackage,qca-nss-drv-dtlsmgr))
$(eval $(call KernelPackage,qca-nss-drv-l2tpv2))
$(eval $(call KernelPackage,qca-nss-drv-pptp))
$(eval $(call KernelPackage,qca-nss-drv-pppoe))
$(eval $(call KernelPackage,qca-nss-drv-map-t))
$(eval $(call KernelPackage,qca-nss-drv-tunipip6))
$(eval $(call KernelPackage,qca-nss-drv-qdisc))
$(eval $(call KernelPackage,qca-nss-drv-netlink))
$(eval $(call KernelPackage,qca-nss-drv-ipsecmgr))
$(eval $(call KernelPackage,qca-nss-drv-bridge-mgr))
$(eval $(call KernelPackage,qca-nss-drv-vlan-mgr))
$(eval $(call KernelPackage,qca-nss-drv-lag-mgr))
$(eval $(call KernelPackage,qca-nss-drv-gre))
$(eval $(call KernelPackage,qca-nss-drv-ovpn-mgr))
$(eval $(call KernelPackage,qca-nss-drv-ovpn-link))
$(eval $(call KernelPackage,qca-nss-drv-pvxlanmgr))
