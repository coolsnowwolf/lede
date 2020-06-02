#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

NETWORK_DEVICES_MENU:=Network Devices

define KernelPackage/sis190
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=SiS 190 Fast/Gigabit Ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-mii
  KCONFIG:=CONFIG_SIS190
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/sis/sis190.ko
  AUTOLOAD:=$(call AutoProbe,sis190)
endef

$(eval $(call KernelPackage,sis190))


define KernelPackage/skge
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=SysKonnect Yukon support
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_SKGE \
	CONFIG_SKGE_DEBUG=n \
	CONFIG_SKGE_GENESIS=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/marvell/skge.ko
  AUTOLOAD:=$(call AutoProbe,skge)
endef

$(eval $(call KernelPackage,skge))


define KernelPackage/alx
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Qualcomm Atheros AR816x/AR817x PCI-E Ethernet Network Driver
  DEPENDS:=@PCI_SUPPORT +kmod-mdio
  KCONFIG:=CONFIG_ALX
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/atheros/alx/alx.ko
  AUTOLOAD:=$(call AutoProbe,alx)
endef

$(eval $(call KernelPackage,alx))


define KernelPackage/atl2
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Atheros L2 Fast Ethernet support
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_ATL2
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/atheros/atlx/atl2.ko
  AUTOLOAD:=$(call AutoProbe,atl2)
endef

$(eval $(call KernelPackage,atl2))


define KernelPackage/atl1
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Atheros L1 Gigabit Ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-mii
  KCONFIG:=CONFIG_ATL1
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/atheros/atlx/atl1.ko
  AUTOLOAD:=$(call AutoProbe,atl1)
endef

$(eval $(call KernelPackage,atl1))


define KernelPackage/atl1c
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Atheros L1C
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_ATL1C
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/atheros/atl1c/atl1c.ko
  AUTOLOAD:=$(call AutoProbe,atl1c)
endef

$(eval $(call KernelPackage,atl1c))


define KernelPackage/atl1e
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Atheros L1E
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_ATL1E
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/atheros/atl1e/atl1e.ko
  AUTOLOAD:=$(call AutoProbe,atl1e)
endef

$(eval $(call KernelPackage,atl1e))


define KernelPackage/libphy
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=PHY library
  KCONFIG:=CONFIG_PHYLIB
  FILES:=$(LINUX_DIR)/drivers/net/phy/libphy.ko
  AUTOLOAD:=$(call AutoLoad,15,libphy,1)
endef

define KernelPackage/libphy/description
 PHY library
endef

$(eval $(call KernelPackage,libphy))

define KernelPackage/mii
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=MII library
  KCONFIG:=CONFIG_MII
  FILES:=$(LINUX_DIR)/drivers/net/mii.ko
  AUTOLOAD:=$(call AutoLoad,15,mii,1)
endef

define KernelPackage/mii/description
  MII library
endef

$(eval $(call KernelPackage,mii))


define KernelPackage/mdio-gpio
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:= Supports GPIO lib-based MDIO busses
  DEPENDS:=+kmod-libphy @GPIO_SUPPORT +(TARGET_armvirt||TARGET_bcm27xx_bcm2708||TARGET_samsung||TARGET_tegra):kmod-of-mdio
  KCONFIG:= \
	CONFIG_MDIO_BITBANG \
	CONFIG_MDIO_GPIO
  FILES:= \
	$(LINUX_DIR)/drivers/net/phy/mdio-gpio.ko \
	$(LINUX_DIR)/drivers/net/phy/mdio-bitbang.ko
  AUTOLOAD:=$(call AutoProbe,mdio-gpio)
endef

define KernelPackage/mdio-gpio/description
 Supports GPIO lib-based MDIO busses
endef

$(eval $(call KernelPackage,mdio-gpio))


define KernelPackage/et131x
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Agere ET131x Gigabit Ethernet driver
  URL:=http://sourceforge.net/projects/et131x
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/agere/et131x.ko
  KCONFIG:= \
	CONFIG_ET131X \
	CONFIG_ET131X_DEBUG=n
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  AUTOLOAD:=$(call AutoProbe,et131x)
endef

define KernelPackage/et131x/description
 This package contains the et131x kernel module
endef

$(eval $(call KernelPackage,et131x))


define KernelPackage/phylib-broadcom
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Broadcom Ethernet PHY library
   KCONFIG:=CONFIG_BCM_NET_PHYLIB
   HIDDEN:=1
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/bcm-phy-lib.ko
   AUTOLOAD:=$(call AutoLoad,17,bcm-phy-lib)
endef

$(eval $(call KernelPackage,phylib-broadcom))


