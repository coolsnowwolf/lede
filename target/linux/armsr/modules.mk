define KernelPackage/acpi-mdio
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=ACPI MDIO support
  DEPENDS:=@TARGET_armsr +kmod-libphy +kmod-mdio-devres
  KCONFIG:=CONFIG_ACPI_MDIO
  FILES:=$(LINUX_DIR)/drivers/net/mdio/acpi_mdio.ko
  AUTOLOAD:=$(call AutoLoad,11,acpi_mdio)
endef

define KernelPackage/acpi-mdio/description
 Kernel driver for ACPI MDIO support
endef

$(eval $(call KernelPackage,acpi-mdio))

define KernelPackage/bcmgenet
  SUBMENU=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@TARGET_armsr +kmod-mdio-bcm-unimac
  TITLE:=Broadcom GENET internal MAC (Raspberry Pi 4)
  KCONFIG:=CONFIG_BCMGENET
  FILES=$(LINUX_DIR)/drivers/net/ethernet/broadcom/genet/genet.ko
  AUTOLOAD=$(call AutoLoad,30,genet)
endef

$(eval $(call KernelPackage,bcmgenet))

define KernelPackage/mdio-bcm-unimac
  SUBMENU=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@TARGET_armsr +kmod-of-mdio
  TITLE:=Broadcom UniMAC MDIO bus controller
  KCONFIG:=CONFIG_MDIO_BCM_UNIMAC
  FILES=$(LINUX_DIR)/drivers/net/mdio/mdio-bcm-unimac.ko
  AUTOLOAD=$(call AutoLoad,30,mdio-bcm-unimac)
endef

$(eval $(call KernelPackage,mdio-bcm-unimac))

define KernelPackage/fsl-pcs-lynx
  SUBMENU=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@TARGET_armsr +kmod-libphy +kmod-of-mdio +kmod-phylink
  TITLE:=NXP (Freescale) Lynx PCS
  HIDDEN:=1
  KCONFIG:=CONFIG_PCS_LYNX
  FILES=$(LINUX_DIR)/drivers/net/pcs/pcs-lynx.ko
  AUTOLOAD=$(call AutoLoad,30,pcs-lynx)
endef

$(eval $(call KernelPackage,fsl-pcs-lynx))

define KernelPackage/fsl-fec
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@TARGET_armsr +kmod-libphy +kmod-of-mdio \
    +kmod-ptp +kmod-net-selftests
  TITLE:=NXP (Freescale) FEC Ethernet controller (i.MX)
  KCONFIG:=CONFIG_FEC
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/freescale/fec.ko
  AUTOLOAD:=$(call AutoLoad,35,fec)
endef

$(eval $(call KernelPackage,fsl-fec))

define KernelPackage/fsl-xgmac-mdio
  SUBMENU=$(NETWORK_DEVICES_MENU)
  DEPENDS:=@TARGET_armsr +kmod-libphy +kmod-of-mdio +kmod-acpi-mdio
  TITLE:=NXP (Freescale) MDIO bus
  KCONFIG:=CONFIG_FSL_XGMAC_MDIO
  FILES=$(LINUX_DIR)/drivers/net/ethernet/freescale/xgmac_mdio.ko
  AUTOLOAD=$(call AutoLoad,30,xgmac_mdio)
endef

$(eval $(call KernelPackage,fsl-xgmac-mdio))

define KernelPackage/fsl-mc-dpio
  SUBMENU:=$(OTHER_MENU)
  TITLE:=NXP DPAA2 DPIO (Data Path IO) driver
  HIDDEN:=1
  KCONFIG:=CONFIG_FSL_MC_BUS=y \
    CONFIG_FSL_MC_DPIO
  FILES:=$(LINUX_DIR)/drivers/soc/fsl/dpio/fsl-mc-dpio.ko
  AUTOLOAD=$(call AutoLoad,30,fsl-mc-dpio)
endef

$(eval $(call KernelPackage,fsl-mc-dpio))

define KernelPackage/fsl-enetc-net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=:NXP ENETC (LS1028A) Ethernet
  DEPENDS:=@TARGET_armsr +kmod-phylink +kmod-fsl-pcs-lynx
  KCONFIG:= \
    CONFIG_FSL_ENETC \
    CONFIG_FSL_ENETC_VF \
    CONFIG_FSL_ENETC_QOS=y
  FILES:= \
    $(LINUX_DIR)/drivers/net/ethernet/freescale/enetc/fsl-enetc.ko \
    $(LINUX_DIR)/drivers/net/ethernet/freescale/enetc/fsl-enetc-core.ko \
    $(LINUX_DIR)/drivers/net/ethernet/freescale/enetc/fsl-enetc-vf.ko \
    $(LINUX_DIR)/drivers/net/ethernet/freescale/enetc/fsl-enetc-mdio.ko \
    $(LINUX_DIR)/drivers/net/ethernet/freescale/enetc/fsl-enetc-ierb.ko
  AUTOLOAD=$(call AutoLoad,35,fsl-enetc)
