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

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define EPON_BASE_OFFSET 0x6000
#define GPON_BASE_OFFSET 0x4000

#ifdef TCSUPPORT_CPU_EN7581
#define XGPON_BASE_OFFSET 0x5000
#endif

#define BASE_RANGE 0x1000

#define XPON_MAC_INT_MAX_NUM	(2)

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
struct ecnt_xpon_str {
	struct device *dev;
	void __iomem *gpon_base;
	void __iomem *epon_base;
#ifdef TCSUPPORT_CPU_EN7581
	void __iomem *xgpon_base;
#endif
	int xpon_irq[XPON_MAC_INT_MAX_NUM];
};

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
struct ecnt_xpon_str *ecnt_xpon = NULL;

static const struct of_device_id ecnt_xpon_of_id[] = {
    { .compatible = "econet,ecnt-xpon"},
    { /* sentinel */}
};
MODULE_DEVICE_TABLE(of, ecnt_xpon_of_id);
/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
/*****************************************************************
 ****  N P    S C U    a c c e s s ********************************
 ******************************************************************/
u32 get_gpon_data(u32 reg)
{
	return readl(ecnt_xpon->gpon_base + reg);
}
u32 get_epon_data(u32 reg)
{
    return readl(ecnt_xpon->epon_base + reg);
}

void set_gpon_data(u32 reg, u32 val)
{
    writel(val, ecnt_xpon->gpon_base + reg); 
}

void set_epon_data(u32 reg, u32 val)
{
    writel(val, ecnt_xpon->epon_base + reg); 
}

u32 read_gpon_data(u32 reg)
{
	reg = reg - GPON_BASE_OFFSET;
	return  get_gpon_data(reg);
}

u32 read_epon_data(u32 reg)
{
	reg = reg - EPON_BASE_OFFSET;
	return  get_epon_data(reg);
}

void write_gpon_data(u32 reg, u32 val)
{
	reg = reg - GPON_BASE_OFFSET;
	set_gpon_data(reg,val);
}

void write_epon_data(u32 reg, u32 val)
{
	reg = reg - EPON_BASE_OFFSET;
	set_epon_data(reg,val);
}


#ifdef TCSUPPORT_CPU_EN7581
/*****************/
u32 get_xgpon_data(u32 reg)
{
	return readl(ecnt_xpon->xgpon_base + reg);
}

void set_xgpon_data(u32 reg, u32 val)
{
    writel(val, ecnt_xpon->xgpon_base + reg); 
}

void write_xgpon_data(u32 reg, u32 val)
{
	reg = reg - XGPON_BASE_OFFSET;
	set_xgpon_data(reg,val);
}
u32 read_xgpon_data(u32 reg)
{
	reg = reg - XGPON_BASE_OFFSET;
	return  get_xgpon_data(reg);
}
#endif


/* APIs */
u32 get_xpon_data(u32 reg)
{
	if( (GPON_BASE_OFFSET <= reg) && (reg < (GPON_BASE_OFFSET+BASE_RANGE)) )
    	return read_gpon_data(reg);
	else if( (EPON_BASE_OFFSET <= reg) && (reg < (EPON_BASE_OFFSET+BASE_RANGE)) )
    	return read_epon_data(reg);
#ifdef TCSUPPORT_CPU_EN7581
	else if( (XGPON_BASE_OFFSET <= reg) && (reg < (XGPON_BASE_OFFSET+BASE_RANGE)) )
    	return read_xgpon_data(reg);
#endif
	else
		printk("\nxpon(%s) (%d)get reg error, reg=0x%08X\n", __func__,__LINE__, reg);

	return 0;
}
EXPORT_SYMBOL(get_xpon_data);

void set_xpon_data(u32 reg, u32 val)
{
	if( (GPON_BASE_OFFSET <= reg) && (reg < (GPON_BASE_OFFSET+BASE_RANGE)) )
    	write_gpon_data(reg, val);
	else if( (EPON_BASE_OFFSET <= reg) && (reg < (EPON_BASE_OFFSET+BASE_RANGE)) )
    	write_epon_data(reg, val);
#ifdef TCSUPPORT_CPU_EN7581
	else if( (XGPON_BASE_OFFSET <= reg) && (reg < (XGPON_BASE_OFFSET+BASE_RANGE)) )
			write_xgpon_data(reg, val);
#endif
	else
		printk("\nxpon(%s) (%d)set reg error, reg=0x%08X\n", __func__,__LINE__, reg);
}
EXPORT_SYMBOL(set_xpon_data);

static int ecnt_xpon_drv_probe(struct platform_device *pdev)
{
    struct resource *res = NULL;
	int irq_idx = 0;
	int irq = -1;

    if (!pdev->dev.of_node) {
        dev_err(&pdev->dev, "No xpon DT node found");
        return -EINVAL;
    }

    ecnt_xpon = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_xpon_str), GFP_KERNEL);
    if (!ecnt_xpon)
        return -ENOMEM;

    platform_set_drvdata(pdev, ecnt_xpon);

    /* get GPON MAC base address */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    ecnt_xpon->gpon_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_xpon->gpon_base))
        return PTR_ERR(ecnt_xpon->gpon_base);
	
    /* get EPON MAC base address */	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
   	ecnt_xpon->epon_base = devm_ioremap_resource(&pdev->dev, res);
   	if (IS_ERR(ecnt_xpon->epon_base))
	   return PTR_ERR(ecnt_xpon->epon_base);

#ifdef TCSUPPORT_CPU_EN7581
	/* get XGPON MAC base address */ 
	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	ecnt_xpon->xgpon_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ecnt_xpon->xgpon_base))
	   return PTR_ERR(ecnt_xpon->xgpon_base);
#endif

    ecnt_xpon->dev = &pdev->dev;

	/* get irq num */
	for( irq_idx = 0; irq_idx < XPON_MAC_INT_MAX_NUM; irq_idx++ ){
		irq = platform_get_irq(pdev, irq_idx);
		if (irq <= 0)
			return irq;
		ecnt_xpon->xpon_irq[irq_idx] = irq;
		//debug by da
		printk("irq-%d: %d ", irq_idx, irq);
	}
#if 0
	printk("[xpon] res->name:%s\n", res->name);
	printk("[xpon] res->start:0x%llx ===\n", res->start);
	printk("[xpon] res->end:0x%llx ===\n", res->end);
	printk("[xpon] ecnt_xpon->base:0x%lx\n", (unsigned long)ecnt_xpon->base);
#endif

    return 0;
}

int get_xpon_irq(int index)
{
	return ecnt_xpon->xpon_irq[index];
}

EXPORT_SYMBOL(get_xpon_irq);

struct device* get_xpon_dev(void)
{
    if ((ecnt_xpon) && (ecnt_xpon->dev))
        return ecnt_xpon->dev;
    else
        return NULL;
}

EXPORT_SYMBOL(get_xpon_dev);
static int ecnt_xpon_drv_remove(struct platform_device *pdev)
{
    return 0;
}

/************************************************************************
*      P L A T F O R M   D R I V E R S   D E C L A R A T I O N S
*************************************************************************
*/
static struct platform_driver ecnt_xpon_driver = {
    .probe = ecnt_xpon_drv_probe,
    .remove = ecnt_xpon_drv_remove,
    .driver = {
	    .name = "ecnt-xpon",
	    .of_match_table = ecnt_xpon_of_id
    },
};
module_platform_driver(ecnt_xpon_driver);


MODULE_DESCRIPTION("EcoNet XPON Driver");

