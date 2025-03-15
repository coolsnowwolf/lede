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
  KCONFIG:=CONFIG_PHYLIB \
	   CONFIG_PHYLIB_LEDS=y
  FILES:=$(LINUX_DIR)/drivers/net/phy/libphy.ko
  AUTOLOAD:=$(call AutoLoad,15,libphy,1)
endef

define KernelPackage/libphy/description
 PHY library
endef

$(eval $(call KernelPackage,libphy))


define KernelPackage/phylink
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Model for MAC to optional PHY connection
  DEPENDS:=+kmod-libphy
  KCONFIG:=CONFIG_PHYLINK
  FILES:=$(LINUX_DIR)/drivers/net/phy/phylink.ko
  AUTOLOAD:=$(call AutoLoad,15,phylink,1)
endef

define KernelPackage/phylink/description
 Model for MAC to optional PHY connection
endef

$(eval $(call KernelPackage,phylink))


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


define KernelPackage/mdio-devres
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Supports MDIO device registration
  DEPENDS:=@!LINUX_5_4 +kmod-libphy +(TARGET_armvirt||TARGET_bcm27xx_bcm2708||TARGET_loongarch64||TARGET_malta||TARGET_tegra):kmod-of-mdio
  KCONFIG:=CONFIG_MDIO_DEVRES
  HIDDEN:=1
  FILES:=$(LINUX_DIR)/drivers/net/phy/mdio_devres.ko
  AUTOLOAD:=$(call AutoProbe,mdio-devres)
endef

define KernelPackage/mdio-devres/description
 Supports MDIO device registration
endef

$(eval $(call KernelPackage,mdio-devres))


define KernelPackage/mdio-gpio
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:= Supports GPIO lib-based MDIO busses
  DEPENDS:=+kmod-libphy @GPIO_SUPPORT +(TARGET_armvirt||TARGET_bcm27xx_bcm2708||TARGET_loongarch64||TARGET_malta||TARGET_tegra):kmod-of-mdio
  KCONFIG:= \
	CONFIG_MDIO_BITBANG \
	CONFIG_MDIO_GPIO
  FILES:= \
	$(LINUX_DIR)/drivers/net/phy/mdio-gpio.ko@lt5.10 \
	$(LINUX_DIR)/drivers/net/phy/mdio-bitbang.ko@lt5.10 \
	$(LINUX_DIR)/drivers/net/mdio/mdio-gpio.ko@ge5.10 \
	$(LINUX_DIR)/drivers/net/mdio/mdio-bitbang.ko@ge5.10
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

define KernelPackage/phy-microchip
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Microchip Ethernet PHY driver
   KCONFIG:=CONFIG_MICROCHIP_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/microchip.ko
   AUTOLOAD:=$(call AutoLoad,18,microchip,1)
endef

define KernelPackage/phy-microchip/description
   Supports the LAN88XX PHYs.
endef

$(eval $(call KernelPackage,phy-microchip))


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


define KernelPackage/phylib-qcom
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Qualcomm Ethernet PHY library
   KCONFIG:=CONFIG_QCOM_NET_PHYLIB
   HIDDEN:=1
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/qcom/qcom-phy-lib.ko
   AUTOLOAD:=$(call AutoLoad,17,qcom-phy-lib)
endef

$(eval $(call KernelPackage,phylib-qcom))


define KernelPackage/phy-amd
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=AMD PHY driver
   KCONFIG:=CONFIG_AMD_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/amd.ko
   AUTOLOAD:=$(call AutoProbe,amd,1)
endef

define KernelPackage/phy-amd/description
   Currently supports the AMD and Altima PHYs.
endef

$(eval $(call KernelPackage,phy-amd))


define KernelPackage/phy-at803x
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Qualcomm Atheros 8337 internal PHY
   KCONFIG:=CONFIG_AT803X_PHY
   DEPENDS:=+kmod-phylib-qcom
   FILES:=$(LINUX_DIR)/drivers/net/phy/qcom/at803x.ko
   AUTOLOAD:=$(call AutoLoad,18,at803x,1)
endef

$(eval $(call KernelPackage,phy-at803x))


define KernelPackage/phy-ax88796b
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Asix PHY driver
   KCONFIG:=CONFIG_AX88796B_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/ax88796b.ko
   AUTOLOAD:=$(call AutoProbe,ax88796b)
endef

define KernelPackage/phy-ax88796b/description
   Currently supports the Asix Electronics PHY found in the X-Surf 100
   AX88796B package.
endef

$(eval $(call KernelPackage,phy-ax88796b))


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


define KernelPackage/phy-bcm84881
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Broadcom BCM84881 PHY driver
   KCONFIG:=CONFIG_BCM84881_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/bcm84881.ko
   AUTOLOAD:=$(call AutoLoad,18,bcm84881,1)
endef

define KernelPackage/phy-bcm84881/description
   Supports the Broadcom 84881 PHY.
endef

$(eval $(call KernelPackage,phy-bcm84881))


define KernelPackage/phy-intel-xway
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Intel XWAY PHYs
   KCONFIG:=CONFIG_INTEL_XWAY_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/intel-xway.ko
   AUTOLOAD:=$(call AutoLoad,18,intel-xway,1)
endef