endef

$(eval $(call KernelPackage,fsl-enetc-net))

define KernelPackage/fsl-dpaa1-net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=NXP DPAA1 (LS1043/LS1046) Ethernet
  DEPENDS:=@TARGET_armsr +kmod-fsl-xgmac-mdio +kmod-libphy +kmod-crypto-crc32
  KCONFIG:= \
    CONFIG_FSL_DPAA=y \
    CONFIG_FSL_DPAA_ETH \
    CONFIG_FSL_FMAN \
    CONFIG_FSL_DPAA_CHECKING=n \
    CONFIG_FSL_BMAN_TEST=n \
    CONFIG_FSL_QMAN_TEST=n
  MODULES:= \
    $(LINUX_DIR)/drivers/net/ethernet/freescale/dpaa/fsl_dpa.ko \
    $(LINUX_DIR)/drivers/net/ethernet/freescale/fman/fsl_dpaa_fman.ko \
    $(LINUX_DIR)/drivers/net/ethernet/freescale/fman/fsl_dpaa_mac.ko
  AUTOLOAD=$(call AutoLoad,35,fsl-dpa)
endef

$(eval $(call KernelPackage,fsl-dpaa1-net))

define KernelPackage/fsl-dpaa2-net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=NXP DPAA2 Ethernet
  DEPENDS:=@TARGET_armsr +kmod-fsl-xgmac-mdio +kmod-phylink \
    +kmod-fsl-pcs-lynx +kmod-fsl-mc-dpio
  KCONFIG:= \
    CONFIG_FSL_MC_UAPI_SUPPORT=y \
    CONFIG_FSL_DPAA2_ETH \
    CONFIG_FSL_DPAA2_ETH_DCB=y
  FILES:= \
  $(LINUX_DIR)/drivers/net/ethernet/freescale/dpaa2/fsl-dpaa2-eth.ko
  AUTOLOAD=$(call AutoLoad,35,fsl-dpaa2-eth)
endef

$(eval $(call KernelPackage,fsl-dpaa2-net))

define KernelPackage/fsl-dpaa2-console
  SUBMENU:=$(OTHER_MENU)
  TITLE:=NXP DPAA2 Debug console
  DEPENDS:=@TARGET_armsr
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
  DEPENDS:=@TARGET_armsr +kmod-libphy +kmod-of-mdio +kmod-acpi-mdio
  KCONFIG:=CONFIG_MVMDIO
  FILES=$(LINUX_DIR)/drivers/net/ethernet/marvell/mvmdio.ko
  AUTOLOAD=$(call AutoLoad,30,marvell-mdio)
endef

$(eval $(call KernelPackage,marvell-mdio))

define KernelPackage/mvneta
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell Armada 370/38x/XP/37xx network driver
  DEPENDS:=@TARGET_armsr +kmod-marvell-mdio +kmod-phylink
  KCONFIG:=CONFIG_MVNETA
  FILES:=$(LINUX_DIR)/drivers/net/ethernet/marvell/mvneta.ko
  AUTOLOAD=$(call AutoLoad,30,mvneta)
endef

$(eval $(call KernelPackage,mvneta))

define KernelPackage/mvpp2
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell Armada 375/7K/8K network driver
  DEPENDS:=@TARGET_armsr +kmod-marvell-mdio +kmod-phylink
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

define KernelPackage/dwmac-imx
  SUBMENU=$(NETWORK_DEVICES_MENU)
  TITLE:=NXP i.MX8 Ethernet controller
  DEPENDS:=+kmod-stmmac-core +kmod-of-mdio
  KCONFIG:=CONFIG_DWMAC_IMX8
  FILES=$(LINUX_DIR)/drivers/net/ethernet/stmicro/stmmac/dwmac-imx.ko
  AUTOLOAD=$(call AutoLoad,45,dwmac-imx)
endef

$(eval $(call KernelPackage,dwmac-imx))