define KernelPackage/phy-broadcom
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Broadcom Ethernet PHY driver
   KCONFIG:=CONFIG_BROADCOM_PHY
   DEPENDS:=+kmod-libphy +kmod-phylib-broadcom
   FILES:=$(LINUX_DIR)/drivers/net/phy/broadcom.ko
   AUTOLOAD:=$(call AutoLoad,18,broadcom,1)
endef

define KernelPackage/phy-broadcom/description
   Currently supports the BCM5411, BCM5421, BCM5461, BCM5464, BCM5481,
   BCM5482 and BCM57780 PHYs.
endef

$(eval $(call KernelPackage,phy-broadcom))


define KernelPackage/phy-realtek
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Realtek Ethernet PHY driver
   KCONFIG:=CONFIG_REALTEK_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/realtek.ko
   AUTOLOAD:=$(call AutoLoad,18,realtek,1)
endef

define KernelPackage/phy-realtek/description
   Supports the Realtek 821x PHY.
endef

$(eval $(call KernelPackage,phy-realtek))


define KernelPackage/swconfig
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=switch configuration API
  DEPENDS:=+kmod-libphy
  KCONFIG:=CONFIG_SWCONFIG
  FILES:=$(LINUX_DIR)/drivers/net/phy/swconfig.ko
  AUTOLOAD:=$(call AutoLoad,41,swconfig)
endef

define KernelPackage/swconfig/description
 Switch configuration API module
endef

$(eval $(call KernelPackage,swconfig))

define KernelPackage/switch-mvsw61xx
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell 88E61xx switch support
  DEPENDS:=+kmod-swconfig
  KCONFIG:=CONFIG_MVSW61XX_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/mvsw61xx.ko
  AUTOLOAD:=$(call AutoLoad,42,mvsw61xx)
endef

define KernelPackage/switch-mvsw61xx/description
 Marvell 88E61xx switch support
endef

$(eval $(call KernelPackage,switch-mvsw61xx))

define KernelPackage/switch-ip17xx
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=IC+ IP17XX switch support
  DEPENDS:=+kmod-swconfig
  KCONFIG:=CONFIG_IP17XX_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/ip17xx.ko
  AUTOLOAD:=$(call AutoLoad,42,ip17xx)
endef

define KernelPackage/switch-ip17xx/description
 IC+ IP175C/IP178C switch support
endef

$(eval $(call KernelPackage,switch-ip17xx))


define KernelPackage/switch-rtl8306
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8306S switch support
  DEPENDS:=+kmod-swconfig
  KCONFIG:=CONFIG_RTL8306_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/rtl8306.ko
  AUTOLOAD:=$(call AutoLoad,43,rtl8306)
endef

define KernelPackage/switch-rtl8306/description
 Realtek RTL8306S switch support
endef

$(eval $(call KernelPackage,switch-rtl8306))


define KernelPackage/switch-rtl8366-smi
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8366 SMI switch interface support
  DEPENDS:=@GPIO_SUPPORT +kmod-swconfig +(TARGET_armvirt||TARGET_bcm27xx_bcm2708||TARGET_samsung||TARGET_tegra):kmod-of-mdio
  KCONFIG:=CONFIG_RTL8366_SMI
  FILES:=$(LINUX_DIR)/drivers/net/phy/rtl8366_smi.ko
  AUTOLOAD:=$(call AutoLoad,42,rtl8366_smi)
endef

define KernelPackage/switch-rtl8366-smi/description
  Realtek RTL8366 series SMI switch interface support
endef

$(eval $(call KernelPackage,switch-rtl8366-smi))


define KernelPackage/switch-rtl8366rb
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8366RB switch support
  DEPENDS:=+kmod-switch-rtl8366-smi
  KCONFIG:=CONFIG_RTL8366RB_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/rtl8366rb.ko
  AUTOLOAD:=$(call AutoLoad,43,rtl8366rb)
endef

define KernelPackage/switch-rtl8366rb/description
 Realtek RTL8366RB switch support
endef

$(eval $(call KernelPackage,switch-rtl8366rb))


define KernelPackage/switch-rtl8366s
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8366S switch support
  DEPENDS:=+kmod-switch-rtl8366-smi
  KCONFIG:=CONFIG_RTL8366S_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/rtl8366s.ko
  AUTOLOAD:=$(call AutoLoad,43,rtl8366s)
endef

define KernelPackage/switch-rtl8366s/description
 Realtek RTL8366S switch support
endef

$(eval $(call KernelPackage,switch-rtl8366s))


define KernelPackage/switch-rtl8367b
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8367R/B switch support
  DEPENDS:=+kmod-switch-rtl8366-smi
  KCONFIG:=CONFIG_RTL8367B_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/rtl8367b.ko
  AUTOLOAD:=$(call AutoLoad,43,rtl8367b)
endef

define KernelPackage/switch-rtl8367b/description
 Realtek RTL8367R/B switch support
