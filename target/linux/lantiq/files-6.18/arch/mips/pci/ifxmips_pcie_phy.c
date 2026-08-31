/******************************************************************************
**
** FILE NAME    : ifxmips_pcie_phy.c
** PROJECT      : IFX UEIP for VRX200
** MODULES      : PCIe PHY sub module
**
** DATE         : 14 May 2009
** AUTHOR       : Lei Chuanhua
** DESCRIPTION  : PCIe Root Complex Driver
** COPYRIGHT    :       Copyright (c) 2009
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
** HISTORY
** $Version $Date        $Author         $Comment
** 0.0.1    14 May,2009  Lei Chuanhua    Initial version
*******************************************************************************/
/*!
 \file ifxmips_pcie_phy.c
 \ingroup IFX_PCIE
 \brief PCIe PHY PLL register programming source file
*/
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/paccess.h>
#include <linux/delay.h>

#include "ifxmips_pcie_reg.h"
#include "ifxmips_pcie.h"

/* PCIe PDI only supports 16 bit operation */

#define IFX_PCIE_PHY_REG_WRITE16(__addr, __data) \
    ((*(volatile u16 *) (__addr)) = (__data))

#define IFX_PCIE_PHY_REG_READ16(__addr)  \
    (*(volatile u16 *) (__addr))

#define IFX_PCIE_PHY_REG16(__addr)   \
    (*(volatile u16 *) (__addr))

#define IFX_PCIE_PHY_REG(__reg, __value, __mask) do { \
    u16 read_data;                                    \
    u16 write_data;                                   \
    read_data = IFX_PCIE_PHY_REG_READ16((__reg));      \
    write_data = (read_data & ((u16)~(__mask))) | (((u16)(__value)) & ((u16)(__mask)));\
    IFX_PCIE_PHY_REG_WRITE16((__reg), write_data);               \
} while (0)

#define IFX_PCIE_PLL_TIMEOUT 1000 /* Tunnable */

//#define IFX_PCI_PHY_REG_DUMP

