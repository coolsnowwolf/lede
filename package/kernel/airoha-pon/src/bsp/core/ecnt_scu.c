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
#include <linux/interrupt.h>
#include <asm/io.h>

#include <modules/scu/ecnt_scu.h>
#include <ecnt_hook/ecnt_hook_dgasp.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/* NP SCU */
#define CR_NP_SCU_SMB0       	(CR_NP_SCU_BASE + 0x20)
#define CR_NP_SCU_SMB1       	(CR_NP_SCU_BASE + 0x24)
#define CR_NP_SCU_SMB2       	(CR_NP_SCU_BASE + 0x28)
#define CR_NP_SCU_SMB5       	(CR_NP_SCU_BASE + 0x34)
#define CR_NP_SCU_SMB0ALS      	(CR_NP_SCU_BASE + 0x38)
#define CR_NP_SCU_PB_TO_ERR		(CR_NP_SCU_BASE + 0x3c)
#define CR_NP_SCU_PSTCR			(CR_NP_SCU_BASE + 0x40)
#define CR_NP_SCU_PRATIR      	(CR_NP_SCU_BASE + 0x58)
#define CR_NP_SCU_PDIDR     	(CR_NP_SCU_BASE + 0x5C)
#define CR_NP_SCU_MON_TMR     	(CR_NP_SCU_BASE + 0x60)
#define CR_NP_SCU_HIR	     	(CR_NP_SCU_BASE + 0x64)
#define CR_NP_WAN_CONF			(CR_NP_SCU_BASE + 0x70)
#define CR_NP_DRAMC_CONF		(CR_NP_SCU_BASE + 0x74)
#define CR_NP_SCU_PMC       	(CR_NP_SCU_BASE + 0x80)
#define CR_NP_SCU_DMTC       	(CR_NP_SCU_BASE + 0x84)
#define CR_NP_SCU_PCIC	       	(CR_NP_SCU_BASE + 0x88)
#define CR_NP_SCU_HWCONF       	(CR_NP_SCU_BASE + 0x8C)
#define CR_NP_SCU_SSR       	(CR_NP_SCU_BASE + 0x90)
#define CR_NP_SCU_SSR3       	(CR_NP_SCU_BASE + 0x94)
#define CR_NP_SCU_SSTR			(CR_NP_SCU_BASE + 0x9C)
#define CR_NP_SCU_UHCR			(CR_NP_SCU_BASE + 0xA8)
#define CR_NP_SCU_BOOT_TRAP_CONF_DEC	(CR_NP_SCU_BASE + 0xB8)
#define CR_NP_SCU_CKGEN_CONF	(CR_NP_SCU_BASE + 0xC0)
#define CR_NP_SCU_MDIO0IAC		(CR_NP_SCU_BASE + 0xC8)
#define CR_NP_SCU_MDIO1IAC		(CR_NP_SCU_BASE + 0xCC)
#define CR_NP_SCU_HWCONF2		(CR_NP_SCU_BASE + 0xF8)
#define CR_NP_SCU_SCREG_WF0		(CR_NP_SCU_BASE + 0x240)	//Watch Dog Free Scratch Register 0
#define CR_NP_SCU_SCREG_WF1		(CR_NP_SCU_BASE + 0x244)	//Watch Dog Free Scratch Register 1
#define CR_NP_SCU_SCREG_WR0		(CR_NP_SCU_BASE + 0x280)	//Watch Dog Reset Scratch Register 0
#define CR_NP_SCU_SCREG_WR1		(CR_NP_SCU_BASE + 0x284)	//Watch Dog Reset Scratch Register 1
#define CR_NP_SCU_GPIO_SSR		(CR_NP_SCU_BASE + 0x860)
#define CR_NP_SCU_CLK_CFG		(CR_NP_SCU_BASE + 0x82c)	
#define CR_NP_SCU_RST_CFG		(CR_NP_SCU_BASE + 0x830)	//RG_SCU_RST	
#define CR_NP_SCU_RSTCTRL1    	(CR_NP_SCU_BASE + 0x834)			
#define CR_NP_SCU_RST_ACC_CHK_BMAP	(CR_NP_SCU_BASE + 0x92c)
#define CR_NP_SCU_SHARE_UNZIPMEM_SEL	(CR_NP_SCU_BASE + 0x954)
#define CR_NP_SCU_SHARE_FEMEM_SEL		(CR_NP_SCU_BASE + 0x958)

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define CR_NP_SCU_SSTR_SERDES_USB0_SEL_OFFSET		(29)
#define CR_NP_SCU_SSTR_SERDES_USB0_SEL_MASK			(0x1 << CR_NP_SCU_SSTR_SERDES_USB0_SEL_OFFSET)
#define CR_NP_SCU_SSTR_SERDES_USB1_SEL_OFFSET		(3)
#define CR_NP_SCU_SSTR_SERDES_USB1_SEL_MASK			(0x1 << CR_NP_SCU_SSTR_SERDES_USB1_SEL_OFFSET)
#define CR_NP_SCU_SSTR_SERDES_PON_SEL_OFFSET		(9)
#define CR_NP_SCU_SSTR_SERDES_PON_SEL_MASK			(0x3 << CR_NP_SCU_SSTR_SERDES_PON_SEL_OFFSET)
#define CR_NP_SCU_SSTR_SERDES_WIFI1_SEL_OFFSET		(11)
#define CR_NP_SCU_SSTR_SERDES_WIFI1_SEL_MASK		(0x3 << CR_NP_SCU_SSTR_SERDES_WIFI1_SEL_OFFSET)
#define CR_NP_SCU_SSTR_SERDES_WIFI0_SEL_OFFSET		(13)
#define CR_NP_SCU_SSTR_SERDES_WIFI0_SEL_MASK		(0x3 << CR_NP_SCU_SSTR_SERDES_WIFI0_SEL_OFFSET)
#define CR_NP_SCU_SSTR_SERDES_ETHERNET_SEL_OFFSET	(13)
#define CR_NP_SCU_SSTR_SERDES_ETHERNET_SEL_MASK		(0x3 << CR_NP_SCU_SSTR_SERDES_ETHERNET_SEL_OFFSET)
#endif

