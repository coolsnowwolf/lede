define KernelPackage/acpi-mdio
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=ACPI MDIO support
  DEPENDS:=@(TARGET_armvirt_64) +kmod-libphy +kmod-mdio-devres
  KCONFIG:=CONFIG_ACPI_MDIO
  FILES:=$(LINUX_DIR)/drivers/net/mdio/acpi_mdio.ko
  AUTOLOAD:=$(call AutoLoad,11,acpi_mdio)
endef

define KernelPackage/acpi-mdio/description
 Kernel driver for ACPI MDIO support
endef

$(eval $(call KernelPackage,acpi-mdio))

define KernelPackage/pcs-xpcs
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Synopsis DesignWare PCS driver
  DEPENDS:=@(TARGET_armvirt_64)
  KCONFIG:=CONFIG_PCS_XPCS
  FILES:=$(LINUX_DIR)/drivers/net/pcs/pcs_xpcs.ko
  AUTOLOAD:=$(call AutoLoad,20,pcs_xpcs)
endef

$(eval $(call KernelPackage,pcs-xpcs))

define KernelPackage/fsl-fec
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@(TARGET_armvirt_64) +kmod-libphy +kmod-of-mdio \
    +kmod-ptp +kmod-net-selftests
  TITLE:=NXP (Freescale) FEC Ethernet controller (i.MX)
  KCONFIG:=CONFIG_FEC
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/freescale/fec.ko
  AUTOLOAD:=$(call AutoLoad,35,fec)
endef

$(eval $(call KernelPackage,fsl-fec))

define KernelPackage/fsl-xgmac-mdio
  SUBMENU=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@(TARGET_armvirt_64) +kmod-libphy +kmod-of-mdio +kmod-acpi-mdio
  TITLE:=NXP (Freescale) MDIO bus
  KCONFIG:=CONFIG_FSL_XGMAC_MDIO
  FILES=$(LINUX_DIR)/drivers/net/ethernet/freescale/xgmac_mdio.ko
  AUTOLOAD=$(call AutoLoad,30,xgmac_mdio)
endef

$(eval $(call KernelPackage,fsl-xgmac-mdio))

define KernelPackage/fsl-dpaa2-net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=NXP DPAA2 Ethernet
  DEPENDS:=@(TARGET_armvirt_64) +kmod-fsl-xgmac-mdio +kmod-phylink
  KCONFIG:=CONFIG_FSL_MC_BUS=y \
    CONFIG_FSL_MC_DPIO \
    CONFIG_FSL_MC_UAPI_SUPPORT=y \
    CONFIG_FSL_DPAA2_ETH
  FILES:= \
  $(LINUX_DIR)/drivers/net/ethernet/freescale/dpaa2/fsl-dpaa2-eth.ko \
  $(LINUX_DIR)/drivers/soc/fsl/dpio/fsl-mc-dpio.ko \
  $(LINUX_DIR)/drivers/net/pcs/pcs-lynx.ko
  AUTOLOAD=$(call AutoLoad,35,fsl-dpaa2-eth)
endef

$(eval $(call KernelPackage,fsl-dpaa2-net))

define KernelPackage/fsl-dpaa2-console
  SUBMENU:=$(MISC_DEVICES_MENU)
  TITLE:=NXP DPAA2 Debug console
  DEPENDS:=@(TARGET_armvirt_64)
  KCONFIG:=CONFIG_DPAA2_CONSOLE
  FILES=$(LINUX_DIR)/drivers/soc/fsl/dpaa2-console.ko
  AUTOLOAD=$(call AutoLoad,40,dpaa2-console)
endef

define KernelPackage/fsl-dpaa2-console/description
  Kernel modules for the NXP DPAA2 debug consoles
  (Management Complex and AIOP).
endef

$(eval $(call KernelPackage,fsl-dpaa2-console))

define KernelPackage/marvell-mdio
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell Armada platform MDIO driver
  DEPENDS:=@(TARGET_armvirt_64) +kmod-libphy +kmod-of-mdio +kmod-acpi-mdio
  KCONFIG:=CONFIG_MVMDIO
  FILES=$(LINUX_DIR)/drivers/net/ethernet/marvell/mvmdio.ko
  AUTOLOAD=$(call AutoLoad,30,marvell-mdio)
endef

$(eval $(call KernelPackage,marvell-mdio))

define KernelPackage/mvneta
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell Armada 370/38x/XP/37xx network driver
  DEPENDS:=@(TARGET_armvirt_64) +kmod-marvell-mdio +kmod-phylink
  KCONFIG:=CONFIG_MVNETA
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/marvell/mvneta.ko
  AUTOLOAD=$(call AutoLoad,40,mvneta)
