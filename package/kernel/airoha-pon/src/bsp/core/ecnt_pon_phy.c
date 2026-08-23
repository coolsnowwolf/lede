/***************************************************************
Copyright Statement:

This software/firmware and related documentation (��EcoNet Software��) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (��EcoNet��) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (��ECONET SOFTWARE��) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ��AS IS�� 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER��S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER��S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>

#include <ecnt_hook/ecnt_hook_pon_phy.h>


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

#define PON_PHY_RG_BASE_1        	(0x1faf0000)
#define PON_PHY_RG_RANGE_1			(0x1fff)
#define PON_PHY_RG_END_1			(PON_PHY_RG_BASE_1 + PON_PHY_RG_RANGE_1)

#define PON_PHY_RG_BASE_2        	(0x1faf3000)
#define PON_PHY_RG_RANGE_2			(0xfff)
#define PON_PHY_RG_END_2			(PON_PHY_RG_BASE_2 + PON_PHY_RG_RANGE_2)

#define PON_PHY_RG_BASE_3        	(0x1faf4000)
#define PON_PHY_RG_RANGE_3			(0xfff)
#define PON_PHY_RG_END_3			(PON_PHY_RG_BASE_3 + PON_PHY_RG_RANGE_3)

#define PON_PHY_RG_SCU_RST			(0x1fb00830)
#define PON_PHY_FPGA_RG_TX_OFF		(0x1fa2ff24)
#ifdef TCSUPPORT_CPU_AN7552
#define PON_PHY_FPGA_RG_XILINX_RST  (0x1fa84024) // only for xilinx_serdes rst, ang_20211117
#define PON_PHY_FPGA_RG_XILINX_CTL  (0x1fa84030) // only for xilinx_serdes ctl, ang_20211117
#define PON_PHY_FPGA_RG_XILINX      (0x1fa84000) // only for xilinx_serdes rst, ang_20211117
#define PON_PHY_PON_ANA_BASE        (0x1faf3000)  // only for asic
#define PON_PHY_PON_PMA_BASE        (0x1faf4000)  // only for asic

#endif
#ifdef TCSUPPORT_CPU_EN7581
#define PON_PHY_FPGA_RG_XILINX_RST  (0x1fa84024) // only for xilinx_serdes rst, ang_20211117
#define PON_PHY_FPGA_RG_XILINX_CTL  (0x1fa84030) // only for xilinx_serdes ctl, ang_20211117
#define PON_PHY_FPGA_RG_XILINX  (0x1fa84000) // only for xilinx_serdes rst, ang_20211117

#ifdef TCSUPPORT_CPU_AN7583
#define PON_PHY_XFI_ANA_BASE  (0x1fa7F000)  // only for asic
#define PON_PHY_XFI_PMA_BASE  (0x1fa7E000)  // only for asic
#define PON_PHY_PON_ANA_BASE  (0x1fa8F000)  // only for asic
#define PON_PHY_PON_PMA_BASE  (0x1fa8E000)  // only for asic
#else
#define PON_PHY_XFI_ANA_BASE  (0x1fa7A000)  // only for asic
#define PON_PHY_XFI_PMA_BASE  (0x1fa7B000)  // only for asic
#define PON_PHY_PON_ANA_BASE  (0x1fa8A000)  // only for asic
#define PON_PHY_PON_PMA_BASE  (0x1fa8B000)  // only for asic

#endif
#endif
#define XPONPHY_ALL_INT_MAX_NUM		(1)

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

struct ecnt_pon_phy {
	struct device *dev;
	void __iomem *pon_phy_rg_base_1; /* PON_PHY_ASIC_RG physical address */
	//void __iomem *pon_phy_rg_scu_rst; /* PON_PHY_RG TOP_SCU_RST physical address */
	void __iomem *pon_phy_fpga_rg_tx_off; /* PON_PHY_FPGA_RG TX_OFF physical address */
	void __iomem *pon_phy_rg_base_2; /* PON_PHY_ASIC_RG physical address */	
	void __iomem *pon_phy_rg_base_3; /* PON_PHY_ASIC_RG physical address */