/* CHIP SCU */
#define CR_CHIP_SCU_RGS_OPEN_DRAIN	(CR_CHIP_SCU_BASE + 0x018C)
#define CR_CHIP_SCU_RGS_CLK_GSW	(CR_CHIP_SCU_BASE + 0x01B4)
#define CR_CHIP_SCU_RGS_CLK_EMI	(CR_CHIP_SCU_BASE + 0x01B8)
#define CR_CHIP_SCU_RGS_CLK_BUS	(CR_CHIP_SCU_BASE + 0x01BC)
#define CR_CHIP_SCU_RGS_CLK_FE	(CR_CHIP_SCU_BASE + 0x01C0)
#define CR_CHIP_SCU_RGS_CLK_NPU	(CR_CHIP_SCU_BASE + 0x01FC)
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define CR_CHIP_SCU_RGS_ECC_SEL	(CR_CHIP_SCU_BASE + 0x0258)
#define CR_CHIP_SCU_TOP_REV_24	(CR_CHIP_SCU_BASE + 0x036C)
#else /* TCSUPPORT_CPU_EN7523 */
#define CR_CHIP_SCU_RGS_ECC_SEL	(CR_CHIP_SCU_BASE + 0x0254)
#endif

#define CR_RBUS_CTL_CLK_BASE	0x00000000
#define CR_RBUS_CLK_CTL			(CR_RBUS_CTL_CLK_BASE+0x8)

#define OPEN_DRAIN_MASK     0x7 //bit[2:0] mapping PCIe port2/1/0
#define GET_REG_SAVE_INFO				((SYS_GLOBAL_PARM_T){ .word = get_np_scu_data(CR_NP_SCU_SCREG_WR1)})
#define SET_REG_SAVE_INFO(x)			(set_np_scu_data(CR_NP_SCU_SCREG_WR1, x))

#define CR_CHIP_SCU_IOMUX_NUM        7
u32 CHIP_SCU_IOMUX_REG[CR_CHIP_SCU_IOMUX_NUM] = {
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
	(CR_CHIP_SCU_BASE + 0x214),  //IOMUX_CTRL_1
	(CR_CHIP_SCU_BASE + 0x218),  //IOMUX_CTRL_2
	(CR_CHIP_SCU_BASE + 0x21C),  //IOMUX_CTRL_3
	(CR_CHIP_SCU_BASE + 0x220),  //IOMUX_CTRL_4
	(CR_CHIP_SCU_BASE + 0x224),  //IOMUX_CTRL_5
	(CR_CHIP_SCU_BASE + 0x228),  //IOMUX_CTRL_6
	(CR_CHIP_SCU_BASE + 0x22C),  //IOMUX_CTRL_7
	(CR_CHIP_SCU_BASE + 0x20C),  //IOMUX_CTRL_8
	(CR_CHIP_SCU_BASE + 0x4C),  //IOMUX_CTRL_9
#else /* TCSUPPORT_CPU_EN7523 */
    (CR_CHIP_SCU_BASE + 0x210),  //IOMUX_CTRL_1
    (CR_CHIP_SCU_BASE + 0x214),  //IOMUX_CTRL_2
    (CR_CHIP_SCU_BASE + 0x218),  //IOMUX_CTRL_3
    (CR_CHIP_SCU_BASE + 0x21c),  //IOMUX_CTRL_4
    (CR_CHIP_SCU_BASE + 0x220),  //IOMUX_CTRL_5
    (CR_CHIP_SCU_BASE + 0x224),  //IOMUX_CTRL_6
    (CR_CHIP_SCU_BASE + 0x228),  //IOMUX_CTRL_7
#endif
};

#ifdef TCSUPPORT_CPU_EN7581
#define FPGA_PBUS_CLK   (0x02800000) /*40MHz*/
#else
#define FPGA_PBUS_CLK   (0x04000000) /*60MHz*/
#endif
#define ASIC_PBUS_CLK   (0x10000000) /*235MHz*/
#define PBUS_TOUT_EN    (0x40000000)


/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
struct ecnt_scu {
	struct device *dev;
	void __iomem *npScu_base; /* NP SCU base virtual address */
	void __iomem *chipScu_base; /* NP SCU base virtual address */
	void __iomem *clk_ctl_base;
	u32 irq;
};

#ifdef TCSUPPORT_DYING_GASP
typedef void (*cb_dyinggasp_t)(void *func);
typedef struct DYING_GASP_LIST
{
	struct list_head list;
	char name[10];
	cb_dyinggasp_t cb_dgasp_fn;
	void *context;
	
}CB_DGASP_LIST;

CB_DGASP_LIST *cb_dgasp_list_head = NULL;