endef

$(eval $(call KernelPackage,switch-rtl8367b))


define KernelPackage/natsemi
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=National Semiconductor DP8381x series
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_NATSEMI
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/natsemi/natsemi.ko
  AUTOLOAD:=$(call AutoLoad,20,natsemi)
endef

define KernelPackage/natsemi/description
 Kernel modules for National Semiconductor DP8381x series PCI Ethernet
 adapters.
endef

$(eval $(call KernelPackage,natsemi))


define KernelPackage/r6040
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=RDC Fast-Ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  KCONFIG:=CONFIG_R6040 \
		CONFIG_R6040_NAPI=y
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/rdc/r6040.ko
  AUTOLOAD:=$(call AutoProbe,r6040)
endef

define KernelPackage/r6040/description
 Kernel modules for RDC Fast-Ethernet adapters.
endef

$(eval $(call KernelPackage,r6040))


define KernelPackage/niu
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Sun Neptune 10Gbit Ethernet support
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_NIU
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/sun/niu.ko
  AUTOLOAD:=$(call AutoProbe,niu)
endef

define KernelPackage/niu/description
 This enables support for cards based upon Sun's Neptune chipset.
endef

$(eval $(call KernelPackage,niu))


define KernelPackage/sis900
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=SiS 900 Ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-mii
  KCONFIG:=CONFIG_SIS900
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/sis/sis900.ko
  AUTOLOAD:=$(call AutoProbe,sis900)
endef

define KernelPackage/sis900/description
 Kernel modules for Sis 900 Ethernet adapters.
endef

$(eval $(call KernelPackage,sis900))


define KernelPackage/sky2
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=SysKonnect Yukon2 support
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_SKY2
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/marvell/sky2.ko
  AUTOLOAD:=$(call AutoProbe,sky2)
endef

define KernelPackage/sky2/description
  This driver supports Gigabit Ethernet adapters based on the
  Marvell Yukon 2 chipset:
  Marvell 88E8021/88E8022/88E8035/88E8036/88E8038/88E8050/88E8052/
  88E8053/88E8055/88E8061/88E8062, SysKonnect SK-9E21D/SK-9S21

  There is companion driver for the older Marvell Yukon and
  Genesis based adapters: skge.
endef

$(eval $(call KernelPackage,sky2))


define KernelPackage/via-rhine
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Via Rhine ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-mii
  KCONFIG:=CONFIG_VIA_RHINE \
    CONFIG_VIA_RHINE_MMIO=y
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/via/via-rhine.ko
  AUTOLOAD:=$(call AutoProbe,via-rhine)
endef

define KernelPackage/via-rhine/description
 Kernel modules for Via Rhine Ethernet chipsets
endef

$(eval $(call KernelPackage,via-rhine))


define KernelPackage/via-velocity
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=VIA Velocity Gigabit Ethernet Adapter kernel support
  DEPENDS:=@PCI_SUPPORT +kmod-lib-crc-ccitt
  KCONFIG:=CONFIG_VIA_VELOCITY
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/via/via-velocity.ko
  AUTOLOAD:=$(call AutoProbe,via-velocity)
endef

define KernelPackage/via-velocity/description
 Kernel modules for VIA Velocity Gigabit Ethernet chipsets
endef

$(eval $(call KernelPackage,via-velocity))


define KernelPackage/8139too
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=RealTek RTL-8139 PCI Fast Ethernet Adapter kernel support
  DEPENDS:=@PCI_SUPPORT +kmod-mii
  KCONFIG:=CONFIG_8139TOO \
    CONFIG_8139TOO_PIO=y \
    CONFIG_8139TOO_TUNE_TWISTER=n \
    CONFIG_8139TOO_8129=n \
    CONFIG_8139_OLD_RX_RESET=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/8139too.ko
  AUTOLOAD:=$(call AutoProbe,8139too)
endef

define KernelPackage/8139too/description
 Kernel modules for RealTek RTL-8139 PCI Fast Ethernet adapters
endef

$(eval $(call KernelPackage,8139too))


define KernelPackage/8139cp
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=RealTek RTL-8139C+ PCI Fast Ethernet Adapter kernel support
  DEPENDS:=@PCI_SUPPORT +kmod-mii
  KCONFIG:=CONFIG_8139CP
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/8139cp.ko
  AUTOLOAD:=$(call AutoProbe,8139cp)
endef

define KernelPackage/8139cp/description
 Kernel module for RealTek RTL-8139C+ PCI Fast Ethernet adapters
endef

$(eval $(call KernelPackage,8139cp))