define KernelPackage/phy-intel-xway/description
   Supports the Intel XWAY (former Lantiq) 11G and 22E PHYs.
   These PHYs are marked as standalone chips under the names
   PEF 7061, PEF 7071 and PEF 7072 or integrated into the Intel
   SoCs xRX200, xRX300, xRX330, xRX350 and xRX550.
endef

$(eval $(call KernelPackage,phy-intel-xway))


define KernelPackage/phy-qca83xx
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Qualcomm Atheros QCA833x PHY driver
   KCONFIG:=CONFIG_QCA83XX_PHY
   DEPENDS:=+kmod-phylib-qcom
   FILES:=$(LINUX_DIR)/drivers/net/phy/qcom/qca83xx.ko
   AUTOLOAD:=$(call AutoLoad,18,qca83xx,1)
endef

$(eval $(call KernelPackage,phy-qca83xx))


define KernelPackage/phy-marvell
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Marvell Gigabit Ethernet PHY driver
   KCONFIG:=CONFIG_MARVELL_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/marvell.ko
   AUTOLOAD:=$(call AutoLoad,18,marvell)
endef

define KernelPackage/phy-marvell/description
   Supports Marvell Gigabit Ethernet PHYs:
   * 88E1101
   * 88E1112
   * 88E1111 (incl. Finisar variant)
   * 88E1118
   * 88E1121R
   * 88E1145
   * 88E1149R
   * 88E1240
   * 88E1318S
   * 88E1116R
   * 88E1510
   * 88E1540
   * 88E1545
   * 88E3016
   * 88E6341 family
   * 88E6390 family
   * 88E6393 family
   * 88E1340S
   * 88E1548P
endef

$(eval $(call KernelPackage,phy-marvell))

define KernelPackage/phy-marvell-10g
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Marvell 10 Gigabit Ethernet PHY driver
   KCONFIG:=CONFIG_MARVELL_10G_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/marvell10g.ko
   AUTOLOAD:=$(call AutoLoad,18,marvell10g)
endef

define KernelPackage/phy-marvell/description
   Supports Marvell 10 Gigabit Ethernet PHYs:
   * 88E2110
   * 88E2111
   * 88x3310
   * 88x3340
endef

$(eval $(call KernelPackage,phy-marvell-10g))


define KernelPackage/phy-micrel
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Micrel PHYs
   KCONFIG:=CONFIG_MICREL_PHY
   DEPENDS:=+kmod-libphy +kmod-ptp
   FILES:=$(LINUX_DIR)/drivers/net/phy/micrel.ko
   AUTOLOAD:=$(call AutoLoad,18,micrel,1)
endef

define KernelPackage/phy-micrel/description
   Supports the KSZ9021, VSC8201, KS8001 PHYs.
endef

$(eval $(call KernelPackage,phy-micrel))


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


define KernelPackage/phy-smsc
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=SMSC PHY driver
   KCONFIG:=CONFIG_SMSC_PHY
   DEPENDS:=+kmod-libphy +(LINUX_6_6||LINUX_6_12):kmod-lib-crc16
   FILES:=$(LINUX_DIR)/drivers/net/phy/smsc.ko
   AUTOLOAD:=$(call AutoProbe,smsc)
endef

define KernelPackage/phy-smsc/description
   Currently supports the LAN83C185, LAN8187 and LAN8700 PHYs
endef

$(eval $(call KernelPackage,phy-smsc))


define KernelPackage/phy-vitesse
   SUBMENU:=$(NETWORK_DEVICES_MENU)
   TITLE:=Vitesse PHYs
   KCONFIG:=CONFIG_VITESSE_PHY
   DEPENDS:=+kmod-libphy
   FILES:=$(LINUX_DIR)/drivers/net/phy/vitesse.ko
   AUTOLOAD:=$(call AutoLoad,18,vitesse,1)
endef

define KernelPackage/phy-vitesse/description
   Currently supports the vsc8244
endef

$(eval $(call KernelPackage,phy-vitesse))


define KernelPackage/phy-airoha-en8811h
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Airoha EN8811H 2.5G Ethernet PHY
  DEPENDS:=+en8811h-firmware +kmod-libphy
  KCONFIG:=CONFIG_AIR_EN8811H_PHY
  FILES:= \
   $(LINUX_DIR)/drivers/net/phy/air_en8811h.ko
  AUTOLOAD:=$(call AutoLoad,18,air_en8811h,1)
endef

define KernelPackage/phy-airoha-en8811h/description
  Kernel modules for Airoha EN8811H 2.5G Ethernet PHY
endef

$(eval $(call KernelPackage,phy-airoha-en8811h))


define KernelPackage/phy-aquantia
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Aquantia Ethernet PHYs
  DEPENDS:=+kmod-libphy +kmod-hwmon-core +kmod-lib-crc-ccitt
  KCONFIG:=CONFIG_AQUANTIA_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/aquantia.ko@lt6.1 \
	$(LINUX_DIR)/drivers/net/phy/aquantia/aquantia.ko@ge6.1
  AUTOLOAD:=$(call AutoLoad,18,aquantia,1)
endef

define KernelPackage/phy-aquantia/description
  Kernel modules for Aquantia Ethernet PHYs
endef

