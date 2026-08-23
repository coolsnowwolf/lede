#ifndef _COMMON_PHY
#define _COMMON_PHY
struct str_serdes_common_phy {
	struct device *dev;
	void __iomem *G3_ana_phy_rg_base; /* serdes 3: PCIEG3_PHY_PMA_PHYA physical address */
	void __iomem *G3_pma_phy_rg_base; /* serdes 3: PCIEG3_PHY_PMA_PHYD_0 physical address */
	void __iomem *xfi_ana_pxp_phy_rg_base; /* serdes 2: xfi_ana_pxp physical address */
	void __iomem *xfi_pma_phy_rg_base; /* serdes 2: xfi_pma physical address */
	void __iomem *pon_ana_pxp_phy_rg_base; /* serdes 1: pon_ana_pxp physical address */
	void __iomem *pon_pma_phy_rg_base; /* serdes 1 :pon_pma physical address */
	void __iomem *multi_sgmii_base; /* multi_sgmii, PON_PHY_ASIC_RG range4,only for xilinx_fpga, PON_PHY_ASIC_RG range5,only for xilinx_fpga */
	
	void __iomem *qp_pcie_ana_base; /* serdes 4: QPHY PCIe ana*/
	void __iomem *qp_pcie_dig_base; /* serdes 4: QPHY PCIe dig*/
	void __iomem *qp_pcie_pma_base; /* serdes 4: QPHY PCIe pma*/
	
	void __iomem *qp_usb_ana_base; /* serdes 5: QPHY USB ana*/
	void __iomem *qp_usb_dig_base; /* serdes 5: QPHY USB dig*/
	void __iomem *qp_usb_pma_base; /* serdes 5: QPHY USB pma*/
	int irq;
};
#endif