define KernelPackage/r8169
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=RealTek RTL-8169 PCI Gigabit Ethernet Adapter kernel support
  DEPENDS:=@PCI_SUPPORT +kmod-mii +r8169-firmware +LINUX_4_19:kmod-phy-realtek
  KCONFIG:=CONFIG_R8169 \
    CONFIG_R8169_NAPI=y \
    CONFIG_R8169_VLAN=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/realtek/r8169.ko
  AUTOLOAD:=$(call AutoProbe,r8169)
endef

define KernelPackage/r8169/description
 Kernel modules for RealTek RTL-8169 PCI Gigabit Ethernet adapters
endef

$(eval $(call KernelPackage,r8169))


define KernelPackage/ne2k-pci
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=ne2k-pci Ethernet Adapter kernel support
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_NE2K_PCI
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/8390/ne2k-pci.ko \
	$(LINUX_DIR)/drivers/net/ethernet/8390/8390.ko
  AUTOLOAD:=$(call AutoProbe,8390 ne2k-pci)
endef

define KernelPackage/ne2k-pci/description
 Kernel modules for NE2000 PCI Ethernet Adapter kernel
endef

$(eval $(call KernelPackage,ne2k-pci))


define KernelPackage/e100
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) PRO/100+ cards kernel support
  DEPENDS:=@PCI_SUPPORT +kmod-mii +e100-firmware
  KCONFIG:=CONFIG_E100
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/e100.ko
  AUTOLOAD:=$(call AutoProbe,e100)
endef

define KernelPackage/e100/description
 Kernel modules for Intel(R) PRO/100+ Ethernet adapters
endef

$(eval $(call KernelPackage,e100))


define KernelPackage/e1000
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) PRO/1000 PCI cards kernel support
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_E1000 \
    CONFIG_E1000_DISABLE_PACKET_SPLIT=n \
    CONFIG_E1000_NAPI=y
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/e1000/e1000.ko
  AUTOLOAD:=$(call AutoLoad,35,e1000)
endef

define KernelPackage/e1000/description
 Kernel modules for Intel(R) PRO/1000 PCI Ethernet adapters.
endef

$(eval $(call KernelPackage,e1000))


define KernelPackage/e1000e
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) PRO/1000 PCIe cards kernel support
  DEPENDS:=@PCIE_SUPPORT +kmod-ptp
  KCONFIG:=CONFIG_E1000E
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/e1000e/e1000e.ko
  AUTOLOAD:=$(call AutoProbe,e1000e)
  MODPARAMS.e1000e:= \
    IntMode=1 \
    InterruptThrottleRate=4,4,4,4,4,4,4,4
endef

define KernelPackage/e1000e/description
 Kernel modules for Intel(R) PRO/1000 PCIe Ethernet adapters.
endef

$(eval $(call KernelPackage,e1000e))


define KernelPackage/igb
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) 82575/82576 PCI-Express Gigabit Ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-i2c-core +kmod-i2c-algo-bit +kmod-ptp +kmod-hwmon-core
  KCONFIG:=CONFIG_IGB \
    CONFIG_IGB_HWMON=y \
    CONFIG_IGB_DCA=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/igb/igb.ko
  AUTOLOAD:=$(call AutoLoad,35,igb)
endef

define KernelPackage/igb/description
 Kernel modules for Intel(R) 82575/82576 PCI-Express Gigabit Ethernet adapters.
endef

$(eval $(call KernelPackage,igb))


define KernelPackage/igbvf
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) 82576 Virtual Function Ethernet support
  DEPENDS:=@PCI_SUPPORT @TARGET_x86 +kmod-i2c-core +kmod-i2c-algo-bit +kmod-ptp
  KCONFIG:=CONFIG_IGBVF \
    CONFIG_IGB_HWMON=y \
    CONFIG_IGB_DCA=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/igbvf/igbvf.ko
  AUTOLOAD:=$(call AutoLoad,35,igbvf)
endef

define KernelPackage/igbvf/description
 Kernel modules for Intel(R) 82576 Virtual Function Ethernet adapters.
endef

$(eval $(call KernelPackage,igbvf))


define KernelPackage/ixgbe
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) 82598/82599 PCI-Express 10 Gigabit Ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-mdio +kmod-ptp +kmod-hwmon-core
  KCONFIG:=CONFIG_IXGBE \
    CONFIG_IXGBE_VXLAN=n \
    CONFIG_IXGBE_HWMON=y \
    CONFIG_IXGBE_DCA=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/ixgbe/ixgbe.ko
  AUTOLOAD:=$(call AutoLoad,35,ixgbe)
endef

define KernelPackage/ixgbe/description
 Kernel modules for Intel(R) 82598/82599 PCI-Express 10 Gigabit Ethernet adapters.
endef

$(eval $(call KernelPackage,ixgbe))