static DEFINE_MUTEX(dying_gasp_lock);
#endif

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
struct ecnt_scu *ecnt_scu = NULL;
#if 0
static const struct of_device_id ecnt_scu_of_ids[] = {
	{ .compatible = "econet,ecnt-scu"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ecnt_scu_of_ids);
#endif

static unsigned int l2c_sram_size=0;
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
/* don't EXPORT this function. Create API for your purpose instead. */
u32 get_np_scu_data(u32 reg)
{
	return readl(ecnt_scu->npScu_base + reg);
}

/* don't EXPORT this function. Create API for your purpose instead. */
void set_np_scu_data(u32 reg, u32 val)
{
	writel(val, ecnt_scu->npScu_base + reg); 
}

u32 get_npScuReg_bits(u32 reg, u32 mask, u32 offset)
{
    u32 val = get_np_scu_data(reg);

    val &= mask;
    val >>= offset;
    return val;
}

void set_npScuReg_bits(u32 reg, u32 mask, u32 bits)
{
    u32 val = get_np_scu_data(reg);

    bits &= mask;
    val &= (~mask);
    val |= bits;

    set_np_scu_data(reg, val);
}

u32 GET_IS_DDR4(void)
{
	return GET_REG_SAVE_INFO.raw.isDDR4;
}

void SET_IS_DDR4(u32 val)
{												
	SYS_GLOBAL_PARM_T z = GET_REG_SAVE_INFO;	
	z.raw.isDDR4 = val;							
	SET_REG_SAVE_INFO(z.word);					
}

u32 GET_DRAM_SIZE(void)
{
	return (GET_REG_SAVE_INFO.raw.dram_size << 4);
}
EXPORT_SYMBOL(GET_DRAM_SIZE);

void SET_DRAM_SIZE(u32 val)		
{												
	SYS_GLOBAL_PARM_T z = GET_REG_SAVE_INFO;	
	z.raw.dram_size = (val >> 4);						
	SET_REG_SAVE_INFO(z.word);					
}

u32 GET_SYS_CLK(void)
{
	return GET_REG_SAVE_INFO.raw.sys_clk;
}
EXPORT_SYMBOL(GET_SYS_CLK);


void SET_SYS_CLK(u32 val)			
{												
	SYS_GLOBAL_PARM_T z = GET_REG_SAVE_INFO;	
	z.raw.sys_clk = val;						
	SET_REG_SAVE_INFO(z.word);					
}

u32 GET_IS_FPGA(void)
{
	return GET_REG_SAVE_INFO.raw.isFpga;
}
EXPORT_SYMBOL(GET_IS_FPGA);

void SET_IS_FPGA(u32 val)			
{
	SYS_GLOBAL_PARM_T z = GET_REG_SAVE_INFO;	
	z.raw.isFpga = val;						
	SET_REG_SAVE_INFO(z.word);
}

u32 GET_IS_SPI_CONTROLLER_ECC(void)
{
	return GET_REG_SAVE_INFO.raw.isCtrlEcc;
}

void SET_IS_SPI_CONTROLLER_ECC(u32 val)			
{												
	SYS_GLOBAL_PARM_T z = GET_REG_SAVE_INFO;	
	z.raw.isCtrlEcc = val;						
	SET_REG_SAVE_INFO(z.word);					
}

u32 GET_PACKAGE_ID(void)
{
	u32 id;

	id = (GET_REG_SAVE_INFO.raw.packageID | (GET_REG_SAVE_INFO.raw.packageID_ext << 4));
	return id;
}
EXPORT_SYMBOL(GET_PACKAGE_ID);

#ifdef TCSUPPORT_SECURE_BOOT
u32 GET_IS_SECURE_MODE(void)
{
	return GET_REG_SAVE_INFO.raw.isSecureModeEn;
}

void SET_IS_SECURE_MODE(u32 val)	
{												
	SYS_GLOBAL_PARM_T z = GET_REG_SAVE_INFO;	
	z.raw.isSecureModeEn = val;					
	SET_REG_SAVE_INFO(z.word);					
}

u32 GET_IS_SECURE_HWTRAP(void)
{
	return GET_REG_SAVE_INFO.raw.isSecureHwTrapEn;
}

void SET_IS_SECURE_HWTRAP(u32 val)	
{												
	SYS_GLOBAL_PARM_T z = GET_REG_SAVE_INFO;	
	z.raw.isSecureHwTrapEn = val;				
	SET_REG_SAVE_INFO(z.word);					
}
#endif

u32 GET_PDIDR(void)
{
	return (get_np_scu_data(CR_NP_SCU_PDIDR) & (0xFFFF));
}
EXPORT_SYMBOL(GET_PDIDR);

u32 GET_HIR(void)
{
	return ((get_np_scu_data(CR_NP_SCU_HIR) >> 16) & (0xFFFF));
}
EXPORT_SYMBOL(GET_HIR);

u32 GET_SMB0ALS(void)
{
	return get_np_scu_data(CR_NP_SCU_SMB0ALS);
}

u32 GET_MDIOIAC(int bank)
{
	if(bank== 0 )
	{
		return get_np_scu_data(CR_NP_SCU_MDIO0IAC);
	}
	else
	{
		return get_np_scu_data(CR_NP_SCU_MDIO1IAC);
	}
	
}

void SET_MDIOIAC(int bank,u32 data)
{
	if(bank== 0 )
	{
		return set_np_scu_data(CR_NP_SCU_MDIO0IAC, data);
	}
	else
	{
		return set_np_scu_data(CR_NP_SCU_MDIO1IAC, data);
	}
}

EXPORT_SYMBOL(SET_MDIOIAC);
EXPORT_SYMBOL(GET_MDIOIAC);

u32 GET_HWCONF(void)
{
	return get_np_scu_data(CR_NP_SCU_HWCONF);
}

void SET_SMB0ALS(u32 data)
{
	set_np_scu_data(CR_NP_SCU_SMB0ALS, data);
}

u32 GET_GPIO_SSR(void)
{
    return get_np_scu_data(CR_NP_SCU_GPIO_SSR);
}
EXPORT_SYMBOL(GET_GPIO_SSR);

void SET_GPIO_SSR(u32 val)
{
    set_np_scu_data(CR_NP_SCU_GPIO_SSR, val);
    return;
}
EXPORT_SYMBOL(SET_GPIO_SSR);

u32 GET_WAN_CONF(void)
{
    return get_np_scu_data(CR_NP_WAN_CONF);
}
EXPORT_SYMBOL(GET_WAN_CONF);

void SET_WAN_CONF(u32 val)
{
    set_np_scu_data(CR_NP_WAN_CONF, val);
    return;
}
EXPORT_SYMBOL(SET_WAN_CONF);

u32 GET_DRAMC_CONF(void)
{
    return get_np_scu_data(CR_NP_DRAMC_CONF);
}
EXPORT_SYMBOL(GET_DRAMC_CONF);

void SET_DRAMC_CONF(u32 val)
{
    set_np_scu_data(CR_NP_DRAMC_CONF, val);
    return;
}
EXPORT_SYMBOL(SET_DRAMC_CONF);

u32 GET_DMTC(void)
{
    return get_np_scu_data(CR_NP_SCU_DMTC);
}
EXPORT_SYMBOL(GET_DMTC);

void SET_DMTC(u32 val)
{
    set_np_scu_data(CR_NP_SCU_DMTC, val);
    return;
}
EXPORT_SYMBOL(SET_DMTC);

/*=====================for PCIe reset begin=========================*/
//#define CR_NP_SCU_PCIC	       	(CR_NP_SCU_BASE + 0x88)
u32 GET_PCIC(void)
{
    return get_np_scu_data(CR_NP_SCU_PCIC);
}
EXPORT_SYMBOL(GET_PCIC);

void SET_PCIC(u32 val)
{
    set_np_scu_data(CR_NP_SCU_PCIC, val);
    return;
}
EXPORT_SYMBOL(SET_PCIC);
/*=====================for PCIe reset end============================*/

u32 GET_SSR3(void)
{
    return get_np_scu_data(CR_NP_SCU_SSR3);
}
EXPORT_SYMBOL(GET_SSR3);

void SET_SSR3(u32 val)
{
    set_np_scu_data(CR_NP_SCU_SSR3, val);
    return;
}
EXPORT_SYMBOL(SET_SSR3);

u32 GET_NP_SCU_SSTR(void)
{
	return get_np_scu_data(CR_NP_SCU_SSTR);
}
EXPORT_SYMBOL(GET_NP_SCU_SSTR);

void SET_NP_SCU_SSTR(u32 val)
{
    set_np_scu_data(CR_NP_SCU_SSTR, val);
    return;
}
EXPORT_SYMBOL(SET_NP_SCU_SSTR);

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
u32 GET_NP_SCU_SERDES_ETHERNET_SEL(void)
{
	return get_npScuReg_bits(CR_NP_SCU_SSR3, CR_NP_SCU_SSTR_SERDES_ETHERNET_SEL_MASK, CR_NP_SCU_SSTR_SERDES_ETHERNET_SEL_OFFSET);
}
EXPORT_SYMBOL(GET_NP_SCU_SERDES_ETHERNET_SEL);

void SET_NP_SCU_SERDES_ETHERNET_SEL(u32 val)
{
	val <<= CR_NP_SCU_SSTR_SERDES_ETHERNET_SEL_OFFSET;
	set_npScuReg_bits(CR_NP_SCU_SSR3, CR_NP_SCU_SSTR_SERDES_ETHERNET_SEL_MASK, val);
}
EXPORT_SYMBOL(SET_NP_SCU_SERDES_ETHERNET_SEL);

u32 GET_NP_SCU_SERDES_USB0_SEL(void)
{
	return get_npScuReg_bits(CR_NP_SCU_SSR3, CR_NP_SCU_SSTR_SERDES_USB0_SEL_MASK, CR_NP_SCU_SSTR_SERDES_USB0_SEL_OFFSET);
}
EXPORT_SYMBOL(GET_NP_SCU_SERDES_USB0_SEL);

void SET_NP_SCU_SERDES_USB0_SEL(u32 val)
{
	val <<= CR_NP_SCU_SSTR_SERDES_USB0_SEL_OFFSET;
	set_npScuReg_bits(CR_NP_SCU_SSR3, CR_NP_SCU_SSTR_SERDES_USB0_SEL_MASK, val);
}
EXPORT_SYMBOL(SET_NP_SCU_SERDES_USB0_SEL);

u32 GET_NP_SCU_SERDES_USB1_SEL(void)
{
	return get_npScuReg_bits(CR_NP_SCU_SSTR, CR_NP_SCU_SSTR_SERDES_USB1_SEL_MASK, CR_NP_SCU_SSTR_SERDES_USB1_SEL_OFFSET);
}
EXPORT_SYMBOL(GET_NP_SCU_SERDES_USB1_SEL);

void SET_NP_SCU_SERDES_USB1_SEL(u32 val)
{
	val <<= CR_NP_SCU_SSTR_SERDES_USB1_SEL_OFFSET;
	set_npScuReg_bits(CR_NP_SCU_SSTR, CR_NP_SCU_SSTR_SERDES_USB1_SEL_MASK, val);
}
EXPORT_SYMBOL(SET_NP_SCU_SERDES_USB1_SEL);

u32 GET_NP_SCU_SERDES_PON_SEL(void)
{
	return get_npScuReg_bits(CR_NP_SCU_SSTR, CR_NP_SCU_SSTR_SERDES_PON_SEL_MASK, CR_NP_SCU_SSTR_SERDES_PON_SEL_OFFSET);
}
EXPORT_SYMBOL(GET_NP_SCU_SERDES_PON_SEL);

void SET_NP_SCU_SERDES_PON_SEL(u32 val)
{
	val <<= CR_NP_SCU_SSTR_SERDES_PON_SEL_OFFSET;
	set_npScuReg_bits(CR_NP_SCU_SSTR, CR_NP_SCU_SSTR_SERDES_PON_SEL_MASK, val);
}
EXPORT_SYMBOL(SET_NP_SCU_SERDES_PON_SEL);

u32 GET_NP_SCU_SERDES_WIFI0_SEL(void)
{
	return get_npScuReg_bits(CR_NP_SCU_SSTR, CR_NP_SCU_SSTR_SERDES_WIFI0_SEL_MASK, CR_NP_SCU_SSTR_SERDES_WIFI0_SEL_OFFSET);
}
EXPORT_SYMBOL(GET_NP_SCU_SERDES_WIFI0_SEL);

void SET_NP_SCU_SERDES_WIFI0_SEL(u32 val)
{
	val <<= CR_NP_SCU_SSTR_SERDES_WIFI0_SEL_OFFSET;
	set_npScuReg_bits(CR_NP_SCU_SSTR, CR_NP_SCU_SSTR_SERDES_WIFI0_SEL_MASK, val);
}
EXPORT_SYMBOL(SET_NP_SCU_SERDES_WIFI0_SEL);

u32 GET_NP_SCU_SERDES_WIFI1_SEL(void)
{
	return get_npScuReg_bits(CR_NP_SCU_SSTR, CR_NP_SCU_SSTR_SERDES_WIFI1_SEL_MASK, CR_NP_SCU_SSTR_SERDES_WIFI1_SEL_OFFSET);
}
EXPORT_SYMBOL(GET_NP_SCU_SERDES_WIFI1_SEL);

void SET_NP_SCU_SERDES_WIFI1_SEL(u32 val)
{
	val <<= CR_NP_SCU_SSTR_SERDES_WIFI1_SEL_OFFSET;
	set_npScuReg_bits(CR_NP_SCU_SSTR, CR_NP_SCU_SSTR_SERDES_WIFI1_SEL_MASK, val);
}
EXPORT_SYMBOL(SET_NP_SCU_SERDES_WIFI1_SEL);
#endif

u32 GET_SCU_RSTCTRL1(void)
{
    return get_np_scu_data(CR_NP_SCU_RSTCTRL1);
}
EXPORT_SYMBOL(GET_SCU_RSTCTRL1);

u32 GET_NP_SCU_EMMC(void)
{
	return ((get_np_scu_data(CR_NP_SCU_BOOT_TRAP_CONF_DEC) & BOOT_TRAP_CONF_DEC_EMMC_MASK) >> BOOT_TRAP_CONF_DEC_EMMC_OFFSET);
}
EXPORT_SYMBOL(GET_NP_SCU_EMMC);


void SET_SCU_RSTCTRL1(u32 val)
{
    set_np_scu_data(CR_NP_SCU_RSTCTRL1, val);
    return;
}
EXPORT_SYMBOL(SET_SCU_RSTCTRL1);

u32 GET_SCU_RST_RG(void)
{
    return get_np_scu_data(CR_NP_SCU_RST_CFG);
}
EXPORT_SYMBOL(GET_SCU_RST_RG);

void SET_SCU_RST_RG(u32 val)
{
    set_np_scu_data(CR_NP_SCU_RST_CFG, val);
    return;
}
EXPORT_SYMBOL(SET_SCU_RST_RG);

int RW_SCREG_REG_TEST(u32 wVal)
{
    /* write reg */
    set_np_scu_data(CR_NP_SCU_SCREG_WR0, wVal);
    set_np_scu_data(CR_NP_SCU_SCREG_WR1, wVal);
    
    /* read reg and compare*/
    if ((get_np_scu_data(CR_NP_SCU_SCREG_WR0) != wVal) || (get_np_scu_data(CR_NP_SCU_SCREG_WR1) != wVal)) {
        printk("\nERROR1 SCREG_WR0:0x%x, SCREG_WR1:0x%x at wVal:0x%x\n", get_np_scu_data(CR_NP_SCU_SCREG_WR0), get_np_scu_data(CR_NP_SCU_SCREG_WR1), wVal);
        return -1;
    }
    return 0;
}
EXPORT_SYMBOL(RW_SCREG_REG_TEST);

u32 GET_SCU_RST_ACC_CHK_BMA(void)
{
    return get_np_scu_data(CR_NP_SCU_RST_ACC_CHK_BMAP);
}
EXPORT_SYMBOL(GET_SCU_RST_ACC_CHK_BMA);

void SET_SCU_RST_ACC_CHK_BMA(u32 val)
{
    set_np_scu_data(CR_NP_SCU_RST_ACC_CHK_BMAP, val);
    return;
}
EXPORT_SYMBOL(SET_SCU_RST_ACC_CHK_BMA);


u32 GET_SHARE_UNZIPMEM_SEL(void)
{
    return get_np_scu_data(CR_NP_SCU_SHARE_UNZIPMEM_SEL);
}
EXPORT_SYMBOL(GET_SHARE_UNZIPMEM_SEL);

void SET_SHARE_UNZIPMEM_SEL(u32 val)
{
    set_np_scu_data(CR_NP_SCU_SHARE_UNZIPMEM_SEL, val);
    return;
}
EXPORT_SYMBOL(SET_SHARE_UNZIPMEM_SEL);

u32 GET_PCM_NP_SCU(u32 reg)
{
	return get_np_scu_data(reg);
}
EXPORT_SYMBOL(GET_PCM_NP_SCU);

void SET_PCM_NP_SCU(u32 reg, u32 val)
{
	set_np_scu_data(reg, val); 
}
EXPORT_SYMBOL(SET_PCM_NP_SCU);

unsigned int get_l2c_sram_size(void)
{
    return l2c_sram_size;
}
EXPORT_SYMBOL(get_l2c_sram_size);

void SET_SHARE_FEMEM_SEL(u32 val)
{
    set_np_scu_data(CR_NP_SCU_SHARE_FEMEM_SEL, val);
    return;
}
EXPORT_SYMBOL(SET_SHARE_FEMEM_SEL);

/**********************************************************************
 ****  C H I P    S C U    a c c e s s ********************************
 *********************************************************************/
/* don't EXPORT this function. Create API for your purpose instead. */
u32 get_chip_scu_data(u32 reg)
{
	return readl(ecnt_scu->chipScu_base + reg);
}

/* don't EXPORT this function. Create API for your purpose instead. */
void set_chip_scu_data(u32 reg, u32 val)
{
	writel(val, ecnt_scu->chipScu_base + reg); 
}

u32 GET_IOMUX_CTRL_2(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_2);
}
EXPORT_SYMBOL(GET_IOMUX_CTRL_2);