#ifdef TCSUPPORT_CPU_EN7581
	void __iomem *pon_phy_fpga_rg_xilinx_rst; /* PON_PHY_FPGA_RG_XILINX_RST physical address */
	void __iomem *pon_phy_fpga_rg_xilinx_ctl; /* PON_PHY_FPGA_RG_XILINX_CTL physical address */
	void __iomem *pon_phy_xfi_ana; 
	void __iomem *pon_phy_xfi_pma; 
	void __iomem *pon_phy_pon_ana; 
	void __iomem *pon_phy_pon_pma; 	
#endif
#ifdef TCSUPPORT_CPU_AN7552
	void __iomem *pon_phy_fpga_rg_xilinx_rst; /* PON_PHY_FPGA_RG_XILINX_RST physical address */
	void __iomem *pon_phy_fpga_rg_xilinx_ctl; /* PON_PHY_FPGA_RG_XILINX_CTL physical address */
	void __iomem *pon_phy_pon_ana;
	void __iomem *pon_phy_pon_pma; 	
#endif
	//int xponphy_irq[XPONPHY_ALL_INT_MAX_NUM];
	int phy_irq;
};
static spinlock_t	 data_lock;

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
struct ecnt_pon_phy *ecnt_pon_phy = NULL;

static const struct of_device_id ecnt_pon_phy_of_ids[] = {
	{ .compatible = "econet,ecnt-pon_phy"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ecnt_pon_phy_of_ids);

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern void __iomem* Get_Base(u32 base);

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
u32 get_pon_phy_asic_data_1(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_rg_base_1 + reg);
}

void set_pon_phy_asic_data_1(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_rg_base_1 + reg); 
}


u32 get_pon_phy_fpga_rg_tx_off_data(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_fpga_rg_tx_off);
}

void set_pon_phy_fpga_rg_tx_off_data(u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_fpga_rg_tx_off); 
}

u32 get_pon_phy_asic_data_2(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_rg_base_2 + reg);
}

void set_pon_phy_asic_data_2(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_rg_base_2 + reg); 
}

u32 get_pon_phy_asic_data_3(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_rg_base_3 + reg);
}

void set_pon_phy_asic_data_3(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_rg_base_3 + reg); 
}

//only for xilinx_fpga

#ifdef TCSUPPORT_CPU_EN7581
u32 get_pon_phy_fpga_rg_xilinx_rst_data(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst);
}

void set_pon_phy_fpga_rg_xilinx_rst_data(u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst); 
}

u32 get_pon_phy_fpga_rg_xilinx_ctl_data(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl);
}

void set_pon_phy_fpga_rg_xilinx_ctl_data(u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl); 
}

u32 get_pon_phy_xfi_ana(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_xfi_ana + reg);
}

void set_pon_phy_xfi_ana(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_xfi_ana + reg); 
}	


u32 get_pon_phy_xfi_pma(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_xfi_pma + reg);
}

void set_pon_phy_xfi_pma(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_xfi_pma + reg); 
}	

u32 get_pon_phy_pon_ana(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_pon_ana + reg);
}

void set_pon_phy_pon_ana(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_pon_ana + reg); 
}	

u32 get_pon_phy_pon_pma(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_pon_pma + reg);
}

void set_pon_phy_pon_pma(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_pon_pma + reg); 
}	
#endif
#ifdef TCSUPPORT_CPU_AN7552
u32 get_pon_phy_fpga_rg_xilinx_rst_data(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst);
}

void set_pon_phy_fpga_rg_xilinx_rst_data(u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst); 
}

u32 get_pon_phy_fpga_rg_xilinx_ctl_data(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl);
}

void set_pon_phy_fpga_rg_xilinx_ctl_data(u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl); 
}
u32 get_pon_phy_pon_ana(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_pon_ana + reg);
}