define KernelPackage/ixgbevf
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) 82599 Virtual Function Ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-ixgbe
  KCONFIG:=CONFIG_IXGBEVF \
    CONFIG_IXGBE_VXLAN=n \
    CONFIG_IXGBE_HWMON=y \
    CONFIG_IXGBE_DCA=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/ixgbevf/ixgbevf.ko
  AUTOLOAD:=$(call AutoLoad,35,ixgbevf)
endef

define KernelPackage/ixgbevf/description
 Kernel modules for Intel(R) 82599 Virtual Function Ethernet adapters.
endef

$(eval $(call KernelPackage,ixgbevf))

define KernelPackage/i40e
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) Ethernet Controller XL710 Family support
  DEPENDS:=@PCI_SUPPORT +kmod-mdio +kmod-ptp +kmod-hwmon-core
  KCONFIG:=CONFIG_I40E \
    CONFIG_I40E_DCB=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/i40e/i40e.ko
  AUTOLOAD:=$(call AutoProbe,i40e)
endef

define KernelPackage/i40e/description
 Kernel modules for Intel(R) Ethernet Controller XL710 Family 40 Gigabit Ethernet adapters.
endef

$(eval $(call KernelPackage,i40e))


define KernelPackage/i40evf
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) Ethernet Adaptive Virtual Function support
  DEPENDS:=@PCI_SUPPORT +kmod-i40e
  KCONFIG:=CONFIG_I40EVF
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/i40evf/i40evf.ko
  AUTOLOAD:=$(call AutoProbe,i40evf)
endef

define KernelPackage/i40evf/description
 Kernel modules for Intel(R) Ethernet Controller XL710 Family Virtual Function Ethernet adapters.
endef

$(eval $(call KernelPackage,i40evf))


define KernelPackage/b44
  TITLE:=Broadcom 44xx driver
  KCONFIG:=CONFIG_B44
  DEPENDS:=@PCI_SUPPORT @!TARGET_brcm47xx_mips74k +!TARGET_brcm47xx:kmod-ssb +kmod-mii +kmod-libphy
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/broadcom/b44.ko
  AUTOLOAD:=$(call AutoLoad,19,b44,1)
endef

define KernelPackage/b44/description
 Kernel modules for Broadcom 44xx Ethernet adapters.
endef

$(eval $(call KernelPackage,b44))


define KernelPackage/3c59x
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=3Com 3c590/3c900 series (592/595/597) Vortex/Boomerang
  DEPENDS:=@PCI_SUPPORT +kmod-mii
  KCONFIG:=CONFIG_VORTEX
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/3com/3c59x.ko
  AUTOLOAD:=$(call AutoProbe,3c59x)
endef

define KernelPackage/3c59x/description
 This option enables driver support for a large number of 10mbps and
 10/100mbps EISA, PCI and PCMCIA 3Com Ethernet adapters:
 - "Vortex"    (Fast EtherLink 3c590/3c592/3c595/3c597) EISA and PCI
 - "Boomerang" (EtherLink XL 3c900 or 3c905)            PCI
 - "Cyclone"   (3c540/3c900/3c905/3c980/3c575/3c656)    PCI and Cardbus
 - "Tornado"   (3c905)                                  PCI
 - "Hurricane" (3c555/3cSOHO)                           PCI
endef

$(eval $(call KernelPackage,3c59x))


define KernelPackage/pcnet32
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=AMD PCnet32 PCI support
  DEPENDS:=@(PCI_SUPPORT||TARGET_malta) +kmod-mii
  KCONFIG:=CONFIG_PCNET32
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/amd/pcnet32.ko
  AUTOLOAD:=$(call AutoProbe,pcnet32)
endef

define KernelPackage/pcnet32/description
 Kernel modules for AMD PCnet32 Ethernet adapters
endef

$(eval $(call KernelPackage,pcnet32))


define KernelPackage/tg3
  TITLE:=Broadcom Tigon3 Gigabit Ethernet
  KCONFIG:=CONFIG_TIGON3 \
	CONFIG_TIGON3_HWMON=n
  DEPENDS:=+!TARGET_brcm47xx:kmod-libphy +LINUX_4_9:kmod-hwmon-core +kmod-ptp
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/broadcom/tg3.ko
  AUTOLOAD:=$(call AutoLoad,19,tg3,1)
endef

define KernelPackage/tg3/description
 Kernel modules for Broadcom Tigon3 Gigabit Ethernet adapters
endef

$(eval $(call KernelPackage,tg3))


define KernelPackage/hfcpci
  TITLE:=HFC PCI cards (single port) support for mISDN
  KCONFIG:=CONFIG_MISDN_HFCPCI
  DEPENDS:=+kmod-misdn
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  FILES:=$(LINUX_DIR)/drivers/isdn/hardware/mISDN/hfcpci.ko
  AUTOLOAD:=$(call AutoLoad,31,hfcpci)