void SET_IOMUX_CTRL_2(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_2, val);
}

u32 GET_IOMUX_CTRL_3(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_3);
}
EXPORT_SYMBOL(GET_IOMUX_CTRL_3);


void SET_IOMUX_CTRL_3(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_3, val);
}
EXPORT_SYMBOL(SET_IOMUX_CTRL_3);

void SET_IOMUX_CTRL_BIT(u32 iomux, u32 val)
{
    u32 regval, ctladr;
    u32 ctrno = (iomux >> 16)-1;

    if (ctrno < CR_CHIP_SCU_IOMUX_NUM) {
        ctladr = CHIP_SCU_IOMUX_REG[ctrno];
    }else {
        printk("No this iomux number.\n");
        printk("Bit[16] = IOMUX_Control No.(Start form 1) Bit[0:1] = Bit No.");
        return;
    }

    regval = get_chip_scu_data(ctladr);

    if (val) {
        regval |= (0x1 << (iomux & 0xff));
    }else {
        regval &= ~(0x1 << (iomux & 0xff));
    }
    set_chip_scu_data(ctladr, regval);
}
EXPORT_SYMBOL(SET_IOMUX_CTRL_BIT);


u32 GET_SPI_CLK(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_SPI_CLK);
}

void SET_SPI_CLK(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_SPI_CLK, val);
}