$(eval $(call KernelPackage,phy-aquantia))

define KernelPackage/dsa
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Distributed Switch Architecture support
  DEPENDS:=+kmod-mdio-devres +kmod-net-selftests +kmod-phylink
  KCONFIG:=CONFIG_NET_DSA
  FILES:=$(LINUX_DIR)/net/dsa/dsa_core.ko
endef

define KernelPackage/dsa/description
  Kernel module support for Distributed Switch Architecture
endef

$(eval $(call KernelPackage,dsa))

define KernelPackage/dsa-tag-dsa
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell DSA type DSA and EDSA taggers
  DEPENDS:=+kmod-dsa
  KCONFIG:= CONFIG_NET_DSA_TAG_DSA_COMMON \
	CONFIG_NET_DSA_TAG_DSA \
	CONFIG_NET_DSA_TAG_EDSA
  FILES:=$(LINUX_DIR)/net/dsa/tag_dsa.ko
  AUTOLOAD:=$(call AutoLoad,40,tag_dsa,1)
endef

define KernelPackage/dsa-tag-dsa/description
  Kernel modules for Marvell DSA and EDSA tagging
endef

$(eval $(call KernelPackage,dsa-tag-dsa))

define KernelPackage/dsa-mv88e6xxx
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell MV88E6XXX DSA Switch
  DEPENDS:=+kmod-dsa +kmod-ptp +kmod-phy-marvell +kmod-dsa-tag-dsa
  KCONFIG:=CONFIG_NET_DSA_MV88E6XXX \
	CONFIG_NET_DSA_MV88E6XXX_PTP=y
  FILES:=$(LINUX_DIR)/drivers/net/dsa/mv88e6xxx/mv88e6xxx.ko
  AUTOLOAD:=$(call AutoLoad,41,mv88e6xxx,1)
endef

define KernelPackage/dsa-mv88e6xxx/description
  Kernel modules for MV88E6XXX DSA switches
endef

$(eval $(call KernelPackage,dsa-mv88e6xxx))

define KernelPackage/dsa-qca8k
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Qualcomm Atheros QCA8xxx switch family DSA support
  DEPENDS:=+kmod-dsa +kmod-regmap-core
  KCONFIG:= \
	CONFIG_NET_DSA_QCA8K \
	CONFIG_NET_DSA_QCA8K_LEDS_SUPPORT=y \
	CONFIG_NET_DSA_TAG_QCA
  FILES:= \
	$(LINUX_DIR)/drivers/net/dsa/qca/qca8k.ko \
	$(LINUX_DIR)/net/dsa/tag_qca.ko
  AUTOLOAD:=$(call AutoLoad,42,qca8k,1)
endef

define KernelPackage/dsa-qca8k/description
  DSA based kernel modules for the Qualcomm Atheros QCA8xxx switch family
endef

$(eval $(call KernelPackage,dsa-qca8k))


define KernelPackage/dsa-realtek
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek common module RTL83xx DSA switch family
  DEPENDS:=@LINUX_6_6||LINUX_6_12 +kmod-dsa +kmod-phy-realtek +kmod-regmap-core
  KCONFIG:= \
	CONFIG_NET_DSA_REALTEK \
	CONFIG_NET_DSA_REALTEK_MDIO=y \
	CONFIG_NET_DSA_REALTEK_SMI=y
  FILES:= $(LINUX_DIR)/drivers/net/dsa/realtek/realtek_dsa.ko
endef

define KernelPackage/dsa-realtek/description
  Common kernel module for Realtek RTL83xx DSA switch family
endef

$(eval $(call KernelPackage,dsa-realtek))


define KernelPackage/dsa-rtl8366rb
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8365MB switch DSA support
  DEPENDS:=+kmod-dsa-realtek @!TARGET_x86 @!TARGET_bcm47xx @!TARGET_uml
  KCONFIG:= \
	CONFIG_NET_DSA_REALTEK_RTL8366RB \
	CONFIG_NET_DSA_TAG_RTL4_A
  FILES:= \
	$(LINUX_DIR)/drivers/net/dsa/realtek/rtl8366.ko \
	$(LINUX_DIR)/net/dsa/tag_rtl4_a.ko
  AUTOLOAD:=$(call AutoLoad,42,rtl8366,1)
endef

define KernelPackage/dsa-rtl8366rb/description
  DSA based kernel modules for the Realtek RTL8366RB switch family
endef

$(eval $(call KernelPackage,dsa-rtl8366rb))


define KernelPackage/dsa-rtl8365mb
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8365MB switch DSA support
  DEPENDS:=+kmod-dsa-realtek @!TARGET_x86 @!TARGET_bcm47xx @!TARGET_uml
  KCONFIG:= \
	CONFIG_NET_DSA_REALTEK_RTL8365MB \
	CONFIG_NET_DSA_TAG_RTL8_4
  FILES:= \
	$(LINUX_DIR)/drivers/net/dsa/realtek/rtl8365mb.ko \
	$(LINUX_DIR)/net/dsa/tag_rtl8_4.ko
  AUTOLOAD:=$(call AutoLoad,42,rtl8365mb,1)
endef

define KernelPackage/dsa-rtl8365mb/description
  DSA based kernel modules for the Realtek RTL8365MB switch family