void set_pon_phy_pon_ana(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_pon_ana + reg); 
}

u32 get_pon_phy_pon_pma(u32 reg)
{
	return readl(ecnt_pon_phy->pon_phy_pon_pma + reg);
}

void set_pon_phy_pon_pma(u32 reg, u32 val)
{
	writel(val, ecnt_pon_phy->pon_phy_pon_pma + reg); 
}
#endif

void (*ledTurnOff_hook)(uint8 led_no);
EXPORT_SYMBOL(ledTurnOff_hook);

/********************************************************************/



/* APIs */
/***********************************************/
u32 get_pon_phy_data(u32 reg)
{
	u32 reg_phy = 0;
	u32 reg_val = 0;
	u32 flags;

	spin_lock_irqsave(&data_lock, flags);

	/* translate addr to physical addr */
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;

	//reg_offset = reg_phy % 4;
	if(( reg_phy % 4) != 0){
		printk("\n(%s)Get reg_val error, reg=0x%08X\n", __func__, reg);		
		spin_unlock_irqrestore(&data_lock, flags);
		return 0;
	}

	if( (PON_PHY_RG_BASE_1 <= reg_phy) && (reg_phy < PON_PHY_RG_END_1) )
	{	
		reg_val= get_pon_phy_asic_data_1(reg_phy - PON_PHY_RG_BASE_1);
	}
	else if(PON_PHY_FPGA_RG_TX_OFF == reg_phy)
	{    	
		reg_val = get_pon_phy_fpga_rg_tx_off_data(reg_phy);
	}
	#ifndef TCSUPPORT_CPU_AN7552
	else if( (PON_PHY_RG_BASE_2 <= reg_phy) && (reg_phy < PON_PHY_RG_END_2) )
	{   
		reg_val= get_pon_phy_asic_data_2(reg_phy - PON_PHY_RG_BASE_2);
	}    
	else if( (PON_PHY_RG_BASE_3 <= reg_phy) && (reg_phy < PON_PHY_RG_END_3) )
	{   
		reg_val= get_pon_phy_asic_data_3(reg_phy - PON_PHY_RG_BASE_3);
	}
	#endif
#ifdef TCSUPPORT_CPU_EN7581
	else if((PON_PHY_FPGA_RG_XILINX_RST== reg_phy)&&(isFPGA))
	{    	
		reg_val = get_pon_phy_fpga_rg_xilinx_rst_data(reg_phy);
	}
	else if((PON_PHY_FPGA_RG_XILINX_CTL== reg_phy)&&(isFPGA))
	{    	
		reg_val = get_pon_phy_fpga_rg_xilinx_ctl_data(reg_phy);
	}
	else if( (PON_PHY_XFI_ANA_BASE <= reg_phy) && (reg_phy < PON_PHY_XFI_ANA_BASE+0xfff) )
    {    	
    	reg_val= get_pon_phy_xfi_ana(reg_phy - PON_PHY_XFI_ANA_BASE);
    } 
	else if( (PON_PHY_XFI_PMA_BASE <= reg_phy) && (reg_phy < PON_PHY_XFI_PMA_BASE+0xfff) )
    {    	
    	reg_val= get_pon_phy_xfi_pma(reg_phy - PON_PHY_XFI_PMA_BASE);
    } 
	else if( (PON_PHY_PON_ANA_BASE <= reg_phy) && (reg_phy < PON_PHY_PON_ANA_BASE+0xfff) )
    {    	
    	reg_val= get_pon_phy_pon_ana(reg_phy - PON_PHY_PON_ANA_BASE);
    } 
	else if( (PON_PHY_PON_PMA_BASE <= reg_phy) && (reg_phy < PON_PHY_PON_PMA_BASE+0xfff) )
    {    	
    	reg_val= get_pon_phy_pon_pma(reg_phy - PON_PHY_PON_PMA_BASE);
    } 
#endif
#ifdef TCSUPPORT_CPU_AN7552
	else if((PON_PHY_FPGA_RG_XILINX_RST== reg_phy)&&(isFPGA))
	{    	
		reg_val = get_pon_phy_fpga_rg_xilinx_rst_data(reg_phy);
	}
	else if((PON_PHY_FPGA_RG_XILINX_CTL== reg_phy)&&(isFPGA))
	{    	
		reg_val = get_pon_phy_fpga_rg_xilinx_ctl_data(reg_phy);
	}
	else if( (PON_PHY_PON_ANA_BASE <= reg_phy) && (reg_phy < PON_PHY_PON_ANA_BASE+0xfff) )
    {    	
    	reg_val= get_pon_phy_pon_ana(reg_phy - PON_PHY_PON_ANA_BASE);
    } 
	else if( (PON_PHY_PON_PMA_BASE <= reg_phy) && (reg_phy < PON_PHY_PON_PMA_BASE+0xfff) )
    {    	
    	reg_val= get_pon_phy_pon_pma(reg_phy - PON_PHY_PON_PMA_BASE);
    }
#endif
	else
	{
		printk("\n Datapath(%s) get reg error, reg=0x%08X\n", __func__, reg);
	}

	spin_unlock_irqrestore(&data_lock, flags);
	return reg_val;
}
EXPORT_SYMBOL(get_pon_phy_data);