define KernelPackage/dwmac-sun8i
  SUBMENU=$(NETWORK_DEVICES_MENU)
  TITLE:=Allwinner H3/A83T/A64 (sun8i) Ethernet
  DEPENDS:=+kmod-stmmac-core +kmod-of-mdio +kmod-mdio-bus-mux
  KCONFIG:=CONFIG_DWMAC_SUN8I
  FILES=$(LINUX_DIR)/drivers/net/ethernet/stmicro/stmmac/dwmac-sun8i.ko
  AUTOLOAD=$(call AutoLoad,45,dwmac-sun8i)
endef

$(eval $(call KernelPackage,dwmac-sun8i))

define KernelPackage/dwmac-rockchip
  SUBMENU=$(NETWORK_DEVICES_MENU)
  TITLE:=Rockchip RK3328/RK3399/RK3568 Ethernet
  DEPENDS:=+kmod-stmmac-core +kmod-of-mdio +kmod-mdio-bus-mux
  KCONFIG:=CONFIG_DWMAC_ROCKCHIP
  FILES=$(LINUX_DIR)/drivers/net/ethernet/stmicro/stmmac/dwmac-rk.ko
  AUTOLOAD=$(call AutoLoad,45,dwmac-rk)
endef

$(eval $(call KernelPackage,dwmac-rockchip))

define KernelPackage/mdio-thunder
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell (Cavium) Thunder MDIO controller
  DEPENDS:=@TARGET_armsr +kmod-of-mdio
  KCONFIG:=CONFIG_MDIO_THUNDER
  FILES=$(LINUX_DIR)/drivers/net/mdio/mdio-cavium.ko \
    $(LINUX_DIR)/drivers/net/mdio/mdio-thunder.ko
  AUTOLOAD=$(call AutoLoad,30,mdio-cavium mdio-thunder)
endef

$(eval $(call KernelPackage,mdio-thunder))

define KernelPackage/thunderx-net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell (Cavium) Thunder network drivers
  DEPENDS:=@TARGET_armsr_armv8 +kmod-phylink +kmod-mdio-thunder
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

define KernelPackage/octeontx2-net
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Marvell (Cavium) ThunderX2 network drivers
  DEPENDS:=@TARGET_armsr +kmod-phylink +kmod-of-mdio +kmod-macsec \
    +kmod-ptp
  KCONFIG:=CONFIG_OCTEONTX2_MBOX \
    CONFIG_OCTEONTX2_AF \
    CONFIG_OCTEONTX2_PF \
    CONFIG_OCTEONTX2_VF \
    CONFIG_NDC_DIS_DYNAMIC_CACHING=n
  FILES=$(LINUX_DIR)/drivers/net/ethernet/marvell/octeontx2/af/rvu_mbox.ko \
    $(LINUX_DIR)/drivers/net/ethernet/marvell/octeontx2/af/rvu_af.ko \
    $(LINUX_DIR)/drivers/net/ethernet/marvell/octeontx2/nic/rvu_nicpf.ko \
    $(LINUX_DIR)/drivers/net/ethernet/marvell/octeontx2/nic/rvu_nicvf.ko \
    $(LINUX_DIR)/drivers/net/ethernet/marvell/octeontx2/nic/otx2_ptp.ko
  AUTOLOAD=$(call AutoLoad,40,rvu_af rvu_mbox rvu_nicpf rvu_nicvf otx2_ptp)
endef
$(eval $(call KernelPackage,octeontx2-net))

define KernelPackage/renesas-net-avb
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Renesas network drivers
  DEPENDS:=@TARGET_armsr +kmod-phylink +kmod-mii +kmod-ptp +kmod-libphy +kmod-mdio-gpio
  KCONFIG:=CONFIG_RAVB
  FILES=$(LINUX_DIR)/drivers/net/ethernet/renesas/ravb.ko
  AUTOLOAD:=$(call AutoProbe,ravb)
endef
 
define KernelPackage/renesas-net-avb/description
  Support Renesas RZ platform Ethernet module
endef
 
$(eval $(call KernelPackage,renesas-net-avb))

define KernelPackage/wdt-sp805
  SUBMENU:=$(OTHER_MENU)
  TITLE:=ARM SP805 Watchdog
  KCONFIG:=CONFIG_ARM_SP805_WATCHDOG
  FILES=$(LINUX_DIR)/drivers/watchdog/sp805_wdt.ko
  AUTOLOAD=$(call AutoLoad,50,sp805_wdt)
endef

define KernelPackage/wdt-sp805/description
  Support for the ARM SP805 wathchdog module.
  This is present in the NXP Layerscape family,
  HiSilicon HI3660 among others.
endef

$(eval $(call KernelPackage,wdt-sp805))