endef

$(eval $(call KernelPackage,mvneta))

define KernelPackage/mvpp2
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell Armada 375/7K/8K network driver
  DEPENDS:=@(TARGET_armvirt_64) +kmod-marvell-mdio +kmod-phylink
  KCONFIG:=CONFIG_MVPP2 \
    CONFIG_MVPP2_PTP=n
  FILES=$(LINUX_DIR)/drivers/net/ethernet/marvell/mvpp2/mvpp2.ko
  AUTOLOAD=$(call AutoLoad,40,mvpp2)
endef

$(eval $(call KernelPackage,mvpp2))

define KernelPackage/imx2-wdt
  SUBMENU:=$(OTHER_MENU)
  TITLE:=NXP (Freescale) i.MX2+ and Layerscape watchdog driver
  KCONFIG:=CONFIG_IMX2_WDT
  FILES=$(LINUX_DIR)/drivers/watchdog/imx2_wdt.ko
  AUTOLOAD=$(call AutoLoad,60,imx2_wdt)
endef

$(eval $(call KernelPackage,imx2-wdt))

define KernelPackage/imx7-ulp-wdt
  SUBMENU:=$(OTHER_MENU)
  TITLE:=NXP (Freescale) i.MX7ULP and later watchdog
  KCONFIG:=CONFIG_IMX7ULP_WDT
  FILES=$(LINUX_DIR)/drivers/watchdog/imx7ulp_wdt.ko
  AUTOLOAD=$(call AutoLoad,60,imx7ulp_wdt)
endef

$(eval $(call KernelPackage,imx7-ulp-wdt))

define KernelPackage/stmmac-core
  SUBMENU=$(NETWORK_DEVICES_MENU)
  TITLE:=Synopsis Ethernet Controller core (NXP,STMMicro,others)
  DEPENDS:=@(TARGET_armvirt_64) +kmod-phylink +kmod-pcs-xpcs +kmod-ptp
  KCONFIG:=CONFIG_STMMAC_ETH \
    CONFIG_STMMAC_SELFTESTS=n \
    CONFIG_STMMAC_PLATFORM \
    CONFIG_CONFIG_DWMAC_DWC_QOS_ETH=n \
    CONFIG_DWMAC_GENERIC
  FILES=$(LINUX_DIR)/drivers/net/ethernet/stmicro/stmmac/stmmac.ko \
    $(LINUX_DIR)/drivers/net/ethernet/stmicro/stmmac/stmmac-platform.ko \
    $(LINUX_DIR)/drivers/net/ethernet/stmicro/stmmac/dwmac-generic.ko
  AUTOLOAD=$(call AutoLoad,40,stmmac stmmac-platform dwmac-generic)
endef

$(eval $(call KernelPackage,stmmac-core))

define KernelPackage/dwmac-imx
  SUBMENU=$(NETWORK_DEVICES_MENU)
  TITLE:=NXP i.MX8 Ethernet controller
  DEPENDS:=+kmod-stmmac-core
  KCONFIG:=CONFIG_DWMAC_IMX8
  FILES=$(LINUX_DIR)/drivers/net/ethernet/stmicro/stmmac/dwmac-imx.ko
  AUTOLOAD=$(call AutoLoad,45,dwmac-imx)
endef

$(eval $(call KernelPackage,dwmac-imx))

define KernelPackage/thunderx-net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell (Cavium) ThunderX/2 network drivers
  DEPENDS:=@(TARGET_armvirt_64) +kmod-phylink
  KCONFIG:=CONFIG_NET_VENDOR_CAVIUM \
    CONFIG_THUNDER_NIC_PF \
    CONFIG_THUNDER_NIC_VF \
    CONFIG_THUNDER_NIC_BGX \
    CONFIG_THUNDER_NIC_RGX
  FILES=$(LINUX_DIR)/drivers/net/ethernet/cavium/thunder/nicvf.ko \
    $(LINUX_DIR)/drivers/net/ethernet/cavium/thunder/nicpf.ko \
    $(LINUX_DIR)/drivers/net/ethernet/cavium/thunder/thunder_xcv.ko \
    $(LINUX_DIR)/drivers/net/ethernet/cavium/thunder/thunder_bgx.ko
  AUTOLOAD=$(call AutoLoad,40,nicpf nicvf thunder_xcv thunder_bgx)
endef

$(eval $(call KernelPackage,thunderx-net))