void set_pon_phy_data(u32 reg, u32 val)
{
	u32 reg_phy = 0;
	u32 flags;

	spin_lock_irqsave(&data_lock, flags);

	/* translate addr to physical addr */
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;

	//reg_offset = reg_phy % 4;
	if(( reg_phy % 4) != 0){
		printk("\n(%s)Set reg_val error, reg=0x%08X\n", __func__, reg);		
		spin_unlock_irqrestore(&data_lock, flags);
		return ;
	}

	if( (PON_PHY_RG_BASE_1<= reg_phy) && (reg_phy < PON_PHY_RG_END_1) )
	{			
		set_pon_phy_asic_data_1(reg_phy - PON_PHY_RG_BASE_1, val); 
	}
	else if(PON_PHY_FPGA_RG_TX_OFF == reg_phy)
	{
		set_pon_phy_fpga_rg_tx_off_data(val);
	}
	#ifndef TCSUPPORT_CPU_AN7552
	else if( (PON_PHY_RG_BASE_2<= reg_phy) && (reg_phy < PON_PHY_RG_END_2) )
	{			
		set_pon_phy_asic_data_2(reg_phy - PON_PHY_RG_BASE_2, val); 
	}
	else if( (PON_PHY_RG_BASE_3<= reg_phy) && (reg_phy < PON_PHY_RG_END_3) )
	{			
		set_pon_phy_asic_data_3(reg_phy - PON_PHY_RG_BASE_3, val); 
	}
	#endif
#ifdef TCSUPPORT_CPU_EN7581
	else if((PON_PHY_FPGA_RG_XILINX_RST== reg_phy)&&(isFPGA))
	{    	
		set_pon_phy_fpga_rg_xilinx_rst_data(val);
	}
	else if((PON_PHY_FPGA_RG_XILINX_CTL== reg_phy)&&(isFPGA))
	{    	
		set_pon_phy_fpga_rg_xilinx_ctl_data(val);
	}
	else if( (PON_PHY_XFI_ANA_BASE <= reg_phy) && (reg_phy < PON_PHY_XFI_ANA_BASE+0xfff) )
    {    	
    	set_pon_phy_xfi_ana(reg_phy - PON_PHY_XFI_ANA_BASE, val);
    } 
	else if( (PON_PHY_XFI_PMA_BASE <= reg_phy) && (reg_phy < PON_PHY_XFI_PMA_BASE+0xfff) )
    {    	
    	set_pon_phy_xfi_pma(reg_phy - PON_PHY_XFI_PMA_BASE, val);
    } 
	else if( (PON_PHY_PON_ANA_BASE <= reg_phy) && (reg_phy < PON_PHY_PON_ANA_BASE+0xfff) )
    {    	
    	set_pon_phy_pon_ana(reg_phy - PON_PHY_PON_ANA_BASE, val);
    } 
	else if( (PON_PHY_PON_PMA_BASE <= reg_phy) && (reg_phy < PON_PHY_PON_PMA_BASE+0xfff) )
    {    	
    	set_pon_phy_pon_pma(reg_phy - PON_PHY_PON_PMA_BASE, val);
    } 

#endif
#ifdef TCSUPPORT_CPU_AN7552
	else if((PON_PHY_FPGA_RG_XILINX_RST== reg_phy)&&(isFPGA))
	{    	
		set_pon_phy_fpga_rg_xilinx_rst_data(val);
	}
	else if((PON_PHY_FPGA_RG_XILINX_CTL== reg_phy)&&(isFPGA))
	{    	
		set_pon_phy_fpga_rg_xilinx_ctl_data(val);
	}
	else if( (PON_PHY_PON_ANA_BASE <= reg_phy) && (reg_phy < PON_PHY_PON_ANA_BASE+0xfff) )
    {    	
    	set_pon_phy_pon_ana(reg_phy - PON_PHY_PON_ANA_BASE, val);
    } 
	else if( (PON_PHY_PON_PMA_BASE <= reg_phy) && (reg_phy < PON_PHY_PON_PMA_BASE+0xfff) )
    {    	
    	set_pon_phy_pon_pma(reg_phy - PON_PHY_PON_PMA_BASE, val);
    }
#endif
	else
	{
		printk("\nDatapath(%s) set reg error, reg=0x%08X\n", __func__, reg);
	}

	spin_unlock_irqrestore(&data_lock, flags);

}
EXPORT_SYMBOL(set_pon_phy_data);
/***********************************************/


