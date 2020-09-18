/******************************************************************************
**
** FILE NAME    : ifxmips_atm_amazon_se.c
** PROJECT      : UEIP
** MODULES      : ATM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : ATM driver common source file (core functions)
** COPYRIGHT    :       Copyright (c) 2006
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 07 JUL 2009  Xu Liang        Init Version
*******************************************************************************/



/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>
#include <asm/delay.h>

/*
 *  Chip Specific Head File
 */
#include "ifxmips_atm_core.h"
#include "ifxmips_atm_fw_amazon_se.h"

#include <lantiq_soc.h>


/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  EMA Settings
 */
#define EMA_CMD_BUF_LEN      0x0040
#define EMA_CMD_BASE_ADDR    (0x00001580 << 2)
#define EMA_DATA_BUF_LEN     0x0100
#define EMA_DATA_BASE_ADDR   (0x00000B00 << 2)
#define EMA_WRITE_BURST      0x2
#define EMA_READ_BURST       0x2



/*
 * ####################################
 *             Declaration
 * ####################################
 */

/*
 *  Hardware Init/Uninit Functions
 */
static inline void init_pmu(void);
static inline void uninit_pmu(void);
static inline void reset_ppe(struct platform_device *pdev);
static inline void init_ema(void);
static inline void init_mailbox(void);
static inline void init_atm_tc(void);
static inline void clear_share_buffer(void);



/*
 * ####################################
 *            Local Variable
 * ####################################
 */



/*
 * ####################################
 *            Local Function
 * ####################################
 */
#define IFX_PMU_MODULE_PPE_SLL01  BIT(19)
#define IFX_PMU_MODULE_PPE_TC     BIT(21)
#define IFX_PMU_MODULE_PPE_EMA    BIT(22)
#define IFX_PMU_MODULE_PPE_QSB    BIT(18)
#define IFX_PMU_MODULE_TPE       BIT(13)
#define IFX_PMU_MODULE_DSL_DFE    BIT(9)

static inline void init_pmu(void)
{
    //*(unsigned long *)0xBF10201C &= ~((1 << 15) | (1 << 13) | (1 << 9));
    //PPE_TOP_PMU_SETUP(IFX_PMU_ENABLE);
/*    PPE_SLL01_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_TC_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_EMA_PMU_SETUP(IFX_PMU_ENABLE);
    //PPE_QSB_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_TPE_PMU_SETUP(IFX_PMU_ENABLE);
    DSL_DFE_PMU_SETUP(IFX_PMU_ENABLE);*/
	ltq_pmu_enable(IFX_PMU_MODULE_PPE_SLL01 |
		IFX_PMU_MODULE_PPE_TC |
		IFX_PMU_MODULE_PPE_EMA |
		IFX_PMU_MODULE_TPE |
		IFX_PMU_MODULE_DSL_DFE);
}

static inline void uninit_pmu(void)
{
    /*PPE_SLL01_PMU_SETUP(IFX_PMU_DISABLE);
    PPE_TC_PMU_SETUP(IFX_PMU_DISABLE);
    PPE_EMA_PMU_SETUP(IFX_PMU_DISABLE);
    //PPE_QSB_PMU_SETUP(IFX_PMU_DISABLE);
    PPE_TPE_PMU_SETUP(IFX_PMU_DISABLE);
    DSL_DFE_PMU_SETUP(IFX_PMU_DISABLE);
    //PPE_TOP_PMU_SETUP(IFX_PMU_DISABLE);*/
}

static inline void reset_ppe(struct platform_device *pdev)
{
#if 0 //MODULE
    unsigned int etop_cfg;
    unsigned int etop_mdio_cfg;
    unsigned int etop_ig_plen_ctrl;
    unsigned int enet_mac_cfg;

    etop_cfg            = *IFX_PP32_ETOP_CFG;
    etop_mdio_cfg       = *IFX_PP32_ETOP_MDIO_CFG;
    etop_ig_plen_ctrl   = *IFX_PP32_ETOP_IG_PLEN_CTRL;
    enet_mac_cfg        = *IFX_PP32_ENET_MAC_CFG;

    *IFX_PP32_ETOP_CFG  = (*IFX_PP32_ETOP_CFG & ~0x03C0) | 0x0001;

    //  reset PPE
    ifx_rcu_rst(IFX_RCU_DOMAIN_PPE, IFX_RCU_MODULE_ATM);

    *IFX_PP32_ETOP_MDIO_CFG     = etop_mdio_cfg;
    *IFX_PP32_ETOP_IG_PLEN_CTRL = etop_ig_plen_ctrl;
    *IFX_PP32_ENET_MAC_CFG      = enet_mac_cfg;
    *IFX_PP32_ETOP_CFG          = etop_cfg;
#endif
}

static inline void init_ema(void)
{
    IFX_REG_W32((EMA_CMD_BUF_LEN << 16) | (EMA_CMD_BASE_ADDR >> 2), EMA_CMDCFG);
    IFX_REG_W32((EMA_DATA_BUF_LEN << 16) | (EMA_DATA_BASE_ADDR >> 2), EMA_DATACFG);
    IFX_REG_W32(0x000000FF, EMA_IER);
    IFX_REG_W32(EMA_READ_BURST | (EMA_WRITE_BURST << 2), EMA_CFG);
}

