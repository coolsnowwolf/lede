/******************************************************************************
**
** FILE NAME    : ifxmips_atm_ar9.c
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

#include "ifxmips_atm_fw_ar9.h"
#include "ifxmips_atm_fw_regs_ar9.h"

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
#define EMA_CMD_BASE_ADDR    (0x00003B80 << 2)
#define EMA_DATA_BUF_LEN     0x0100
#define EMA_DATA_BASE_ADDR   (0x00003C00 << 2)
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
	ltq_pmu_enable(IFX_PMU_MODULE_PPE_SLL01 |
		IFX_PMU_MODULE_PPE_TC |
		IFX_PMU_MODULE_PPE_EMA |
		IFX_PMU_MODULE_PPE_QSB |
		IFX_PMU_MODULE_TPE |
		IFX_PMU_MODULE_DSL_DFE);
}

static inline void uninit_pmu(void)
{
}

static inline void reset_ppe(struct platform_device *pdev)
{
#ifdef MODULE
    //  reset PPE
//    ifx_rcu_rst(IFX_RCU_DOMAIN_PPE, IFX_RCU_MODULE_ATM);
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

static inline void clear_share_buffer(void)
{
    volatile u32 *p = SB_RAM0_ADDR(0);
    unsigned int i;

    for ( i = 0; i < SB_RAM0_DWLEN + SB_RAM1_DWLEN + SB_RAM2_DWLEN + SB_RAM3_DWLEN + SB_RAM4_DWLEN; i++ )
        IFX_REG_W32(0, p++);
}

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

void ar9_fw_ver(unsigned int *major, unsigned int *minor)
{
    ASSERT(major != NULL, "pointer is NULL");
    ASSERT(minor != NULL, "pointer is NULL");

    *major = FW_VER_ID->major;
    *minor = FW_VER_ID->minor;
}

void ar9_init(struct platform_device *pdev)
{
	init_pmu();
	reset_ppe(pdev);
	init_ema();
	init_mailbox();
	clear_share_buffer();
}

void ar9_shutdown(void)
{
	ltq_pmu_disable(IFX_PMU_MODULE_PPE_SLL01 |
		IFX_PMU_MODULE_PPE_TC |
		IFX_PMU_MODULE_PPE_EMA |
		IFX_PMU_MODULE_PPE_QSB |
		IFX_PMU_MODULE_TPE |
		IFX_PMU_MODULE_DSL_DFE);
}

int ar9_start(int pp32)
{
	int ret;

	ret = pp32_download_code(ar9_fw_bin, sizeof(ar9_fw_bin) / sizeof(*ar9_fw_bin),
			ar9_fw_data, sizeof(ar9_fw_data) / sizeof(*ar9_fw_data));
	if ( ret != 0 )
		return ret;

	IFX_REG_W32(DBG_CTRL_RESTART, PP32_DBG_CTRL(0));

	udelay(10);

	return 0;
}

void ar9_stop(int pp32)
{
	IFX_REG_W32(DBG_CTRL_STOP, PP32_DBG_CTRL(0));
}

struct ltq_atm_ops ar9_ops = {
	.init = ar9_init,
	.shutdown = ar9_shutdown,
	.start = ar9_start,
	.stop = ar9_stop,
	.fw_ver = ar9_fw_ver,
};