struct device* get_pon_phy_dev(void)
{
	if( (ecnt_pon_phy) && (ecnt_pon_phy->dev) )
		return ecnt_pon_phy->dev;
	else
		return NULL;
}
EXPORT_SYMBOL(get_pon_phy_dev);

#ifndef TCSUPPORT_CPU_EN7581 //#if 0 //move to common
//#if defined(TCSUPPORT_CPU_AN7552) || defined(TCSUPPORT_CPU_EN7523) //modify for 7583
#ifndef TCSUPPORT_CPU_AN7583
int get_pon_phy_irq(void)
{
	return ecnt_pon_phy->phy_irq;
	
}
EXPORT_SYMBOL(get_pon_phy_irq);
#endif
#endif


void (*set_pon_phy_mode_config)(Xpon_Phy_Mode_t xpon_mode, int tx_enable) = NULL;
EXPORT_SYMBOL(set_pon_phy_mode_config);

void (*set_pon_phy_start)(void) = NULL;
EXPORT_SYMBOL(set_pon_phy_start);

void (*set_pon_phy_stop)(void) = NULL;
EXPORT_SYMBOL(set_pon_phy_stop);

void (*get_pon_phy_trans_status)(PHY_Trans_Status_t * trans_status) = NULL;
EXPORT_SYMBOL(get_pon_phy_trans_status);