static inline void init_mailbox(void)
{
    IFX_REG_W32(0xFFFFFFFF, MBOX_IGU1_ISRC);
    IFX_REG_W32(0x00000000, MBOX_IGU1_IER);
    IFX_REG_W32(0xFFFFFFFF, MBOX_IGU3_ISRC);
    IFX_REG_W32(0x00000000, MBOX_IGU3_IER);
}

static inline void init_atm_tc(void)
{
    IFX_REG_W32(0x0000,     DREG_AT_CTRL);
    IFX_REG_W32(0x0000,     DREG_AR_CTRL);
    IFX_REG_W32(0x0,        DREG_AT_IDLE0);
    IFX_REG_W32(0x0,        DREG_AT_IDLE1);
    IFX_REG_W32(0x0,        DREG_AR_IDLE0);
    IFX_REG_W32(0x0,        DREG_AR_IDLE1);
    IFX_REG_W32(0x40,       RFBI_CFG);
    IFX_REG_W32(0x0700,     SFSM_DBA0);
    IFX_REG_W32(0x0818,     SFSM_DBA1);
    IFX_REG_W32(0x0930,     SFSM_CBA0);
    IFX_REG_W32(0x0944,     SFSM_CBA1);
    IFX_REG_W32(0x14014,    SFSM_CFG0);
    IFX_REG_W32(0x14014,    SFSM_CFG1);
    IFX_REG_W32(0x0958,     FFSM_DBA0);
    IFX_REG_W32(0x09AC,     FFSM_DBA1);
    IFX_REG_W32(0x10006,    FFSM_CFG0);
    IFX_REG_W32(0x10006,    FFSM_CFG1);
    IFX_REG_W32(0x00000001, FFSM_IDLE_HEAD_BC0);
    IFX_REG_W32(0x00000001, FFSM_IDLE_HEAD_BC1);
}

static inline void clear_share_buffer(void)
{
    volatile u32 *p = SB_RAM0_ADDR(0);
    unsigned int i;

    for ( i = 0; i < SB_RAM0_DWLEN + SB_RAM1_DWLEN; i++ )
        IFX_REG_W32(0, p++);
}

/*
 *  Description:
 *    Download PPE firmware binary code.
 *  Input:
 *    src       --- u32 *, binary code buffer
 *    dword_len --- unsigned int, binary code length in DWORD (32-bit)
 *  Output:
 *    int       --- 0:    Success
 *                  else:           Error Code
 */
static inline int pp32_download_code(u32 *code_src, unsigned int code_dword_len, u32 *data_src, unsigned int data_dword_len)
{
    volatile u32 *dest;

    if ( code_src == 0 || ((unsigned long)code_src & 0x03) != 0
        || data_src == 0 || ((unsigned long)data_src & 0x03) != 0 )
        return -1;

    if ( code_dword_len <= CDM_CODE_MEMORYn_DWLEN(0) )
        IFX_REG_W32(0x00, CDM_CFG);
    else
        IFX_REG_W32(0x04, CDM_CFG);

    /*  copy code   */
    dest = CDM_CODE_MEMORY(0, 0);
    while ( code_dword_len-- > 0 )
        IFX_REG_W32(*code_src++, dest++);

    /*  copy data   */
    dest = CDM_DATA_MEMORY(0, 0);
    while ( data_dword_len-- > 0 )
        IFX_REG_W32(*data_src++, dest++);

    return 0;
}



/*
 * ####################################
 *           Global Function
 * ####################################
 */

extern void ase_fw_ver(unsigned int *major, unsigned int *minor)
{
    ASSERT(major != NULL, "pointer is NULL");
    ASSERT(minor != NULL, "pointer is NULL");

    *major = FW_VER_ID->major;
    *minor = FW_VER_ID->minor;
}

void ase_init(struct platform_device *pdev)
{
    init_pmu();

    reset_ppe(pdev);

    init_ema();

    init_mailbox();

    init_atm_tc();

    clear_share_buffer();
}

void ase_shutdown(void)
{
    uninit_pmu();
}

/*
 *  Description:
 *    Initialize and start up PP32.
 *  Input:
 *    none
 *  Output:
 *    int  --- 0: Success
 *             else:        Error Code
 */
int ase_start(int pp32)
{
    int ret;

    /*  download firmware   */
    ret = pp32_download_code(firmware_binary_code, sizeof(firmware_binary_code) / sizeof(*firmware_binary_code), firmware_binary_data, sizeof(firmware_binary_data) / sizeof(*firmware_binary_data));
    if ( ret != 0 )
        return ret;

    /*  run PP32    */
    IFX_REG_W32(DBG_CTRL_RESTART, PP32_DBG_CTRL);

    /*  idle for a while to let PP32 init itself    */
    udelay(10);

    return 0;
}

/*
 *  Description:
 *    Halt PP32.
 *  Input:
 *    none
 *  Output:
 *    none
 */
void ase_stop(int pp32)
{
    /*  halt PP32   */
    IFX_REG_W32(DBG_CTRL_STOP, PP32_DBG_CTRL);
}

struct ltq_atm_ops ase_ops = {
	.init = ase_init,
	.shutdown = ase_shutdown,
	.start = ase_start,
	.stop = ase_stop,
	.fw_ver = ase_fw_ver,
};