endef

define KernelPackage/hfcpci/description
 Kernel modules for Cologne AG's HFC pci cards (single port)
 using the mISDN V2 stack
endef

$(eval $(call KernelPackage,hfcpci))


define KernelPackage/hfcmulti
  TITLE:=HFC multiport cards (HFC-4S/8S/E1) support for mISDN
  KCONFIG:=CONFIG_MISDN_HFCMULTI
  DEPENDS:=+kmod-misdn
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  FILES:=$(LINUX_DIR)/drivers/isdn/hardware/mISDN/hfcmulti.ko
  AUTOLOAD:=$(call AutoLoad,31,hfcmulti)
endef

define KernelPackage/hfcmulti/description
 Kernel modules for Cologne AG's HFC multiport cards (HFC-4S/8S/E1)
 using the mISDN V2 stack
endef

$(eval $(call KernelPackage,hfcmulti))


define KernelPackage/gigaset
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Siemens Gigaset support for isdn4linux
  DEPENDS:=@USB_SUPPORT +kmod-isdn4linux +kmod-lib-crc-ccitt +kmod-usb-core
  URL:=http://gigaset307x.sourceforge.net/
  KCONFIG:= \
    CONFIG_ISDN_DRV_GIGASET \
    CONFIG_GIGASET_BASE \
    CONFIG_GIGASET_M101 \
    CONFIG_GIGASET_M105 \
    CONFIG_GIGASET_UNDOCREQ=y \
    CONFIG_GIGASET_I4L=y
  FILES:= \
    $(LINUX_DIR)/drivers/isdn/gigaset/gigaset.ko \
    $(LINUX_DIR)/drivers/isdn/gigaset/bas_gigaset.ko \
    $(LINUX_DIR)/drivers/isdn/gigaset/ser_gigaset.ko \
    $(LINUX_DIR)/drivers/isdn/gigaset/usb_gigaset.ko
  AUTOLOAD:=$(call AutoProbe,gigaset bas_gigaset ser_gigaset usb_gigaset)
endef

define KernelPackage/gigaset/description
 This driver supports the Siemens Gigaset SX205/255 family of
 ISDN DECT bases, including the predecessors Gigaset 3070/3075
 and 4170/4175 and their T-Com versions Sinus 45isdn and Sinus
 721X.
endef

$(eval $(call KernelPackage,gigaset))


define KernelPackage/macvlan
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=MAC-VLAN support
  KCONFIG:=CONFIG_MACVLAN
  FILES:=$(LINUX_DIR)/drivers/net/macvlan.ko
  AUTOLOAD:=$(call AutoProbe,macvlan)
endef

define KernelPackage/macvlan/description
 A kernel module which allows one to create virtual interfaces that
 map packets to or from specific MAC addresses to a particular interface
endef

$(eval $(call KernelPackage,macvlan))


define KernelPackage/tulip
  TITLE:=Tulip family network device support
  DEPENDS:=@PCI_SUPPORT +kmod-mii
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  KCONFIG:= \
    CONFIG_NET_TULIP=y \
    CONFIG_DE2104X \
    CONFIG_DE2104X_DSL=0 \
    CONFIG_TULIP \
    CONFIG_TULIP_MWI=y \
    CONFIG_TULIP_MMIO=y \
    CONFIG_TULIP_NAPI=y \
    CONFIG_TULIP_NAPI_HW_MITIGATION=y \
    CONFIG_DE4X5=n \
    CONFIG_WINBOND_840 \
    CONFIG_DM9102 \
    CONFIG_ULI526X
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/dec/tulip/tulip.ko \
	$(LINUX_DIR)/drivers/net/ethernet/dec/tulip/de2104x.ko \
	$(LINUX_DIR)/drivers/net/ethernet/dec/tulip/dmfe.ko \
	$(LINUX_DIR)/drivers/net/ethernet/dec/tulip/uli526x.ko \
	$(LINUX_DIR)/drivers/net/ethernet/dec/tulip/winbond-840.ko
  AUTOLOAD:=$(call AutoProbe,tulip)
endef

define KernelPackage/tulip/description
 Kernel modules for the Tulip family of network cards,
 including DECchip Tulip, DIGITAL EtherWORKS, Winbond W89c840,
 Davicom DM910x/DM980x and ULi M526x controller support.
endef

$(eval $(call KernelPackage,tulip))


define KernelPackage/solos-pci
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Solos ADSL2+ multiport modem
  DEPENDS:=@PCI_SUPPORT +kmod-atm
  KCONFIG:=CONFIG_ATM_SOLOS
  FILES:=$(LINUX_DIR)/drivers/atm/solos-pci.ko
  AUTOLOAD:=$(call AutoProbe,solos-pci)
endef