#ifdef IFX_PCI_PHY_REG_DUMP
static void
pcie_phy_reg_dump(int pcie_port)
{
    printk("PLL REGFILE\n");
    printk("PCIE_PHY_PLL_CTRL1    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_CTRL1(pcie_port)));
    printk("PCIE_PHY_PLL_CTRL2    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_CTRL2(pcie_port)));
    printk("PCIE_PHY_PLL_CTRL3    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_CTRL3(pcie_port)));
    printk("PCIE_PHY_PLL_CTRL4    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_CTRL4(pcie_port)));
    printk("PCIE_PHY_PLL_CTRL5    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_CTRL5(pcie_port)));
    printk("PCIE_PHY_PLL_CTRL6    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_CTRL6(pcie_port)));
    printk("PCIE_PHY_PLL_CTRL7    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_CTRL7(pcie_port)));
    printk("PCIE_PHY_PLL_A_CTRL1  0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_A_CTRL1(pcie_port)));
    printk("PCIE_PHY_PLL_A_CTRL2  0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_A_CTRL2(pcie_port)));
    printk("PCIE_PHY_PLL_A_CTRL3  0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_A_CTRL3(pcie_port)));
    printk("PCIE_PHY_PLL_STATUS   0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_STATUS(pcie_port)));

    printk("TX1 REGFILE\n");
    printk("PCIE_PHY_TX1_CTRL1    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX1_CTRL1(pcie_port)));
    printk("PCIE_PHY_TX1_CTRL2    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX1_CTRL2(pcie_port)));
    printk("PCIE_PHY_TX1_CTRL3    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX1_CTRL3(pcie_port)));
    printk("PCIE_PHY_TX1_A_CTRL1  0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX1_A_CTRL1(pcie_port)));
    printk("PCIE_PHY_TX1_A_CTRL2  0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX1_A_CTRL2(pcie_port)));
    printk("PCIE_PHY_TX1_MOD1     0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX1_MOD1(pcie_port)));
    printk("PCIE_PHY_TX1_MOD2     0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX1_MOD2(pcie_port)));
    printk("PCIE_PHY_TX1_MOD3     0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX1_MOD3(pcie_port)));

    printk("TX2 REGFILE\n");
    printk("PCIE_PHY_TX2_CTRL1    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX2_CTRL1(pcie_port)));
    printk("PCIE_PHY_TX2_CTRL2    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX2_CTRL2(pcie_port)));
    printk("PCIE_PHY_TX2_A_CTRL1  0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX2_A_CTRL1(pcie_port)));
    printk("PCIE_PHY_TX2_A_CTRL2  0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX2_A_CTRL2(pcie_port)));
    printk("PCIE_PHY_TX2_MOD1     0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX2_MOD1(pcie_port)));
    printk("PCIE_PHY_TX2_MOD2     0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX2_MOD2(pcie_port)));
    printk("PCIE_PHY_TX2_MOD3     0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_TX2_MOD3(pcie_port)));

    printk("RX1 REGFILE\n");
    printk("PCIE_PHY_RX1_CTRL1    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_RX1_CTRL1(pcie_port)));
    printk("PCIE_PHY_RX1_CTRL2    0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_RX1_CTRL2(pcie_port)));
    printk("PCIE_PHY_RX1_CDR      0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_RX1_CDR(pcie_port)));
    printk("PCIE_PHY_RX1_EI       0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_RX1_EI(pcie_port)));
    printk("PCIE_PHY_RX1_A_CTRL   0x%04x\n", IFX_PCIE_PHY_REG16(PCIE_PHY_RX1_A_CTRL(pcie_port)));
}
#endif /* IFX_PCI_PHY_REG_DUMP */

static void
pcie_phy_comm_setup(int pcie_port)
{
   /* PLL Setting */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL1(pcie_port), 0x120e, 0xFFFF);

    /* increase the bias reference voltage */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL2(pcie_port), 0x39D7, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL3(pcie_port), 0x0900, 0xFFFF);

    /* Endcnt */
    IFX_PCIE_PHY_REG(PCIE_PHY_RX1_EI(pcie_port), 0x0004, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_RX1_A_CTRL(pcie_port), 0x6803, 0xFFFF);

    /* force */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_CTRL1(pcie_port), 0x0008, 0x0008);

    /* predrv_ser_en */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_A_CTRL2(pcie_port), 0x0706, 0xFFFF);

    /* ctrl_lim */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_CTRL3(pcie_port), 0x1FFF, 0xFFFF);

    /* ctrl */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_A_CTRL1(pcie_port), 0x0800, 0xFF00);

    /* predrv_ser_en */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_A_CTRL2(pcie_port), 0x4702, 0x7F00);

    /* RTERM*/
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_CTRL2(pcie_port), 0x2e00, 0xFFFF);

    /* Improved 100MHz clock output  */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_CTRL2(pcie_port), 0x3096, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_A_CTRL2(pcie_port), 0x4707, 0xFFFF);

    /* Reduced CDR BW to avoid glitches */
    IFX_PCIE_PHY_REG(PCIE_PHY_RX1_CDR(pcie_port), 0x0235, 0xFFFF);
}

#ifdef CONFIG_IFX_PCIE_PHY_36MHZ_MODE
static void
pcie_phy_36mhz_mode_setup(int pcie_port)
{
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "%s pcie_port %d enter\n", __func__, pcie_port);
#ifdef IFX_PCI_PHY_REG_DUMP
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "Initial PHY register dump\n");
    pcie_phy_reg_dump(pcie_port);
#endif

    /* en_ext_mmd_div_ratio */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL3(pcie_port), 0x0000, 0x0002);

    /* ext_mmd_div_ratio*/
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL3(pcie_port), 0x0000, 0x0070);

    /* pll_ensdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0200, 0x0200);

    /* en_const_sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0100, 0x0100);

    /* mmd */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL3(pcie_port), 0x2000, 0xe000);

    /* lf_mode */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL2(pcie_port), 0x0000, 0x4000);

    /* const_sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL1(pcie_port), 0x38e4, 0xFFFF);

    /* const sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x00ee, 0x00FF);

    /* pllmod */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL7(pcie_port), 0x0002, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL6(pcie_port), 0x3a04, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL5(pcie_port), 0xfae3, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL4(pcie_port), 0x1b72, 0xFFFF);

    IFX_PCIE_PRINT(PCIE_MSG_PHY, "%s pcie_port %d exit\n", __func__, pcie_port);
}
#endif /* CONFIG_IFX_PCIE_PHY_36MHZ_MODE */

#ifdef CONFIG_IFX_PCIE_PHY_36MHZ_SSC_MODE
static void
pcie_phy_36mhz_ssc_mode_setup(int pcie_port)
{
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "%s pcie_port %d enter\n", __func__, pcie_port);
#ifdef IFX_PCI_PHY_REG_DUMP
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "Initial PHY register dump\n");
    pcie_phy_reg_dump(pcie_port);
#endif

    /* PLL Setting */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL1(pcie_port), 0x120e, 0xFFFF);

    /* Increase the bias reference voltage */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL2(pcie_port), 0x39D7, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL3(pcie_port), 0x0900, 0xFFFF);

    /* Endcnt */
    IFX_PCIE_PHY_REG(PCIE_PHY_RX1_EI(pcie_port), 0x0004, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_RX1_A_CTRL(pcie_port), 0x6803, 0xFFFF);

    /* Force */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_CTRL1(pcie_port), 0x0008, 0x0008);

    /* Predrv_ser_en */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_A_CTRL2(pcie_port), 0x0706, 0xFFFF);

    /* ctrl_lim */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_CTRL3(pcie_port), 0x1FFF, 0xFFFF);

    /* ctrl */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_A_CTRL1(pcie_port), 0x0800, 0xFF00);

    /* predrv_ser_en */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_A_CTRL2(pcie_port), 0x4702, 0x7F00);

    /* RTERM*/
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_CTRL2(pcie_port), 0x2e00, 0xFFFF);

    /* en_ext_mmd_div_ratio */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL3(pcie_port), 0x0000, 0x0002);

    /* ext_mmd_div_ratio*/
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL3(pcie_port), 0x0000, 0x0070);

    /* pll_ensdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0400, 0x0400);

    /* en_const_sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0200, 0x0200);

    /* mmd */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL3(pcie_port), 0x2000, 0xe000);

    /* lf_mode */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL2(pcie_port), 0x0000, 0x4000);

    /* const_sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL1(pcie_port), 0x38e4, 0xFFFF);

    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0000, 0x0100);
    /* const sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x00ee, 0x00FF);

    /* pllmod */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL7(pcie_port), 0x0002, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL6(pcie_port), 0x3a04, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL5(pcie_port), 0xfae3, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL4(pcie_port), 0x1c72, 0xFFFF);

    /* improved 100MHz clock output  */
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_CTRL2(pcie_port), 0x3096, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_A_CTRL2(pcie_port), 0x4707, 0xFFFF);

    /* reduced CDR BW to avoid glitches */
    IFX_PCIE_PHY_REG(PCIE_PHY_RX1_CDR(pcie_port), 0x0235, 0xFFFF);

    IFX_PCIE_PRINT(PCIE_MSG_PHY, "%s pcie_port %d exit\n", __func__, pcie_port);
}
#endif /* CONFIG_IFX_PCIE_PHY_36MHZ_SSC_MODE */