u32 GET_SPI_QUAD_SHARED_PIN_RG(void)
{
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	return get_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_2);
#elif defined(TCSUPPORT_CPU_EN7516)||defined(TCSUPPORT_CPU_EN7527)
	return get_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_1);
#else
	return get_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_1);
#endif
}
EXPORT_SYMBOL(GET_SPI_QUAD_SHARED_PIN_RG);

void SET_SPI_QUAD_SHARED_PIN_RG(u32 val)
{
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	set_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_2, val);
#elif defined(TCSUPPORT_CPU_EN7516)||defined(TCSUPPORT_CPU_EN7527)
	set_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_1, val);
#else
	set_chip_scu_data(CR_CHIP_SCU_IOMUX_CTRL_1, val);
#endif
}
EXPORT_SYMBOL(SET_SPI_QUAD_SHARED_PIN_RG);

u32 GET_CLK_PER_ICG_ENABLE(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1);
}
EXPORT_SYMBOL(GET_CLK_PER_ICG_ENABLE);

void SET_CLK_PER_ICG_ENABLE(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1, val);
}
EXPORT_SYMBOL(SET_CLK_PER_ICG_ENABLE);

u32 GET_CLK_PER_ICG_ENABLE1(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_2);
}
EXPORT_SYMBOL(GET_CLK_PER_ICG_ENABLE1);

