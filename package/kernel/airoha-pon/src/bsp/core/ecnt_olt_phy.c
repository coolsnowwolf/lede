/***************************************************************
SPDX-Lincense-Identifier: GPL-2.0-only
Copyright (c) 2021-2023 Airoha Inc.

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

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
//PCS
#define OLT_PHY_RG_BASE_1        	(0x1fbe5000)
#define OLT_PHY_RG_RANGE_1			(0x700)
#define OLT_PHY_RG_END_1			(OLT_PHY_RG_BASE_1 + OLT_PHY_RG_RANGE_1)

//BCDR
#define OLT_PHY_RG_BASE_2        	(0x1fbe5800)
#define OLT_PHY_RG_RANGE_2			(0xC4)
#define OLT_PHY_RG_END_2			(OLT_PHY_RG_BASE_2 + OLT_PHY_RG_RANGE_2)

//BIST
#define OLT_PHY_RG_BASE_3        	(0x1fbe5900)
#define OLT_PHY_RG_RANGE_3			(0x3C)
#define OLT_PHY_RG_END_3			(OLT_PHY_RG_BASE_3 + OLT_PHY_RG_RANGE_3)


#define OLTPHY_ALL_INT_MAX_NUM		(1)

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

struct ecnt_olt_phy {
	struct device *dev;
	void __iomem *olt_phy_rg_base_1;
	void __iomem *olt_phy_rg_base_2;
	void __iomem *olt_phy_rg_base_3;
	int olt_phy_irq;
};

static spinlock_t	 data_lock;

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
struct ecnt_olt_phy *ecnt_olt_phy = NULL;

static const struct of_device_id ecnt_olt_phy_of_ids[] = {
	{ .compatible = "econet,ecnt-olt_phy"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ecnt_olt_phy_of_ids);

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern void __iomem* Get_Base(u32 base);

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
u32 get_olt_phy_asic_data_1(u32 reg)
{
	return readl(ecnt_olt_phy->olt_phy_rg_base_1 + reg);
}

void set_olt_phy_asic_data_1(u32 reg, u32 val)
{
	writel(val, ecnt_olt_phy->olt_phy_rg_base_1 + reg); 
}

u32 get_olt_phy_asic_data_2(u32 reg)
{
	return readl(ecnt_olt_phy->olt_phy_rg_base_2 + reg);
}

void set_olt_phy_asic_data_2(u32 reg, u32 val)
{
	writel(val, ecnt_olt_phy->olt_phy_rg_base_2 + reg); 
}

u32 get_olt_phy_asic_data_3(u32 reg)
{
	return readl(ecnt_olt_phy->olt_phy_rg_base_3 + reg);
}

void set_olt_phy_asic_data_3(u32 reg, u32 val)
{
	writel(val, ecnt_olt_phy->olt_phy_rg_base_3 + reg); 
}

/***********************************************/
/* APIs */
/***********************************************/
u32 get_olt_phy_data(u32 reg)
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

	if( (OLT_PHY_RG_BASE_1 <= reg_phy) && (reg_phy < OLT_PHY_RG_END_1) )
	{	
		reg_val= get_olt_phy_asic_data_1(reg_phy - OLT_PHY_RG_BASE_1);
	}
	else if( (OLT_PHY_RG_BASE_2 <= reg_phy) && (reg_phy < OLT_PHY_RG_END_2) )
	{	
		reg_val= get_olt_phy_asic_data_2(reg_phy - OLT_PHY_RG_BASE_2);
	}
	else if( (OLT_PHY_RG_BASE_3 <= reg_phy) && (reg_phy < OLT_PHY_RG_END_3) )
	{	
		reg_val= get_olt_phy_asic_data_3(reg_phy - OLT_PHY_RG_BASE_3);
	}
	else
	{
		printk("\n Datapath(%s) get reg error, reg=0x%08X\n", __func__, reg);
	}

	spin_unlock_irqrestore(&data_lock, flags);
	return reg_val;
}
EXPORT_SYMBOL(get_olt_phy_data);