endef

$(eval $(call KernelPackage,dsa-rtl8365mb))


define KernelPackage/swconfig
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=switch configuration API
  DEPENDS:=+kmod-libphy
  KCONFIG:=CONFIG_SWCONFIG \
	   CONFIG_SWCONFIG_LEDS=y
  FILES:=$(LINUX_DIR)/drivers/net/phy/swconfig.ko
  AUTOLOAD:=$(call AutoLoad,41,swconfig)
endef

define KernelPackage/swconfig/description
 Switch configuration API module
endef

$(eval $(call KernelPackage,swconfig))

define KernelPackage/switch-bcm53xx
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Broadcom bcm53xx switch support
  DEPENDS:=+kmod-swconfig
  KCONFIG:=CONFIG_SWCONFIG_B53
  FILES:=$(LINUX_DIR)/drivers/net/phy/b53/b53_common.ko
  AUTOLOAD:=$(call AutoLoad,42,b53_common)
endef

define KernelPackage/switch-bcm53xx/description
  Broadcom bcm53xx switch support
endef

$(eval $(call KernelPackage,switch-bcm53xx))

define KernelPackage/switch-bcm53xx-mdio
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Broadcom bcm53xx switch MDIO support
  DEPENDS:=+kmod-switch-bcm53xx
  KCONFIG:=CONFIG_SWCONFIG_B53_PHY_DRIVER
  FILES:=$(LINUX_DIR)/drivers/net/phy/b53/b53_mdio.ko
  AUTOLOAD:=$(call AutoLoad,42,b53_mdio)
endef

define KernelPackage/switch-bcm53xx-mdio/description
  Broadcom bcm53xx switch MDIO support
endef

$(eval $(call KernelPackage,switch-bcm53xx-mdio))


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
  DEPENDS:=@GPIO_SUPPORT +kmod-swconfig +(TARGET_armvirt||TARGET_bcm27xx_bcm2708||TARGET_loongarch64||TARGET_malta||TARGET_tegra):kmod-of-mdio
  KCONFIG:=CONFIG_RTL8366_SMI
  FILES:=$(LINUX_DIR)/drivers/net/phy/rtl8366_smi.ko
  AUTOLOAD:=$(call AutoLoad,42,rtl8366_smi,1)
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
  AUTOLOAD:=$(call AutoLoad,43,rtl8366rb,1)
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
  AUTOLOAD:=$(call AutoLoad,43,rtl8366s,1)
endef

define KernelPackage/switch-rtl8366s/description
 Realtek RTL8366S switch support
endef

$(eval $(call KernelPackage,switch-rtl8366s))


define KernelPackage/switch-rtl8367
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8367 switch support
  DEPENDS:=+kmod-switch-rtl8366-smi
  KCONFIG:=CONFIG_RTL8367_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/rtl8367.ko
  AUTOLOAD:=$(call AutoLoad,43,rtl8367,1)
endef

define KernelPackage/switch-rtl8367/description
 Realtek RTL8367 switch support
endef

$(eval $(call KernelPackage,switch-rtl8367))


define KernelPackage/switch-rtl8367b
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Realtek RTL8367R/B switch support
  DEPENDS:=+kmod-switch-rtl8366-smi
  KCONFIG:=CONFIG_RTL8367B_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/rtl8367b.ko
  AUTOLOAD:=$(call AutoLoad,43,rtl8367b,1)
endef

define KernelPackage/switch-rtl8367b/description
 Realtek RTL8367R/B switch support
endef

$(eval $(call KernelPackage,switch-rtl8367b))


define KernelPackage/switch-ar8xxx
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Atheros AR8216/8327 switch support
  DEPENDS:=+kmod-swconfig +!LINUX_5_4:kmod-mdio-devres
  KCONFIG:=CONFIG_AR8216_PHY \
	   CONFIG_AR8216_PHY_LEDS=y
  FILES:=$(LINUX_DIR)/drivers/net/phy/ar8xxx.ko
  AUTOLOAD:=$(call AutoLoad,43,ar8xxx,1)
endef

define KernelPackage/switch-ar8xxx/description
 Atheros AR8216/8327 switch support
endef

$(eval $(call KernelPackage,switch-ar8xxx))


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
  DEPENDS:=@PCI_SUPPORT +kmod-mii +r8169-firmware +kmod-phy-realtek +!LINUX_5_4:kmod-mdio-devres
  KCONFIG:= \
    CONFIG_R8169 \
    CONFIG_R8169_LEDS=y@ge6.6
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

define KernelPackage/libie
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel Ethernet library
  DEPENDS:=@PCI_SUPPORT +kmod-i2c-core +kmod-i2c-algo-bit +kmod-ptp +kmod-hwmon-core
  KCONFIG:=CONFIG_LIBIE
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/libie/libie.ko
  AUTOLOAD:=$(call AutoLoad,30,libie,1)
endef

define KernelPackage/libie/description
 Kernel modules for libie (Intel Ethernet library) common library
endef

$(eval $(call KernelPackage,libie))

define KernelPackage/libeth
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel Ethernet PHY library
  DEPENDS:=@PCI_SUPPORT +kmod-i2c-core +kmod-i2c-algo-bit +kmod-ptp +kmod-hwmon-core
  KCONFIG:=CONFIG_LIBIE
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/libeth/libeth.ko
  AUTOLOAD:=$(call AutoLoad,30,libeth,1)
