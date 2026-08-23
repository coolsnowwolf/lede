/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°Airoha Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to Airoha Limited (¡°Airoha¡±) and/or 
its licensors. Without the prior written permission of Airoha and/or its licensors, 
any reproduction, modification, use or disclosure of Airoha Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

Airoha Limited. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°AIROHA SOFTWARE¡±) RECEIVED FROM AIROHA 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES AIROHA PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE AIROHA SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE.

AIROHA SHALL NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
AIROHA'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AIROHA 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT AIROHA'S SOLE OPTION, TO 
REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO AIROHA FOR SUCH 
AIROHA SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include <asm/io.h>
#include "./ecnt_serdes_common_phy.h"
/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
#if 0
struct str_serdes_common_phy {
	struct device *dev;
	void __iomem *G3_ana_phy_rg_base; /* PCIEG3_PHY_PMA_PHYA physical address */
	void __iomem *G3_pma_phy_rg_base; /* PCIEG3_PHY_PMA_PHYD_0 physical address */
	void __iomem *G3_pma1_phy_rg_base; /* PCIEG3_PHY_PMA_PHYD_1 physical address */
	void __iomem *xfi_ana_pxp_phy_rg_base; /* xfi_ana_pxp physical address */
	void __iomem *xfi_pma_phy_rg_base; /* xfi_pma physical address */
	void __iomem *pon_ana_pxp_phy_rg_base; /* pon_ana_pxp physical address */
	void __iomem *pon_pma_phy_rg_base; /* pon_pma physical address */
};
#endif
/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
struct str_serdes_common_phy *serdes_common_phy = NULL;

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/
static const struct of_device_id serdes_common_phy_of_ids[] = {
	{ .compatible = "airoha,serdes_common_phy"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, serdes_common_phy_of_ids);

static struct platform_device *virt_dev = NULL;


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/


/********************************************************************/

/* APIs */
/***********************************************/


struct str_serdes_common_phy* Get_Struct(void)
{
	return serdes_common_phy;
}
EXPORT_SYMBOL(Get_Struct);

void __iomem* Get_Base(u32 base)
{
	printk("\ncommon_phy Get_Base:0x%x\n",base);
	switch (base)
	{
		case 0x1fc7f000:	
			return serdes_common_phy->G3_ana_phy_rg_base;		
			break;
			
		case 0x1fc7e000:			
			return serdes_common_phy->G3_pma_phy_rg_base;			
			break;
						
		case 0x1fa7f000:
			return serdes_common_phy->xfi_ana_pxp_phy_rg_base; 			
			break;
			
		case 0x1fa7e000:
			return serdes_common_phy->xfi_pma_phy_rg_base; 			
			break;

		case 0x1fa8f000:
			return serdes_common_phy->pon_ana_pxp_phy_rg_base; 			
			break;
			
		case 0x1fa8e000:
			return serdes_common_phy->pon_pma_phy_rg_base; 			
			break;

		case 0x1fa84000:
			return serdes_common_phy->multi_sgmii_base; 			
			break;

		case 0x1fa5f000:
			return serdes_common_phy->qp_pcie_ana_base; 			
			break;
			
		case 0x1fa5a000:
			return serdes_common_phy->qp_pcie_dig_base; 			
			break;

		case 0x1fa5e000:
			return serdes_common_phy->qp_pcie_pma_base; 			
			break;

		case 0x1fa6f000:
			return serdes_common_phy->qp_usb_ana_base; 			
			break;
			
		case 0x1fa6a000:
			return serdes_common_phy->qp_usb_dig_base; 			
			break;

		case 0x1fa6e000:
			return serdes_common_phy->qp_usb_pma_base; 			
			break;

			
		default :
			printk("base addr not 10G Serdes or QPHY Common addr!ex: 0x1fc7f000, 0x1fa7f000, 0x1fa8f000\n");
			return NULL;
			break;
	};
}
EXPORT_SYMBOL(Get_Base);

int get_pon_phy_irq(void)
{
	return serdes_common_phy->irq;
	
}
EXPORT_SYMBOL(get_pon_phy_irq);

/*get phy efuse*/
u32 get_phy_efuse(u32 start_bit, u32 len)
{
	//struct platform_device *virt_dev = NULL;
	u64 status;
	//u32 *phyAddr=NULL;
	unsigned long phyAddr=0;
	//u8 efuse_sample[256]={0};
	unsigned char *buffer_EF = NULL;
	size_t size = 256;
	u32 D_byteIdx, D_bitIdx;
	u32 EF_byteIdx, EF_bitIdx;
	u32 addr=0, ii =0;
	u32 data=0;
	u8 *P_data = (u8 *)&data;

	//phyAddr = virt_to_phys(efuse_sample);

	if(len > 0 && len <= 32 && (start_bit+len-1)<2048)
	{
		//printk("size = %d\n", size);
		size = PAGE_ALIGN(size);
		//printk("size = %d\n", size);
		buffer_EF = (unsigned char *) kzalloc(size, GFP_KERNEL | GFP_DMA);
		//virt_dev = platform_device_register_simple("phy_ef_load", -1, NULL, 0);
		phyAddr = (unsigned long) dma_map_single(&virt_dev->dev, buffer_EF, size, DMA_TO_DEVICE);
		//printk("phyAddr = %lld\n", phyAddr);
		
		status = readout_efuse(phyAddr);

		dma_unmap_single(&virt_dev->dev, (dma_addr_t) phyAddr, size, DMA_TO_DEVICE);
		//platform_device_unregister(virt_dev);
	}
	else
	{
		printk("[get_phy_efuse] input argument error: start=%d, len=%d\n", start_bit, len);
		return -1;
	}


	if(status == 0)
	{
		for(ii = 0; ii < len; ii++) {
			addr = start_bit + ii;
		
			D_byteIdx = ii >> 3; /* ii / 8 */
			D_bitIdx = ii & 0x7; /* ii % 8 */
			EF_byteIdx = addr >> 3;	/* addr / 8 */
			EF_bitIdx = addr & 0x7;	/* addr % 8 */
		
			P_data[D_byteIdx] |= (((buffer_EF[EF_byteIdx] >> EF_bitIdx) & 0x1) << D_bitIdx);
		}
	}
	else
	{
		printk("[get_phy_efuse] readout_efuse api fail: status=%d\n", status);
		return -1;
	}

	return data;
}
EXPORT_SYMBOL(get_phy_efuse);

int serdes_common_phy_probe(void)
{
	struct resource *res = NULL;
	struct device_node *node=NULL;
	struct platform_device *pdev=NULL;


	printk("\nserdes_common_phy_probe\n");
	virt_dev = platform_device_register_simple("phy_efuse_load", -1, NULL, 0);
	
	node = of_find_node_by_path("/serdes_common_phy@1fa7e000");
	if (node==NULL) {
	    printk("\nERROR(%s) node==NULL\n", __func__);
	    return -1;
	}

	pdev = of_find_device_by_node(node);
	if (pdev==NULL) {
	    printk("\nERROR(%s) pdev==NULL\n", __func__);
	    return -1;
	}

	serdes_common_phy = devm_kzalloc(&pdev->dev, sizeof(struct str_serdes_common_phy), GFP_KERNEL);
	if (!serdes_common_phy)
		return -ENOMEM;
	platform_set_drvdata(pdev, serdes_common_phy);

	/* PCIEG3_PHY_PMA_PHYA physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	serdes_common_phy->G3_ana_phy_rg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->G3_ana_phy_rg_base)) {
	    printk("\nERROR(%s) G3_ana_phy_rg_base\n", __func__);
		return -1;
	}
    
	/* PCIEG3_PHY_PMA_PHYD physical address  */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	serdes_common_phy->G3_pma_phy_rg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->G3_pma_phy_rg_base)) {
	    printk("\nERROR(%s) G3_pma_phy_rg_base\n", __func__);
		return -1;
	}



	/* xfi_ana_pxp physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	serdes_common_phy->xfi_ana_pxp_phy_rg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->xfi_ana_pxp_phy_rg_base)) {
	    printk("\nERROR(%s) xfi_ana_pxp_phy_rg_base\n", __func__);
		return -1;
	}

	/* xfi_pma physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	serdes_common_phy->xfi_pma_phy_rg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->xfi_pma_phy_rg_base)) {
	    printk("\nERROR(%s) xfi_pma_phy_rg_base\n", __func__);
		return -1;
	}

	/* pon_ana_pxp physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 4);
	serdes_common_phy->pon_ana_pxp_phy_rg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->pon_ana_pxp_phy_rg_base)) {
	    printk("\nERROR(%s) pon_ana_pxp_phy_rg_base\n", __func__);
		return -1;
	}

	/* pon_pma physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 5);
	serdes_common_phy->pon_pma_phy_rg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->pon_pma_phy_rg_base)) {
	    printk("\nERROR(%s) pon_pma_phy_rg_base\n", __func__);
		return -1;
	}

	/* multi_sgmii physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 6);
	serdes_common_phy->multi_sgmii_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->multi_sgmii_base)) {
	    printk("\nERROR(%s) multi_sgmii_base\n", __func__);
		return -1;
	}


	/* QPHY PCIe ana physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 7);
	serdes_common_phy->qp_pcie_ana_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->qp_pcie_ana_base)) {
	    printk("\nERROR(%s) qp_pcie_ana_base\n", __func__);
		return -1;
	}

	/* QPHY PCIe dig physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 8);
	serdes_common_phy->qp_pcie_dig_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->qp_pcie_dig_base)) {
	    printk("\nERROR(%s) qp_pcie_dig_base\n", __func__);
		return -1;
	}

	/* QPHY PCIe pma physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 9);
	serdes_common_phy->qp_pcie_pma_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->qp_pcie_pma_base)) {
	    printk("\nERROR(%s) qp_pcie_pma_base\n", __func__);
		return -1;
	}

	/* QPHY USB ana physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 10);
	serdes_common_phy->qp_usb_ana_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->qp_usb_ana_base)) {
	    printk("\nERROR(%s) qp_usb_ana_base\n", __func__);
		return -1;
	}

	/* QPHY USB dig physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 11);
	serdes_common_phy->qp_usb_dig_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->qp_usb_dig_base)) {
	    printk("\nERROR(%s) qp_usb_dig_base\n", __func__);
		return -1;
	}

	/* QPHY USB pma physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 12);
	serdes_common_phy->qp_usb_pma_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(serdes_common_phy->qp_usb_pma_base)) {
	    printk("\nERROR(%s) qp_usb_pma_base\n", __func__);
		return -1;
	}


	serdes_common_phy->dev = &pdev->dev;

	serdes_common_phy->irq = platform_get_irq(pdev, 0);
	if (serdes_common_phy->irq <= 0)
	{
		//devm_kfree(&pdev->dev, ecnt_pon_phy);
		return serdes_common_phy->irq;
	}
	else printk("common platform_get_irq %d\n",serdes_common_phy->irq);

	printk("serdes_common_phy->G3_ana_phy_rg_base= %lx\n", (unsigned long)serdes_common_phy->G3_ana_phy_rg_base);
	printk("serdes_common_phy->G3_pma_phy_rg_base= %lx\n", (unsigned long)serdes_common_phy->G3_pma_phy_rg_base);
	
	printk("serdes_common_phy->xfi_ana_pxp_phy_rg_base= %lx\n", (unsigned long)serdes_common_phy->xfi_ana_pxp_phy_rg_base);
	printk("serdes_common_phy->xfi_pma_phy_rg_base= %lx\n", (unsigned long)serdes_common_phy->xfi_pma_phy_rg_base);
	
	printk("serdes_common_phy->pon_ana_pxp_phy_rg_base= %lx\n", (unsigned long)serdes_common_phy->pon_ana_pxp_phy_rg_base);
	printk("serdes_common_phy->pon_pma_phy_rg_base= %lx\n", (unsigned long)serdes_common_phy->pon_pma_phy_rg_base);

	printk("serdes_common_phy->multi_sgmii_base= %lx\n", (unsigned long)serdes_common_phy->multi_sgmii_base);

	printk("serdes_common_phy->qp_pcie_ana_base= %lx\n", (unsigned long)serdes_common_phy->qp_pcie_ana_base);
	printk("serdes_common_phy->qp_pcie_dig_base= %lx\n", (unsigned long)serdes_common_phy->qp_pcie_dig_base);
	printk("serdes_common_phy->qp_pcie_pma_base= %lx\n", (unsigned long)serdes_common_phy->qp_pcie_pma_base);

	printk("serdes_common_phy->qp_usb_ana_base= %lx\n", (unsigned long)serdes_common_phy->qp_usb_ana_base);
	printk("serdes_common_phy->qp_usb_dig_base= %lx\n", (unsigned long)serdes_common_phy->qp_usb_dig_base);
	printk("serdes_common_phy->qp_usb_pma_base= %lx\n", (unsigned long)serdes_common_phy->qp_usb_pma_base);
	
	return 0;
}

/* init SCU registers' base address (a.s.a.p.) before any kernel module might access it. 
 * For example, usb_init() calls "isFPGA" which will access NP SCU register. 
 * If SCU base address has not initialized before that, cpu will crash. 
 * usb_init() uses subsys_initcall to init. Although ECNT_SCU_DRV_PROBE also uses
 * the smae subsys_initcall, it's executed before usb_init(), so it's ok 
 * Note: you can check linux-4.4.115/System.map to see which initcall function will be executed first*/
subsys_initcall(serdes_common_phy_probe);