void SET_CLK_PER_ICG_ENABLE1(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_2, val);
}
EXPORT_SYMBOL(SET_CLK_PER_ICG_ENABLE1);


#ifndef TCSUPPORT_CPU_EN7523
u32 GET_TOD_DIVIDER_ENABLE(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_TOD_DIVIDER_ENABLE);
}
EXPORT_SYMBOL(GET_TOD_DIVIDER_ENABLE);

void SET_TOD_DIVIDER_ENABLE(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_TOD_DIVIDER_ENABLE, val);
}
EXPORT_SYMBOL(SET_TOD_DIVIDER_ENABLE);
#endif

/* module==1 for TRNG
 * module==2 for I2C_Slave */
void scu_Enable_Module_Clock (u8 module)
{
	if (module == 1) /* TRNG module */
	{
		set_chip_scu_data(CR_CHIP_SCU_NP_BUS_DOM_CLK_GAT, (get_chip_scu_data(CR_CHIP_SCU_NP_BUS_DOM_CLK_GAT) | (1 << 3)));
		set_chip_scu_data(CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1, (get_chip_scu_data(CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1) | (1 << 21)));
		set_chip_scu_data(CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_2, (get_chip_scu_data(CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_2) | (1 << 0)));
	}
	else if (module == 2) /* I2C_Slave module */
	{
		set_chip_scu_data(CR_CHIP_SCU_NP_BUS_DOM_CLK_GAT, (get_chip_scu_data(CR_CHIP_SCU_NP_BUS_DOM_CLK_GAT) | (1 << 18)));
	}
	else
	{
		printk("scu_Enable_Module_Clock() uses wrong module index:%d\n", module);
	}
}

void set_chipScuReg_bits(u32 reg, u32 mask, u32 bits)
{
    u32 val = get_chip_scu_data(reg);

    val &= (~mask);
    val |= bits;

    set_chip_scu_data(reg, val);
}

void SET_PON_I2C_MODE(u32 mask, u32 bits)
{
    set_chipScuReg_bits(CR_CHIP_SCU_PON_I2C_MODE, mask, bits);
}
EXPORT_SYMBOL(SET_PON_I2C_MODE);

u32 GET_PON_I2C_MODE(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_PON_I2C_MODE);
}
EXPORT_SYMBOL(GET_PON_I2C_MODE);

void SET_GPIO2_EN(u32 mask, u32 bits)
{
    set_chipScuReg_bits(RG_FORCE_GPIO2_EN, mask, bits);
}
EXPORT_SYMBOL(SET_GPIO2_EN);

u32 GET_GPIO2_EN(void)
{
    return get_chip_scu_data(RG_FORCE_GPIO2_EN);
}
EXPORT_SYMBOL(GET_GPIO2_EN);

void SET_FORCE_GPIO32_EN(u32 mask, u32 bits)
{
    set_chipScuReg_bits(CR_CHIP_SCU_FORCE_GPIO32_EN, mask, bits);
}
EXPORT_SYMBOL(SET_FORCE_GPIO32_EN);

u32 GET_SCU_RST_CTRL_1(void)
{
	return get_np_scu_data(CR_NP_SCU_RSTCTRL1);
}
EXPORT_SYMBOL(GET_SCU_RST_CTRL_1);

void SET_SCU_RST_CTRL_1(u32 val)
{
	set_np_scu_data(CR_NP_SCU_RSTCTRL1, val);
}
EXPORT_SYMBOL(SET_SCU_RST_CTRL_1);

u32 GET_SCU_CLK_CFG(void)
{
	return get_chip_scu_data(CR_NP_SCU_CLK_CFG);
}
EXPORT_SYMBOL(GET_SCU_CLK_CFG);

void SET_SCU_CLK_CFG(u32 val)
{
	set_chip_scu_data(CR_NP_SCU_CLK_CFG, val);
}
EXPORT_SYMBOL(SET_SCU_CLK_CFG);

u32 GET_PCM_CHIP_SCU(u32 reg)
{
	return get_chip_scu_data(reg);
}
EXPORT_SYMBOL(GET_PCM_CHIP_SCU);


void SET_PCM_CHIP_SCU(u32 reg, u32 val)
{
	set_chip_scu_data(reg, val); 
}
EXPORT_SYMBOL(SET_PCM_CHIP_SCU);

u32 GET_RGS_XTAL_FREQ(void)
{
	return ((get_chip_scu_data(CR_CHIP_SCU_RGS_ECC_SEL) >> 19) & 0x1);
}
EXPORT_SYMBOL(GET_RGS_XTAL_FREQ);


u32 GET_SCU_RGS_OPEN_DRAIN(void)
{
	return ((get_chip_scu_data(CR_CHIP_SCU_RGS_OPEN_DRAIN)) & OPEN_DRAIN_MASK);
}
EXPORT_SYMBOL(GET_SCU_RGS_OPEN_DRAIN);

void SET_SCU_RGS_OPEN_DRAIN(u32 val)
{
    set_chipScuReg_bits(CR_CHIP_SCU_RGS_OPEN_DRAIN, OPEN_DRAIN_MASK, val & OPEN_DRAIN_MASK);
	
}
EXPORT_SYMBOL(SET_SCU_RGS_OPEN_DRAIN);

u32 GET_SCU_RGS_CLK_GSW(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_RGS_CLK_GSW);
}
EXPORT_SYMBOL(GET_SCU_RGS_CLK_GSW);