endef

define KernelPackage/libeth/description
 Kernel modules for libeth (Intel Ethernet library) common library
endef

$(eval $(call KernelPackage,libeth))

define KernelPackage/igb
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) 82575/82576 PCI-Express Gigabit Ethernet support
  DEPENDS:=@PCI_SUPPORT +kmod-i2c-core +kmod-i2c-algo-bit +kmod-ptp +kmod-hwmon-core
  KCONFIG:=CONFIG_IGB \
    CONFIG_IGB_HWMON=y \
    CONFIG_IGB_DCA=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/igb/igb.ko
  AUTOLOAD:=$(call AutoLoad,35,igb,1)
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
  DEPENDS:=@PCI_SUPPORT +kmod-mdio +kmod-ptp +kmod-hwmon-core +kmod-libphy +!LINUX_5_4:kmod-mdio-devres
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
  DEPENDS:=@PCI_SUPPORT +kmod-mdio +kmod-ptp +kmod-hwmon-core +kmod-libphy +LINUX_6_12:kmod-libie
  KCONFIG:=CONFIG_I40E \
    CONFIG_I40E_VXLAN=n \
    CONFIG_I40E_HWMON=y \
    CONFIG_I40E_DCA=n
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/i40e/i40e.ko
  AUTOLOAD:=$(call AutoProbe,i40e)
endef

define KernelPackage/i40e/description
 Kernel modules for Intel(R) Ethernet Controller XL710 Family 40 Gigabit Ethernet adapters.
endef

$(eval $(call KernelPackage,i40e))


define KernelPackage/iavf
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) Ethernet Adaptive Virtual Function support
  DEPENDS:=@PCI_SUPPORT +LINUX_6_12:kmod-libie +LINUX_6_12:kmod-libeth
  KCONFIG:= \
       CONFIG_I40EVF \
       CONFIG_IAVF
  FILES:= \
       $(LINUX_DIR)/drivers/net/ethernet/intel/iavf/iavf.ko
  AUTOLOAD:=$(call AutoProbe,i40evf iavf)
  AUTOLOAD:=$(call AutoProbe,iavf)
endef

define KernelPackage/iavf/description
 Kernel modules for Intel XL710,
	  X710, X722, XXV710, and all devices advertising support for
	  Intel Ethernet Adaptive Virtual Function devices.
endef

$(eval $(call KernelPackage,iavf))


define KernelPackage/b44
  TITLE:=Broadcom 44xx driver
  KCONFIG:=CONFIG_B44
  DEPENDS:=@PCI_SUPPORT @!TARGET_bcm47xx_mips74k +!TARGET_bcm47xx:kmod-ssb +kmod-mii +kmod-libphy
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
  DEPENDS:=@PCI_SUPPORT +!TARGET_bcm47xx:kmod-libphy +kmod-ptp
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
  DEPENDS:=@PCI_SUPPORT +kmod-misdn
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
  DEPENDS:=@PCI_SUPPORT +kmod-misdn
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  FILES:=$(LINUX_DIR)/drivers/isdn/hardware/mISDN/hfcmulti.ko
  AUTOLOAD:=$(call AutoLoad,31,hfcmulti)
endef

define KernelPackage/hfcmulti/description
 Kernel modules for Cologne AG's HFC multiport cards (HFC-4S/8S/E1)
 using the mISDN V2 stack
endef

$(eval $(call KernelPackage,hfcmulti))


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


define KernelPackage/ipvlan
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=IP-VLAN support
  KCONFIG:=CONFIG_IPVLAN
  FILES:=$(LINUX_DIR)/drivers/net/ipvlan/ipvlan.ko
  AUTOLOAD:=$(call AutoProbe,ipvlan)
endef

define KernelPackage/ipvlan/description
 A kernel module which allows one to create virtual interfaces that
 map packets to or from specific IP addresses to a particular interface
endef

$(eval $(call KernelPackage,ipvlan))


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
  MODPARAMS.ifb:=numifbs=0
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

define KernelPackage/fixed-phy
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=MDIO Bus/PHY emulation with fixed speed/link PHYs
  DEPENDS:=+kmod-libphy
  KCONFIG:=CONFIG_FIXED_PHY
  FILES:=$(LINUX_DIR)/drivers/net/phy/fixed_phy.ko
  AUTOLOAD:=$(call AutoProbe,fixed_phy)
endef

define KernelPackage/fixed-phy/description
 Kernel driver for "fixed" MDIO Bus to cover the boards
 and devices that use PHYs that are not connected to the real MDIO bus.
endef

$(eval $(call KernelPackage,fixed-phy))

define KernelPackage/of-mdio
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=OpenFirmware MDIO support
  DEPENDS:=+kmod-libphy +kmod-fixed-phy @!TARGET_x86
  KCONFIG:=CONFIG_OF_MDIO
  FILES:= \
	$(LINUX_DIR)/drivers/net/mdio/of_mdio.ko \
	$(LINUX_DIR)/drivers/net/mdio/fwnode_mdio.ko@ge5.15
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