#ifdef CONFIG_IFX_PCIE_PHY_25MHZ_MODE
static void
pcie_phy_25mhz_mode_setup(int pcie_port)
{
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "%s pcie_port %d enter\n", __func__, pcie_port);
#ifdef IFX_PCI_PHY_REG_DUMP
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "Initial PHY register dump\n");
    pcie_phy_reg_dump(pcie_port);
#endif
    /* en_const_sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0100, 0x0100);

    /* pll_ensdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0000, 0x0200);

    /* en_ext_mmd_div_ratio*/
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL3(pcie_port), 0x0002, 0x0002);

    /* ext_mmd_div_ratio*/
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL3(pcie_port), 0x0040, 0x0070);

    /* mmd */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL3(pcie_port), 0x6000, 0xe000);

    /* lf_mode */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL2(pcie_port), 0x4000, 0x4000);

    IFX_PCIE_PRINT(PCIE_MSG_PHY, "%s pcie_port %d exit\n", __func__, pcie_port);
}
#endif /* CONFIG_IFX_PCIE_PHY_25MHZ_MODE */

#ifdef CONFIG_IFX_PCIE_PHY_100MHZ_MODE
static void
pcie_phy_100mhz_mode_setup(int pcie_port)
{
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "%s pcie_port %d enter\n", __func__, pcie_port);
#ifdef IFX_PCI_PHY_REG_DUMP
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "Initial PHY register dump\n");
    pcie_phy_reg_dump(pcie_port);
#endif
    /* en_ext_mmd_div_ratio */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL3(pcie_port), 0x0000, 0x0002);

    /* ext_mmd_div_ratio*/
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL3(pcie_port), 0x0000, 0x0070);

    /* pll_ensdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0200, 0x0200);

    /* en_const_sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x0100, 0x0100);

    /* mmd */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL3(pcie_port), 0x2000, 0xe000);

    /* lf_mode */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_A_CTRL2(pcie_port), 0x0000, 0x4000);

    /* const_sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL1(pcie_port), 0x38e4, 0xFFFF);

    /* const sdm */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL2(pcie_port), 0x00ee, 0x00FF);

    /* pllmod */
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL7(pcie_port), 0x0002, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL6(pcie_port), 0x3a04, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL5(pcie_port), 0xfae3, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_PLL_CTRL4(pcie_port), 0x1b72, 0xFFFF);

    IFX_PCIE_PRINT(PCIE_MSG_PHY, "%s pcie_port %d exit\n", __func__, pcie_port);
}
#endif /* CONFIG_IFX_PCIE_PHY_100MHZ_MODE */

static int
pcie_phy_wait_startup_ready(int pcie_port)
{
    int i;

    for (i = 0; i < IFX_PCIE_PLL_TIMEOUT; i++) {
        if ((IFX_PCIE_PHY_REG16(PCIE_PHY_PLL_STATUS(pcie_port)) & 0x0040) != 0) {
            break;
        }
        udelay(10);
    }
    if (i >= IFX_PCIE_PLL_TIMEOUT) {
        printk(KERN_ERR "%s PLL Link timeout\n", __func__);
        return -1;
    }
    return 0;
}