static int ecnt_pon_phy_probe(struct platform_device *pdev)
{
    struct resource *res = NULL;
	int irq = 0;
	int irq_idx = 0;

	printk("\n(%s) probe start\n", __func__);

    if (!pdev->dev.of_node) {
        dev_err(&pdev->dev, "No pon phy DT node found");
        return -EINVAL;
    }

    ecnt_pon_phy = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_pon_phy), GFP_KERNEL);
    if (!ecnt_pon_phy)
        return -ENOMEM;

    platform_set_drvdata(pdev, ecnt_pon_phy);

    /* PON_PHY_ASIC_RG physical address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    ecnt_pon_phy->pon_phy_rg_base_1= devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_pon_phy->pon_phy_rg_base_1))
    {
    	devm_kfree(&pdev->dev, ecnt_pon_phy);
        return PTR_ERR(ecnt_pon_phy->pon_phy_rg_base_1);
    }
	printk("(%s) rg_base1= 0x%8x\n", __func__,ecnt_pon_phy->pon_phy_rg_base_1);

	/* PON_PHY_FPGA_RG TX_OFF physical address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    ecnt_pon_phy->pon_phy_fpga_rg_tx_off = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_pon_phy->pon_phy_fpga_rg_tx_off))
    {    	
    	devm_kfree(&pdev->dev, ecnt_pon_phy);
        return PTR_ERR(ecnt_pon_phy->pon_phy_fpga_rg_tx_off);
    }
 #ifndef TCSUPPORT_CPU_AN7552       
	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	ecnt_pon_phy->pon_phy_rg_base_2= devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ecnt_pon_phy->pon_phy_rg_base_2))
	{		
    	devm_kfree(&pdev->dev, ecnt_pon_phy);
		return PTR_ERR(ecnt_pon_phy->pon_phy_rg_base_2);
	}

	printk("(%s) rg_base2= 0x%8x\n", __func__,ecnt_pon_phy->pon_phy_rg_base_2);
	
    res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
    ecnt_pon_phy->pon_phy_rg_base_3= devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_pon_phy->pon_phy_rg_base_3))
    {
    	devm_kfree(&pdev->dev, ecnt_pon_phy);
        return PTR_ERR(ecnt_pon_phy->pon_phy_rg_base_3);
    }

	printk("(%s) rg_base3= 0x%8x\n", __func__,ecnt_pon_phy->pon_phy_rg_base_3);
	#endif

#ifdef TCSUPPORT_CPU_EN7581
	#if 0
	if(isFPGA) 
	{
		res = platform_get_resource(pdev, IORESOURCE_MEM, 4);
	    ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst= devm_ioremap_resource(&pdev->dev, res);
	    if (IS_ERR(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst))
	    {
	    	devm_kfree(&pdev->dev, ecnt_pon_phy);
	        return PTR_ERR(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst);
	    }
		
		res = platform_get_resource(pdev, IORESOURCE_MEM, 5);
	    ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl= devm_ioremap_resource(&pdev->dev, res);
	    if (IS_ERR(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl))
	    {
	    	devm_kfree(&pdev->dev, ecnt_pon_phy);
	        return PTR_ERR(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl);
	    }
	}
	#else
	if(isFPGA) 
	{
		ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst = Get_Base(PON_PHY_FPGA_RG_XILINX)+0x24; //  0x1fa84024
		ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl = Get_Base(PON_PHY_FPGA_RG_XILINX)+0x30; //  0x1fa84030
		 if (IS_ERR(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst))
	    {
			 printk("pon_phy_fpga_rg_xilinx_rst= %lx\n", (unsigned long)ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst);
	    }
	}
	ecnt_pon_phy->pon_phy_xfi_ana = Get_Base(PON_PHY_XFI_ANA_BASE); 
	if (IS_ERR(ecnt_pon_phy->pon_phy_xfi_ana))
    {
		printk("pon_phy_xfi_ana= %lx\n", (unsigned long)ecnt_pon_phy->pon_phy_xfi_ana);
    }
	
	ecnt_pon_phy->pon_phy_xfi_pma = Get_Base(PON_PHY_XFI_PMA_BASE);  
	if (IS_ERR(ecnt_pon_phy->pon_phy_xfi_pma))
    {
		printk("pon_phy_xfi_pma= %lx\n", (unsigned long)ecnt_pon_phy->pon_phy_xfi_pma);
    }
	
	ecnt_pon_phy->pon_phy_pon_ana = Get_Base(PON_PHY_PON_ANA_BASE);  
	if (IS_ERR(ecnt_pon_phy->pon_phy_pon_ana))
    {
		printk("pon_phy_pon_ana= %lx\n", (unsigned long)ecnt_pon_phy->pon_phy_pon_ana);
    }
	
	ecnt_pon_phy->pon_phy_pon_pma = Get_Base(PON_PHY_PON_PMA_BASE);  	
	if (IS_ERR(ecnt_pon_phy->pon_phy_pon_pma))
    {
		printk("pon_phy_pon_pma= %lx\n", (unsigned long)ecnt_pon_phy->pon_phy_pon_pma);
    }
	
#endif

#endif
#ifdef TCSUPPORT_CPU_AN7552
if(isFPGA) 
	{
		ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst = Get_Base(PON_PHY_FPGA_RG_XILINX)+0x24; //  0x1fa84024
		ecnt_pon_phy->pon_phy_fpga_rg_xilinx_ctl = Get_Base(PON_PHY_FPGA_RG_XILINX)+0x30; //  0x1fa84030
		 if (IS_ERR(ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst))
	    {
			 printk("pon_phy_fpga_rg_xilinx_rst= %lx\n", (unsigned long)ecnt_pon_phy->pon_phy_fpga_rg_xilinx_rst);
	    }
	}
	ecnt_pon_phy->pon_phy_pon_ana = Get_Base(PON_PHY_PON_ANA_BASE);  
	if (IS_ERR(ecnt_pon_phy->pon_phy_pon_ana))
    {
		printk("pon_phy_pon_ana= %lx\n", (unsigned long)ecnt_pon_phy->pon_phy_pon_ana);
    }
	ecnt_pon_phy->pon_phy_pon_pma = Get_Base(PON_PHY_PON_PMA_BASE); 	
	if (IS_ERR(ecnt_pon_phy->pon_phy_pon_pma))
	{
		printk("pon_phy_pon_pma= %lx\n", (unsigned long)ecnt_pon_phy->pon_phy_pon_pma);
	}

#endif

    ecnt_pon_phy->dev = &pdev->dev;
    
	//printk("spin_lock_init\n");
    spin_lock_init(&data_lock);

	/* get irq num */
	#ifndef TCSUPPORT_CPU_EN7581 //#if 0 //move to common
	//#if defined(TCSUPPORT_CPU_AN7552) || defined(TCSUPPORT_CPU_EN7523) //modify for 7583
	#ifndef TCSUPPORT_CPU_AN7583 //modify for 7583
	for( irq_idx = 0; irq_idx < XPONPHY_ALL_INT_MAX_NUM; irq_idx++ ){
		irq = platform_get_irq(pdev, irq_idx);
		if (irq <= 0)
		{
    		devm_kfree(&pdev->dev, ecnt_pon_phy);
			return irq;
		}

		ecnt_pon_phy->phy_irq = irq;
	}
	#endif 
	#endif 
    return 0;
}

static int ecnt_pon_phy_remove(struct platform_device *pdev)
{
    return 0;
}

static int ecnt_pon_phy_shutdown(struct platform_device *pdev)
{
	printk("ecnt_pon_phy_shutdown\n");
	if ( ledTurnOff_hook && !isFPGA )
	{
		ledTurnOff_hook(LED_PHY_TX_POWER_DISABLE);
	}
    return 0;
}

/************************************************************************
*      P L A T F O R M   D R I V E R S   D E C L A R A T I O N S
*************************************************************************
*/


static struct platform_driver ecnt_pon_phy_driver = {
    .probe = ecnt_pon_phy_probe,
    .remove = ecnt_pon_phy_remove,
	.shutdown = ecnt_pon_phy_shutdown,
    .driver = {
        .name = "ecnt-pon_phy",
        .of_match_table = ecnt_pon_phy_of_ids
    },
};
module_platform_driver(ecnt_pon_phy_driver);


MODULE_DESCRIPTION("EcoNet Pon Phy Driver");