define KernelPackage/bnxt-en
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Broadcom NetXtreme-C/E network driver
  DEPENDS:=@PCI_SUPPORT +kmod-hwmon-core +kmod-lib-crc32c +kmod-mdio +kmod-ptp
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/broadcom/bnxt/bnxt_en.ko
  KCONFIG:= \
	  CONFIG_BNXT \
	  CONFIG_BNXT_SRIOV=y \
	  CONFIG_BNXT_FLOWER_OFFLOAD=y \
	  CONFIG_BNXT_DCB=n \
	  CONFIG_BNXT_HWMON=y
  AUTOLOAD:=$(call AutoProbe,bnxt_en)
endef

define KernelPackage/bnxt-en/description
  Supports Broadcom NetXtreme-C/E based Ethernet NICs including:
  * BCM573xx
  * BCM574xx
endef

$(eval $(call KernelPackage,bnxt-en))

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
  AUTOLOAD:=$(call AutoLoad,36,mlx4_core mlx4_en,1)
endef

define KernelPackage/mlx4-core/description
  Supports Mellanox ConnectX-3 series and previous cards
endef

$(eval $(call KernelPackage,mlx4-core))

define KernelPackage/mlx5-core
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Mellanox ConnectX(R) mlx5 core Network Driver
  DEPENDS:=@PCI_SUPPORT +kmod-ptp +kmod-mlxfw +(LINUX_6_6||LINUX_6_12):kmod-hwmon-core
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
	CONFIG_MLX5_TLS=n \
	CONFIG_MLX5_VFIO_PCI=n
  AUTOLOAD:=$(call AutoLoad,36,mlx5_core,1)
endef

define KernelPackage/mlx5-core/description
  Supports Mellanox Connect-IB/ConnectX-4 series and later cards
endef

$(eval $(call KernelPackage,mlx5-core))


define KernelPackage/mlxfw
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Mellanox Technologies firmware flash module
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlxfw/mlxfw.ko
  KCONFIG:=CONFIG_MLXFW
  AUTOLOAD:=$(call AutoProbe,mlxfw)
endef

define KernelPackage/mlxfw/description
  This driver supports Mellanox Technologies Firmware
  flashing common logic.
endef

$(eval $(call KernelPackage,mlxfw))


define KernelPackage/mlxsw-core
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Mellanox Technologies Switch ASICs support
  DEPENDS:=@TARGET_x86_64 +kmod-mlxfw +kmod-hwmon-core
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlxsw/mlxsw_core.ko
  KCONFIG:= \
  CONFIG_MLXSW_CORE \
  CONFIG_MLXSW_CORE_HWMON=y \
  CONFIG_MLXSW_CORE_THERMAL=y
  AUTOLOAD:=$(call AutoProbe,mlxsw_core)
endef

define KernelPackage/mlxsw-core/description
  This driver supports Mellanox Technologies Switch ASICs family.
endef

$(eval $(call KernelPackage,mlxsw-core))


define KernelPackage/mlxsw-i2c
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=I2C bus implementation for Mellanox Technologies Switch ASICs
  DEPENDS:=+kmod-mlxsw-core +kmod-i2c-core
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlxsw/mlxsw_i2c.ko
  KCONFIG:=CONFIG_MLXSW_I2C
  AUTOLOAD:=$(call AutoProbe,mlxsw_i2c)
endef

define KernelPackage/mlxsw-i2c/description
  This is I2C bus implementation for Mellanox Technologies Switch ASICs.
endef

$(eval $(call KernelPackage,mlxsw-i2c))


define KernelPackage/mlxsw-minimal
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Mellanox Technologies minimal I2C support
  DEPENDS:=+kmod-mlxsw-i2c
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlxsw/mlxsw_minimal.ko
  KCONFIG:=CONFIG_MLXSW_MINIMAL
  AUTOLOAD:=$(call AutoProbe,mlxsw_minimal)
endef

define KernelPackage/mlxsw-minimal/description
  This driver supports I2C access for Mellanox Technologies Switch
  ASICs.
endef

$(eval $(call KernelPackage,mlxsw-minimal))


define KernelPackage/mlxsw-pci
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=PCI bus implementation for Mellanox Technologies Switch ASICs
  DEPENDS:=@PCI_SUPPORT +kmod-mlxsw-core
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlxsw/mlxsw_pci.ko
  KCONFIG:=CONFIG_MLXSW_PCI
  AUTOLOAD:=$(call AutoProbe,mlxsw_pci)
endef

define KernelPackage/mlxsw-pci/description
  This is PCI bus implementation for Mellanox Technologies Switch ASICs.
endef

$(eval $(call KernelPackage,mlxsw-pci))


define KernelPackage/mlxsw-spectrum
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Mellanox Technologies Spectrum family support
  DEPENDS:= \
  +kmod-mlxsw-pci +kmod-lib-objagg +kmod-lib-parman \
  +kmod-ip6-tunnel +kmod-ptp +kmod-sched-act-sample +kmod-vxlan
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/mellanox/mlxsw/mlxsw_spectrum.ko
  KCONFIG:= \
  CONFIG_MLXSW_SPECTRUM \
  CONFIG_MLXSW_SPECTRUM_DCB=y \
  CONFIG_NET_SWITCHDEV=y \
  CONFIG_DCB=y
  AUTOLOAD:=$(call AutoProbe,mlxsw_spectrum)