static void
pcie_phy_load_enable(int pcie_port, int slice)
{
    /* Set the load_en of tx/rx slice to '1' */
    switch (slice) {
        case 1:
            IFX_PCIE_PHY_REG(PCIE_PHY_TX1_CTRL1(pcie_port), 0x0010, 0x0010);
            break;
        case 2:
            IFX_PCIE_PHY_REG(PCIE_PHY_TX2_CTRL1(pcie_port), 0x0010, 0x0010);
            break;
        case 3:
            IFX_PCIE_PHY_REG(PCIE_PHY_RX1_CTRL1(pcie_port), 0x0002, 0x0002);
            break;
    }
}

static void
pcie_phy_load_disable(int pcie_port, int slice)
{
    /* set the load_en of tx/rx slice to '0' */
    switch (slice) {
        case 1:
            IFX_PCIE_PHY_REG(PCIE_PHY_TX1_CTRL1(pcie_port), 0x0000, 0x0010);
            break;
        case 2:
            IFX_PCIE_PHY_REG(PCIE_PHY_TX2_CTRL1(pcie_port), 0x0000, 0x0010);
            break;
        case 3:
            IFX_PCIE_PHY_REG(PCIE_PHY_RX1_CTRL1(pcie_port), 0x0000, 0x0002);
            break;
    }
}

static void
pcie_phy_load_war(int pcie_port)
{
    int slice;

    for (slice = 1; slice < 4; slice++) {
        pcie_phy_load_enable(pcie_port, slice);
        udelay(1);
        pcie_phy_load_disable(pcie_port, slice);
    }
}

static void
pcie_phy_tx2_modulation(int pcie_port)
{
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_MOD1(pcie_port), 0x1FFE, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_MOD2(pcie_port), 0xFFFE, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_MOD3(pcie_port), 0x0601, 0xFFFF);
    mdelay(1);
    IFX_PCIE_PHY_REG(PCIE_PHY_TX2_MOD3(pcie_port), 0x0001, 0xFFFF);
}

static void
pcie_phy_tx1_modulation(int pcie_port)
{
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_MOD1(pcie_port), 0x1FFE, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_MOD2(pcie_port), 0xFFFE, 0xFFFF);
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_MOD3(pcie_port), 0x0601, 0xFFFF);
    mdelay(1);
    IFX_PCIE_PHY_REG(PCIE_PHY_TX1_MOD3(pcie_port), 0x0001, 0xFFFF);
}

static void
pcie_phy_tx_modulation_war(int pcie_port)
{
    int i;

#define PCIE_PHY_MODULATION_NUM 5
    for (i = 0; i < PCIE_PHY_MODULATION_NUM; i++) {
        pcie_phy_tx2_modulation(pcie_port);
        pcie_phy_tx1_modulation(pcie_port);
    }
#undef PCIE_PHY_MODULATION_NUM
}

void
pcie_phy_clock_mode_setup(int pcie_port)
{
    pcie_pdi_big_endian(pcie_port);

    /* Enable PDI to access PCIe PHY register */
    pcie_pdi_pmu_enable(pcie_port);

    /* Configure PLL and PHY clock */
    pcie_phy_comm_setup(pcie_port);

#ifdef CONFIG_IFX_PCIE_PHY_36MHZ_MODE
    pcie_phy_36mhz_mode_setup(pcie_port);
#elif defined(CONFIG_IFX_PCIE_PHY_36MHZ_SSC_MODE)
    pcie_phy_36mhz_ssc_mode_setup(pcie_port);
#elif defined(CONFIG_IFX_PCIE_PHY_25MHZ_MODE)
    pcie_phy_25mhz_mode_setup(pcie_port);
#elif defined (CONFIG_IFX_PCIE_PHY_100MHZ_MODE)
    pcie_phy_100mhz_mode_setup(pcie_port);
#else
    #error "PCIE PHY Clock Mode must be chosen first!!!!"
#endif /* CONFIG_IFX_PCIE_PHY_36MHZ_MODE */

    /* Enable PCIe PHY and make PLL setting take effect */
    pcie_phy_pmu_enable(pcie_port);

    /* Check if we are in startup_ready status */
    pcie_phy_wait_startup_ready(pcie_port);

    pcie_phy_load_war(pcie_port);

    /* Apply TX modulation workarounds */
    pcie_phy_tx_modulation_war(pcie_port);

#ifdef IFX_PCI_PHY_REG_DUMP
    IFX_PCIE_PRINT(PCIE_MSG_PHY, "Modified PHY register dump\n");
    pcie_phy_reg_dump(pcie_port);
#endif
}

