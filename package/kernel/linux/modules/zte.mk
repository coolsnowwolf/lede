# SPDX-License-Identifier: GPL-2.0-only

define KernelPackage/zte-npu
  SUBMENU:=Network Devices
  TITLE:=ZTE ZX279133 NP Ethernet controller
  DEPENDS:=@TARGET_zte
  KCONFIG:=CONFIG_ZTE_ZX279133_ETH \
	CONFIG_ZTE_ZX279133_LAN_CONDUIT \
	CONFIG_ZTE_ZX279133_NATIVE_DEBUG=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/zte/npu.ko \
	$(LINUX_DIR)/drivers/net/ethernet/zte/zx279133-lan-conduit.ko
  AUTOLOAD:=$(call AutoLoad,30,npu zx279133-lan-conduit,1)
endef

define KernelPackage/zte-npu/description
 Shared NP/PPU resources and the WAN Ethernet controller. The LAN conduit
 is a separate child module. The microcode in package/firmware/zte/flies/zx279133-npu.bin
 is embedded by the target during the standard kernel build.
endef

$(eval $(call KernelPackage,zte-npu))

define KernelPackage/zte-fnoffload
  SUBMENU:=Network Devices
  TITLE:=ZTE ZX279133 flowtable hardware offload
  DEPENDS:=@TARGET_zte +kmod-zte-npu +kmod-nf-flow
  KCONFIG:=CONFIG_ZTE_ZX279133_FNOFFLOAD
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/zte/fnoffload.ko
  AUTOLOAD:=$(call AutoLoad,45,fnoffload)
endef

define KernelPackage/zte-fnoffload/description
 Translate Linux flowtable rules into NP hardware actions. Conntrack and
 flowtable retain connection ownership; npu owns global hardware resources.
 Experimental VLAN/QoS controls are not enabled by this package.
endef

$(eval $(call KernelPackage,zte-fnoffload))

define KernelPackage/zte-rtl8372n
  SUBMENU:=Network Devices
  TITLE:=ZTE SR1010 RTL8372N DSA switch
  DEPENDS:=@TARGET_zte +kmod-zte-npu
  KCONFIG:=CONFIG_ZTE_ZX279133_RTL8372N
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/zte/zx279133-rtl8372n.ko
  AUTOLOAD:=$(call AutoLoad,35,zx279133-rtl8372n,1)
endef

$(eval $(call KernelPackage,zte-rtl8372n))