endef

define KernelPackage/mlxsw-spectrum/description
  This driver supports Mellanox Technologies
  Spectrum/Spectrum-2/Spectrum-3/Spectrum-4 Ethernet Switch ASICs.
endef

$(eval $(call KernelPackage,mlxsw-spectrum))


define KernelPackage/net-selftests
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  DEPENDS:=+kmod-libphy
  TITLE:=Network generic selftest support
  KCONFIG:=CONFIG_NET_SELFTESTS
  FILES:=$(LINUX_DIR)/net/core/selftests.ko
  AUTOLOAD:=$(call AutoLoad,99,selftests)
endef

define KernelPackage/net-selftests/description
  Kernel modules for the generic selftest support
endef

$(eval $(call KernelPackage,net-selftests))


define KernelPackage/qlcnic
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@PCI_SUPPORT +kmod-hwmon-core
  TITLE:=QLogic QLE8240 and QLE8242 device support
  KCONFIG:= \
	CONFIG_QLCNIC \
	CONFIG_QLCNIC_HWMON=y \
	CONFIG_QLCNIC_SRIOV=y
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/qlogic/qlcnic/qlcnic.ko
  AUTOLOAD:=$(call AutoProbe,qlcnic)
endef

define KernelPackage/qlcnic/description
  This driver supports QLogic QLE8240 and QLE8242 Converged Ethernet
  devices.
endef

$(eval $(call KernelPackage,qlcnic))

define KernelPackage/qede
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@PCI_SUPPORT +kmod-ptp +kmod-lib-crc8 +kmod-lib-zlib-inflate
  TITLE:=QLogic FastLinQ 10/25/40/100Gb Ethernet NIC device support
  KCONFIG:= \
	CONFIG_QED \
	CONFIG_QED_SRIOV=y \
	CONFIG_QEDE \
	CONFIG_QEDF=n \
	CONFIG_QEDI=n
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/qlogic/qed/qed.ko \
	$(LINUX_DIR)/drivers/net/ethernet/qlogic/qede/qede.ko
  AUTOLOAD:=$(call AutoProbe,qed qede)
endef

define KernelPackage/qede/description
  This driver supports QLogic FastLinQ 25/40/100Gb Ethernet NIC
  devices.
endef

$(eval $(call KernelPackage,qede))


define KernelPackage/sfp
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=SFP cage support
  DEPENDS:=+kmod-i2c-core +kmod-hwmon-core +kmod-phylink
  KCONFIG:= \
	CONFIG_SFP \
	CONFIG_MDIO_I2C
  FILES:= \
	$(LINUX_DIR)/drivers/net/phy/sfp.ko \
	$(LINUX_DIR)/drivers/net/phy/mdio-i2c.ko@lt5.10 \
	$(LINUX_DIR)/drivers/net/mdio/mdio-i2c.ko@ge5.10
  AUTOLOAD:=$(call AutoProbe,mdio-i2c sfp)
endef

define KernelPackage/sfp/description
 Kernel module to support SFP cages
endef

$(eval $(call KernelPackage,sfp))

define KernelPackage/igc
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) Ethernet Controller I225 Series support
  DEPENDS:=@PCI_SUPPORT +kmod-ptp
  KCONFIG:=CONFIG_IGC
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/igc/igc.ko
  AUTOLOAD:=$(call AutoLoad,34,igc,1)
endef

define KernelPackage/igc/description
  Kernel modules for Intel(R) Ethernet Controller I225 Series
endef

$(eval $(call KernelPackage,igc))

define KernelPackage/ice
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intel(R) Ethernet Controller E810 Series support
  DEPENDS:=@PCI_SUPPORT +kmod-ptp
  KCONFIG:=CONFIG_ICE
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/intel/ice/ice.ko
  AUTOLOAD:=$(call AutoProbe,ice)
endef

define KernelPackage/ice/description
  Kernel modules for Intel(R) Ethernet Controller E810 Series
endef

$(eval $(call KernelPackage,ice))

define KernelPackage/sfc
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Solarflare SFC9000/SFC9100/EF100-family support
  DEPENDS:=@PCI_SUPPORT +kmod-mdio +kmod-lib-crc32c +kmod-ptp +kmod-hwmon-core
  KCONFIG:= \
	CONFIG_SFC \
	CONFIG_SFC_MTD=y \
	CONFIG_SFC_MCDI_MON=y \
	CONFIG_SFC_MCDI_LOGGING=y \
	CONFIG_SFC_SRIOV=y
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/sfc/sfc.ko
  AUTOLOAD:=$(call AutoProbe,sfc)
endef

define KernelPackage/sfc/description
  Solarflare SFC9000/SFC9100/EF100-family support
  Solarflare EF100 support requires at least kernel version 5.9
endef

$(eval $(call KernelPackage,sfc))

define KernelPackage/sfc-falcon
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Solarflare SFC4000 support
  DEPENDS:=@PCI_SUPPORT +kmod-mdio +kmod-lib-crc32c +kmod-i2c-algo-bit
  KCONFIG:= \
	CONFIG_SFC_FALCON \
	CONFIG_SFC_FALCON_MTD=y
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/sfc/falcon/sfc-falcon.ko
  AUTOLOAD:=$(call AutoProbe,sfc-falcon)