void set_olt_phy_data(u32 reg, u32 val)
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

	if( (OLT_PHY_RG_BASE_1<= reg_phy) && (reg_phy < OLT_PHY_RG_END_1) )
	{			
		set_olt_phy_asic_data_1(reg_phy - OLT_PHY_RG_BASE_1, val); 
	}
	else if( (OLT_PHY_RG_BASE_2<= reg_phy) && (reg_phy < OLT_PHY_RG_END_2) )
	{			
		set_olt_phy_asic_data_2(reg_phy - OLT_PHY_RG_BASE_2, val); 
	}
	else if( (OLT_PHY_RG_BASE_3<= reg_phy) && (reg_phy < OLT_PHY_RG_END_3) )
	{			
		set_olt_phy_asic_data_3(reg_phy - OLT_PHY_RG_BASE_3, val); 
	}
	else
	{
		printk("\nDatapath(%s) set reg error, reg=0x%08X\n", __func__, reg);
	}

	spin_unlock_irqrestore(&data_lock, flags);

}
EXPORT_SYMBOL(set_olt_phy_data);
/***********************************************/


struct device* get_olt_phy_dev(void)
{
	if( (ecnt_olt_phy) && (ecnt_olt_phy->dev) )
		return ecnt_olt_phy->dev;
	else
		return NULL;
}
EXPORT_SYMBOL(get_olt_phy_dev);

int get_olt_phy_irq(void)
{
	return ecnt_olt_phy->olt_phy_irq;
	
}
EXPORT_SYMBOL(get_olt_phy_irq);

static int ecnt_olt_phy_probe(struct platform_device *pdev)
{
    struct resource *res = NULL;
	int irq = 0;
	int irq_idx = 0;

	printk("\n\e[1;33m%s start\e[0m\n", __func__);

    if (!pdev->dev.of_node) {
        dev_err(&pdev->dev, "No OLT phy DT node found");
        return -EINVAL;
    }

    ecnt_olt_phy = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_olt_phy), GFP_KERNEL);
    if (!ecnt_olt_phy)
        return -ENOMEM;

    platform_set_drvdata(pdev, ecnt_olt_phy);

    /* OLT_PHY_ASIC_RG physical address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    ecnt_olt_phy->olt_phy_rg_base_1= devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_olt_phy->olt_phy_rg_base_1))
    {
    	devm_kfree(&pdev->dev, ecnt_olt_phy);
        return PTR_ERR(ecnt_olt_phy->olt_phy_rg_base_1);
    }
	printk("(%s) rg_base1= 0x%8x\n", __func__,ecnt_olt_phy->olt_phy_rg_base_1);


	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	ecnt_olt_phy->olt_phy_rg_base_2= devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ecnt_olt_phy->olt_phy_rg_base_2))
	{
		devm_kfree(&pdev->dev, ecnt_olt_phy);
		return PTR_ERR(ecnt_olt_phy->olt_phy_rg_base_2);
	}
	printk("(%s) rg_base2= 0x%8x\n", __func__,ecnt_olt_phy->olt_phy_rg_base_2);


	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	ecnt_olt_phy->olt_phy_rg_base_3= devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ecnt_olt_phy->olt_phy_rg_base_3))
	{
		devm_kfree(&pdev->dev, ecnt_olt_phy);
		return PTR_ERR(ecnt_olt_phy->olt_phy_rg_base_3);
	}
	printk("(%s) rg_base3= 0x%8x\n", __func__,ecnt_olt_phy->olt_phy_rg_base_3);


    spin_lock_init(&data_lock);

	irq = platform_get_irq(pdev, irq_idx);
	if (irq <= 0)
	{
		devm_kfree(&pdev->dev, ecnt_olt_phy);
		return irq;
	}
	
	ecnt_olt_phy->olt_phy_irq = irq;

    return 0;
}

static int ecnt_olt_phy_remove(struct platform_device *pdev)
{
	printk("\n%s\n\n", __func__);
    return 0;
}

static int ecnt_olt_phy_shutdown(struct platform_device *pdev)
{
	printk("\n%s\n\n", __func__);
    return 0;
}

/************************************************************************
*      P L A T F O R M   D R I V E R S   D E C L A R A T I O N S
*************************************************************************
*/

static struct platform_driver ecnt_olt_phy_driver = {
    .probe = ecnt_olt_phy_probe,
    .remove = ecnt_olt_phy_remove,
    .shutdown = ecnt_olt_phy_shutdown,
    .driver = {
        .name = "ecnt-olt_phy",
        .of_match_table = ecnt_olt_phy_of_ids
    },
};

module_platform_driver(ecnt_olt_phy_driver);

MODULE_DESCRIPTION("AIROHA OLT Phy Driver");