void SET_SCU_RGS_CLK_GSW(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_RGS_CLK_GSW, val);
}
EXPORT_SYMBOL(SET_SCU_RGS_CLK_GSW);

u32 GET_SCU_RGS_CLK_EMI(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_RGS_CLK_EMI);
}
EXPORT_SYMBOL(GET_SCU_RGS_CLK_EMI);

void SET_SCU_RGS_CLK_EMI(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_RGS_CLK_EMI, val);
}
EXPORT_SYMBOL(SET_SCU_RGS_CLK_EMI);

u32 GET_SCU_RGS_CLK_BUS(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_RGS_CLK_BUS);
}
EXPORT_SYMBOL(GET_SCU_RGS_CLK_BUS);

void SET_SCU_RGS_CLK_BUS(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_RGS_CLK_BUS, val);
}
EXPORT_SYMBOL(SET_SCU_RGS_CLK_BUS);

u32 GET_SCU_RGS_CLK_FE(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_RGS_CLK_FE);
}
EXPORT_SYMBOL(GET_SCU_RGS_CLK_FE);

void SET_SCU_RGS_CLK_FE(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_RGS_CLK_FE, val);
}
EXPORT_SYMBOL(SET_SCU_RGS_CLK_FE);

u32 GET_SCU_RGS_CLK_NPU(void)
{
	return get_chip_scu_data(CR_CHIP_SCU_RGS_CLK_NPU);
}
EXPORT_SYMBOL(GET_SCU_RGS_CLK_NPU);

void SET_SCU_RGS_CLK_NPU(u32 val)
{
	set_chip_scu_data(CR_CHIP_SCU_RGS_CLK_NPU, val);
}
EXPORT_SYMBOL(SET_SCU_RGS_CLK_NPU);

#ifdef TCSUPPORT_CPU_EN7581
u32 GET_ECO_HWFIX(void)
{
	u32 val = 0;
	val = get_chip_scu_data(CR_CHIP_SCU_TOP_REV_24);
	if ((val & 0x1) == 0)
		return 0;
	else
		return 1;
}
EXPORT_SYMBOL(GET_ECO_HWFIX);
#endif

/* don't EXPORT this function. Create API for your purpose instead. */
static u32 get_rbus_clk_data(u32 reg)
{
    return readl(ecnt_scu->clk_ctl_base + reg);
}
/* don't EXPORT this function. Create API for your purpose instead. */
static void set_rbus_clk_data(u32 reg, u32 val)
{
	writel(val, ecnt_scu->clk_ctl_base + reg);
}

u32 get_rbus_clk(void)
{
	return get_rbus_clk_data(CR_RBUS_CLK_CTL);
}

EXPORT_SYMBOL(get_rbus_clk);

void set_rbus_clk(u32 val)
{

	set_rbus_clk_data(CR_RBUS_CLK_CTL, val);

}

EXPORT_SYMBOL(set_rbus_clk);

/* 
	disable global interrupt method is different between aarch32 & aarch64
	In aarch32, use cpsr register to enable/disable IRQ/FIQ
	In aarch64, use daif register to enable/disable IRQ
*/
int disable_interrupts(void)
{
#ifdef TCSUPPORT_CPU_ARMV8_64
	asm volatile("msr daifset, #2");
	return 0;
#else
	unsigned long old, temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
						 "orr %1, %0, #0xc0\n"
						 "msr cpsr_c, %1"
						 : "=r" (old), "=r" (temp)
						 :
						 : "memory");
	return (old & 0x80) == 0;
#endif
}

EXPORT_SYMBOL(disable_interrupts);

#ifdef TCSUPPORT_DYING_GASP
static void dyinggasp_handler_init(void)
{
	CB_DGASP_LIST *node;
	if ( cb_dgasp_list_head != NULL)
	{
		printk("dgasp list is not NULL! \n");
		return ;
	}
	node = (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST),GFP_KERNEL);
	memset(node, 0, sizeof(CB_DGASP_LIST));
	INIT_LIST_HEAD(&node->list);
	cb_dgasp_list_head = node;
	printk("dying gasp handler list init !\n");
}

void dying_gasp_handler_register(char *name, void *fn, void *context)
{
	CB_DGASP_LIST *node;

	node = (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST),GFP_KERNEL);
	memset(node, 0, sizeof(CB_DGASP_LIST));
	INIT_LIST_HEAD(&node->list);
	strncpy(node->name, name, 9);
	node->context = context;
	node->cb_dgasp_fn = (cb_dyinggasp_t)fn;

	mutex_lock(&dying_gasp_lock);
	if (cb_dgasp_list_head == NULL)
	{
		kfree(node);
	}
	else
	{
		list_add(&node->list, &cb_dgasp_list_head->list);
		printk("dying gasp handler %s registered!\n", name);
	}
	mutex_unlock(&dying_gasp_lock);
	
}

void dying_gasp_handler_exec(void)
{
	struct list_head *pos;
	CB_DGASP_LIST *tmp = NULL;

	list_for_each(pos, &cb_dgasp_list_head->list)
	{
		
		tmp = list_entry(pos,CB_DGASP_LIST, list);
		(tmp->cb_dgasp_fn)(tmp->context);
	}
}

EXPORT_SYMBOL(dying_gasp_handler_exec);

EXPORT_SYMBOL(dying_gasp_handler_register);
#endif

void set_npu_clk_power_save(u32 reg, u32 bit, u32 val, char en)
{
    u32 tmpVal = 0;
    reg = CR_CHIP_SCU_BASE + reg;
    tmpVal= get_chip_scu_data(reg);

    if(en == 1)
    {
        tmpVal |= (val<<bit);
    }
    else
    {
        tmpVal &= ~(val<<bit);
    }

    set_chip_scu_data(reg, tmpVal);
    return;
	
}
EXPORT_SYMBOL(set_npu_clk_power_save);

u32 get_pbus_err_addr(void)
{
	return get_np_scu_data(CR_NP_SCU_PB_TO_ERR);
}
EXPORT_SYMBOL(get_pbus_err_addr);

int isPbusTimeoutEn(void)
{
    return ((get_np_scu_data(CR_NP_SCU_MON_TMR)>>30)&0x1);
}
EXPORT_SYMBOL(isPbusTimeoutEn);