endef

define KernelPackage/sfc-falcon/description
  Solarflare SFC4000 support
endef

$(eval $(call KernelPackage,sfc-falcon))


define KernelPackage/wwan
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=WWAN Driver Core
  DEPENDS:=@(LINUX_5_15||LINUX_6_1||LINUX_6_6||LINUX_6_12)
  KCONFIG:= \
  CONFIG_WWAN \
  CONFIG_WWAN_DEBUGFS=y@ge5.17
  FILES:=$(LINUX_DIR)/drivers/net/wwan/wwan.ko
  AUTOLOAD:=$(call AutoProbe,wwan)
endef

define KernelPackage/wwan/description
 This driver provides a common framework for WWAN drivers.
endef

$(eval $(call KernelPackage,wwan))


define KernelPackage/mhi-net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=MHI Network Device
  DEPENDS:=@PCI_SUPPORT +kmod-mhi-bus
  KCONFIG:=CONFIG_MHI_NET
  FILES:=$(LINUX_DIR)/drivers/net/mhi_net.ko
  AUTOLOAD:=$(call AutoProbe,mhi_net)
endef

define KernelPackage/mhi-net/description
 Driver for MHI network interface
endef

$(eval $(call KernelPackage,mhi-net))

define KernelPackage/mhi-wwan-ctrl
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=MHI WWAN Control
  DEPENDS:=@PCI_SUPPORT +kmod-mhi-bus +kmod-wwan
  KCONFIG:=CONFIG_MHI_WWAN_CTRL
  FILES:=$(LINUX_DIR)/drivers/net/wwan/mhi_wwan_ctrl.ko
  AUTOLOAD:=$(call AutoProbe,mhi_wwan_ctrl)
endef

define KernelPackage/mhi-wwan-ctrl/description
 Driver for MHI WWAN Control
 This exposes all modem control ports like AT, MBIM, QMI, DIAG, ..
endef

$(eval $(call KernelPackage,mhi-wwan-ctrl))

define KernelPackage/mhi-wwan-mbim
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=MHI MBIM
  DEPENDS:=@PCI_SUPPORT +kmod-mhi-bus +kmod-wwan
  KCONFIG:=CONFIG_MHI_WWAN_MBIM
  FILES:=$(LINUX_DIR)/drivers/net/wwan/mhi_wwan_mbim.ko
  AUTOLOAD:=$(call AutoProbe,mhi_wwan_mbim)
endef

define KernelPackage/mhi-wwan-mbim/description
 Driver for MHI MBIM
 This implements MBIM over MHI
endef

$(eval $(call KernelPackage,mhi-wwan-mbim))


define KernelPackage/mtk-t7xx
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=MediaTek T7xx 5G modem
  DEPENDS:=@(LINUX_6_1||LINUX_6_6||LINUX_6_12) @PCI_SUPPORT +kmod-wwan
  KCONFIG:=CONFIG_MTK_T7XX
  FILES:=$(LINUX_DIR)/drivers/net/wwan/t7xx/mtk_t7xx.ko
  AUTOLOAD:=$(call AutoProbe,mtk_t7xx)
endef

define KernelPackage/mtk-t7xx/description
 Driver for MediaTek PCIe 5G WWAN modem T7xx device
endef

$(eval $(call KernelPackage,mtk-t7xx))


define KernelPackage/atlantic
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Aquantia AQtion 10Gbps Ethernet NIC
  DEPENDS:=@PCI_SUPPORT +kmod-ptp +kmod-hwmon-core +kmod-macsec
  KCONFIG:=CONFIG_AQTION
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/aquantia/atlantic/atlantic.ko
  AUTOLOAD:=$(call AutoLoad,36,atlantic)
endef

define KernelPackage/atlantic/description
  Kernel modules for Aquantia AQtion 10Gbps Ethernet NIC
endef

$(eval $(call KernelPackage,atlantic))

define KernelPackage/amazon-ena
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Elastic Network Adapter (for Amazon AWS)
  DEPENDS:=@TARGET_x86_64||TARGET_armvirt_64
  KCONFIG:=CONFIG_ENA_ETHERNET
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/amazon/ena/ena.ko
  AUTOLOAD:=$(call AutoLoad,12,ena)
endef

define KernelPackage/amazon-ena/description
  This driver supports Elastic Network Adapter (ENA)
  used by Amazon AWS T3 (2018) and later instances.
endef

$(eval $(call KernelPackage,amazon-ena))

define KernelPackage/lan743x
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Microchip LAN743x PCI Express Gigabit Ethernet NIC
  DEPENDS:=@PCI_SUPPORT +kmod-ptp \
	+!LINUX_5_4:kmod-mdio-devres +(LINUX_6_6||LINUX_6_12):kmod-fixed-phy
  KCONFIG:=CONFIG_LAN743X
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/microchip/lan743x.ko
  AUTOLOAD:=$(call AutoProbe,lan743x)
endef

define KernelPackage/lan743x/description
  Kernel module for Microchip LAN743x PCI Express Gigabit Ethernet NIC
endef

$(eval $(call KernelPackage,lan743x))