define KernelPackage/solos-pci/description
 Kernel module for Traverse Technologies' Solos PCI cards
 and Geos ADSL2+ x86 motherboard
endef

$(eval $(call KernelPackage,solos-pci))


define KernelPackage/dummy
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Dummy network device
  KCONFIG:=CONFIG_DUMMY
  FILES:=$(LINUX_DIR)/drivers/net/dummy.ko
  AUTOLOAD:=$(call AutoLoad,34,dummy)
endef

define KernelPackage/dummy/description
 The dummy network device
endef

$(eval $(call KernelPackage,dummy))


define KernelPackage/ifb
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intermediate Functional Block support
  KCONFIG:= \
	CONFIG_IFB \
	CONFIG_NET_CLS=y
  FILES:=$(LINUX_DIR)/drivers/net/ifb.ko
  AUTOLOAD:=$(call AutoLoad,34,ifb)
endef

define KernelPackage/ifb/description
  The Intermediate Functional Block
endef

$(eval $(call KernelPackage,ifb))


define KernelPackage/dm9000
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Davicom 9000 Ethernet support
  DEPENDS:=+kmod-mii
  KCONFIG:=CONFIG_DM9000 \
    CONFIG_DM9000_DEBUGLEVEL=4 \
    CONFIG_DM9000_FORCE_SIMPLE_PHY_POLL=y
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/davicom/dm9000.ko
  AUTOLOAD:=$(call AutoLoad,34,dm9000)
endef

define KernelPackage/dm9000/description
 Kernel driver for Davicom 9000 Ethernet adapters.
endef

$(eval $(call KernelPackage,dm9000))


define KernelPackage/forcedeth
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=nForce Ethernet support
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_FORCEDETH
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/nvidia/forcedeth.ko
  AUTOLOAD:=$(call AutoProbe,forcedeth)
endef

define KernelPackage/forcedeth/description
 Kernel driver for Nvidia Ethernet support
endef

$(eval $(call KernelPackage,forcedeth))

define KernelPackage/of-mdio
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=OpenFirmware MDIO support
  DEPENDS:=+kmod-libphy
  KCONFIG:=CONFIG_OF_MDIO
  FILES:= \
	$(LINUX_DIR)/drivers/net/phy/fixed_phy.ko@ge4.9 \
	$(LINUX_DIR)/drivers/of/of_mdio.ko
  AUTOLOAD:=$(call AutoLoad,41,of_mdio)
endef

define KernelPackage/of-mdio/description
 Kernel driver for OpenFirmware MDIO support
endef

$(eval $(call KernelPackage,of-mdio))


define KernelPackage/vmxnet3
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=VMware VMXNET3 ethernet driver
  DEPENDS:=@PCI_SUPPORT
  KCONFIG:=CONFIG_VMXNET3
  FILES:=$(LINUX_DIR)/drivers/net/vmxnet3/vmxnet3.ko
  AUTOLOAD:=$(call AutoLoad,35,vmxnet3)
endef

define KernelPackage/vmxnet3/description
 Kernel modules for VMware VMXNET3 ethernet adapters.
endef

$(eval $(call KernelPackage,vmxnet3))


define KernelPackage/spi-ks8995
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Micrel/Kendin KS8995 Ethernet switch control
  FILES:=$(LINUX_DIR)/drivers/net/phy/spi_ks8995.ko
  KCONFIG:=CONFIG_MICREL_KS8995MA \
	CONFIG_SPI=y \
	CONFIG_SPI_MASTER=y
  AUTOLOAD:=$(call AutoLoad,50,spi_ks8995)
endef

define KernelPackage/spi-ks8995/description
  Kernel module for Micrel/Kendin KS8995 ethernet switch
endef

$(eval $(call KernelPackage,spi-ks8995))


define KernelPackage/ethoc
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Opencore.org ethoc driver
  DEPENDS:=+kmod-libphy
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/ethoc.ko
  KCONFIG:=CONFIG_ETHOC
  AUTOLOAD:=$(call AutoProbe,ethoc)
endef

define KernelPackage/ethoc/description
  Kernel module for the Opencores.org ethernet adapter
endef

$(eval $(call KernelPackage,ethoc))


define KernelPackage/bnx2
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=BCM5706/5708/5709/5716 ethernet adapter driver
  DEPENDS:=@PCI_SUPPORT +bnx2-firmware
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/broadcom/bnx2.ko
  KCONFIG:=CONFIG_BNX2
  AUTOLOAD:=$(call AutoProbe,bnx2)
endef

define KernelPackage/bnx2/description
  Kernel module for the BCM5706/5708/5709/5716 ethernet adapter
endef

$(eval $(call KernelPackage,bnx2))