u32 GET_SCREG_WF0(void)
{
    return get_np_scu_data(CR_NP_SCU_SCREG_WF0);
}
EXPORT_SYMBOL(GET_SCREG_WF0);

void SET_SCREG_WF0(u32 val)
{
    set_np_scu_data(CR_NP_SCU_SCREG_WF0, val);
    return;
}
EXPORT_SYMBOL(SET_SCREG_WF0);

static void pbus_timeout_init(void)
{
	if (GET_IS_FPGA()) 
	{
		set_np_scu_data(CR_NP_SCU_MON_TMR, FPGA_PBUS_CLK); /* unit: one bus clock */
		set_np_scu_data(CR_NP_SCU_MON_TMR, FPGA_PBUS_CLK|PBUS_TOUT_EN); /* enable it */
	}else 
	{
		set_np_scu_data(CR_NP_SCU_MON_TMR, ASIC_PBUS_CLK); 
		set_np_scu_data(CR_NP_SCU_MON_TMR, ASIC_PBUS_CLK|PBUS_TOUT_EN);
	}
    return;
}

static irqreturn_t bus_timeout_interrupt(int irq, void *dev_id)
{
	unsigned long addr;
	u32 isBusTout=0;

	/* read to clear interrupt */
	isBusTout = (get_np_scu_data(CR_NP_SCU_PRATIR)&0x1);
	set_np_scu_data(CR_NP_SCU_PRATIR, 1);

	addr =  get_np_scu_data(CR_NP_SCU_PB_TO_ERR);
	addr &= ~((1 << 30) | (1 << 31));
	if (isBusTout)
		printk("pbus timeout interrupt ERR ADDR=%08lx\n", addr);
	else
		printk("unknown bus timeout interrupt ERR ADDR=%08lx\n", addr);
	
	dump_stack();	
	
	return IRQ_HANDLED;
}

int ECNT_SCU_DRV_PROBE(void)
{
	struct resource *res = NULL;
    struct device_node *node=NULL;
    struct platform_device *pdev=NULL;

	int ret = 0;

    node = of_find_node_by_path("/scu@1fb00000");
    if (node==NULL) {
        printk("\nERROR(%s) node==NULL\n", __func__);
        return -1;
    }

    pdev = of_find_device_by_node(node);
    if (pdev==NULL) {
        printk("\nERROR(%s) pdev==NULL\n", __func__);
        return -1;
    }

	ecnt_scu = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_scu), GFP_KERNEL);
	if (!ecnt_scu)
		return -ENOMEM;
	platform_set_drvdata(pdev, ecnt_scu);

	/* Get NP SCU address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ecnt_scu->npScu_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ecnt_scu->npScu_base)) {
        printk("\nERROR(%s) npScu_base\n", __func__);
		return -1;
    }
    
	/* Get CHIP SCU address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	ecnt_scu->chipScu_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ecnt_scu->chipScu_base)) {
        printk("\nERROR(%s) chipScu_base\n", __func__);
		return -1;
    }

	/* Get Clk ctl address (For FPGA only)*/
    res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    ecnt_scu->clk_ctl_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(ecnt_scu->clk_ctl_base))
        return PTR_ERR(ecnt_scu->clk_ctl_base);

	ecnt_scu->dev = &pdev->dev;
	
	/* CHIP SCU Pbus Timeout Interrupt Setting */
	ecnt_scu->irq = platform_get_irq(pdev, 0);
	if(ecnt_scu->irq <= 0) {
		printk("\n get pbus timeout irq number failed\n");
		return ecnt_scu->irq;
	}
	pbus_timeout_init();
	ret = request_irq(ecnt_scu->irq, bus_timeout_interrupt, 0, "ScuPbusTout", ecnt_scu->dev);
	if(ret) {
		printk("\n request_irq() (irq number: %d) failed (ret: %d)\n", ecnt_scu->irq, ret);
		return (ret);
	}

#ifdef TCSUPPORT_DYING_GASP
	dyinggasp_handler_init();
#endif

    l2c_sram_size = get_np_scu_data(CR_NP_SCU_SCREG_WR0);
    printk("\nl2c_sram_size:0x%x\n", l2c_sram_size);
#if defined(TCSUPPORT_CPU_EN7523)
	scu_Enable_Module_Clock(1);
#endif

	return 0;
}

/* init SCU registers' base address (a.s.a.p.) before any kernel module might access it. 
 * For example, usb_init() calls "isFPGA" which will access NP SCU register. 
 * If SCU base address has not initialized before that, cpu will crash. 
 * usb_init() uses subsys_initcall to init. Although ECNT_SCU_DRV_PROBE also uses
 * the smae subsys_initcall, it's executed before usb_init(), so it's ok 
 * Note: you can check linux-4.4.115/System.map to see which initcall function will be executed first*/
subsys_initcall(ECNT_SCU_DRV_PROBE);

#if 0
static int ecnt_scu_drv_probe(struct platform_device *pdev)
{
	struct resource *res = NULL;
	int ret = 0;
    
	if (!pdev->dev.of_node) {
		dev_err(&pdev->dev, "No DT found\n");
		return -EINVAL;
	}

	ecnt_scu = devm_kzalloc(&pdev->dev, sizeof(struct ecnt_scu), GFP_KERNEL);
	if (!ecnt_scu)
		return -ENOMEM;
	platform_set_drvdata(pdev, ecnt_scu);

	/* Get NP SCU address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	ecnt_scu->npScu_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ecnt_scu->npScu_base))
		return PTR_ERR(ecnt_scu->npScu_base);
    
	/* Get CHIP SCU address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	ecnt_scu->chipScu_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ecnt_scu->chipScu_base))
		return PTR_ERR(ecnt_scu->chipScu_base);

	ecnt_scu->dev = &pdev->dev;

	return ret;
}

static int ecnt_scu_drv_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver ecnt_scu_driver = {
	.probe = ecnt_scu_drv_probe,
	.remove = ecnt_scu_drv_remove,
	.driver = {
		.name = "ecnt-scu",
		//.pm = MTK_NOR_DEV_PM_OPS,
		.of_match_table = ecnt_scu_of_ids,
	},
};

module_platform_driver(ecnt_scu_driver);
//MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("EcoNet SCU Driver");
#endif