define KernelPackage/bnx2x
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=QLogic 5771x/578xx 10/20-Gigabit ethernet adapter driver
  DEPENDS:=@PCI_SUPPORT +bnx2x-firmware +kmod-lib-crc32c +kmod-mdio +kmod-ptp +kmod-lib-zlib-inflate
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/broadcom/bnx2x/bnx2x.ko
  KCONFIG:= \
	CONFIG_BNX2X \
	CONFIG_BNX2X_SRIOV=y
  AUTOLOAD:=$(call AutoProbe,bnx2x)
endef

define KernelPackage/bnx2x/description
  QLogic BCM57710/57711/57711E/57712/57712_MF/57800/57800_MF/57810/57810_MF/57840/57840_MF Driver
endef

$(eval $(call KernelPackage,bnx2x))

define KernelPackage/be2net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Broadcom Emulex OneConnect 10Gbps NIC
  DEPENDS:=@PCI_SUPPORT +kmod-hwmon-core
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/emulex/benet/be2net.ko
  KCONFIG:= \
	CONFIG_BE2NET \
	CONFIG_BE2NET_BE2=y \
	CONFIG_BE2NET_BE3=y \
	CONFIG_BE2NET_LANCER=y \
	CONFIG_BE2NET_SKYHAWK=y \
	CONFIG_BE2NET_HWMON=y
  AUTOLOAD:=$(call AutoProbe,be2net)
endef

define KernelPackage/be2net/description
  Broadcom Emulex OneConnect 10Gbit SFP+ support, OneConnect OCe10xxx OCe11xxx OCe14xxx, LightPulse LPe12xxx
endef

$(eval $(call KernelPackage,be2net))

define KernelPackage/mlx4-core
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Mellanox ConnectX(R) mlx4 core Network Driver
  DEPENDS:=@PCI_SUPPORT +kmod-ptp
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlx4/mlx4_core.ko \
	$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlx4/mlx4_en.ko
  KCONFIG:= CONFIG_MLX4_EN \
	CONFIG_MLX4_EN_DCB=n \
	CONFIG_MLX4_CORE=y \
	CONFIG_MLX4_CORE_GEN2=y \
	CONFIG_MLX4_DEBUG=n
  AUTOLOAD:=$(call AutoProbe,mlx4_core mlx4_en)
endef

define KernelPackage/mlx4-core/description
  Supports Mellanox ConnectX-3 series and previous cards
endef

$(eval $(call KernelPackage,mlx4-core))

define KernelPackage/mlx5-core
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Mellanox ConnectX(R) mlx5 core Network Driver
  DEPENDS:=@PCI_SUPPORT +kmod-ptp
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlx5/core/mlx5_core.ko
  KCONFIG:= CONFIG_MLX5_CORE \
	CONFIG_MLX5_CORE_EN=y \
	CONFIG_MLX5_CORE_EN_DCB=n \
	CONFIG_MLX5_CORE_IPOIB=n \
	CONFIG_MLX5_EN_ARFS=n \
	CONFIG_MLX5_EN_IPSEC=n \
	CONFIG_MLX5_EN_RXNFC=y \
	CONFIG_MLX5_EN_TLS=n \
	CONFIG_MLX5_ESWITCH=n \
	CONFIG_MLX5_FPGA=n \
	CONFIG_MLX5_FPGA_IPSEC=n \
	CONFIG_MLX5_FPGA_TLS=n \
	CONFIG_MLX5_MPFS=y \
	CONFIG_MLX5_SW_STEERING=n \
	CONFIG_MLX5_TC_CT=n \
	CONFIG_MLX5_TLS=n
  AUTOLOAD:=$(call AutoProbe,mlx5_core)
endef

define KernelPackage/mlx5-core/description
  Supports Mellanox Connect-IB/ConnectX-4 series and later cards
endef

$(eval $(call KernelPackage,mlx5-core))

define KernelPackage/sfc
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Solarflare SFC9000/SFC9100-family 10Gbps NIC support
  DEPENDS:=@PCI_SUPPORT +kmod-mdio +kmod-i2c-core +kmod-i2c-algo-bit +kmod-hwmon-core +kmod-ptp +kmod-lib-crc32c
# add PCI_IOV
  KCONFIG:= \
    CONFIG_NET_VENDOR_SOLARFLARE=y \
    CONFIG_SFC=y \
    CONFIG_SFC_MTD=y \
    CONFIG_MCDI_MON=y \
    CONFIG_SFC_SRIOV=n \
    CONFIG_SFC_MCDI_LOGGING=n \
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/sfc/sfc.ko
  AUTOLOAD:=$(call AutoProbe, sfc)
endef

define KernelPackage/sfc/description
  Solarflare SFC9000/SFC9100-family 10Gbps NIC support
endef

$(eval $(call KernelPackage,sfc))
