/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_vr9.c
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver common source file (core functions)
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
#include <linux/reset.h>
#include <asm/delay.h>

/*
 *  Chip Specific Head File
 */
#include "ifxmips_ptm_vdsl.h"
#include "ifxmips_ptm_fw_vr9.h"

#include <lantiq_soc.h>

static inline void init_pmu(void);
static inline void uninit_pmu(void);
static inline void reset_ppe(struct platform_device *pdev);
static inline void init_pdma(void);
static inline void init_mailbox(void);
static inline void init_atm_tc(void);
static inline void clear_share_buffer(void);

#define IFX_PMU_MODULE_PPE_SLL01  BIT(19)
#define IFX_PMU_MODULE_PPE_TC     BIT(21)
#define IFX_PMU_MODULE_PPE_EMA    BIT(22)
#define IFX_PMU_MODULE_PPE_QSB    BIT(18)
#define IFX_PMU_MODULE_AHBS       BIT(13)
#define IFX_PMU_MODULE_DSL_DFE    BIT(9)


static inline void init_pmu(void)
{
	ltq_pmu_enable(IFX_PMU_MODULE_PPE_SLL01 |
		IFX_PMU_MODULE_PPE_TC |
		IFX_PMU_MODULE_PPE_EMA |
		IFX_PMU_MODULE_AHBS |
		IFX_PMU_MODULE_DSL_DFE);

}

static inline void uninit_pmu(void)
{
}

static inline void reset_ppe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct reset_control *dsp;
	struct reset_control *dfe;
	struct reset_control *tc;

	dsp = devm_reset_control_get(dev, "dsp");
	if (IS_ERR(dsp)) {
		if (PTR_ERR(dsp) != -EPROBE_DEFER)
			dev_err(dev, "Failed to lookup dsp reset\n");
// 		return PTR_ERR(dsp);
	}

	dfe = devm_reset_control_get(dev, "dfe");
	if (IS_ERR(dfe)) {
		if (PTR_ERR(dfe) != -EPROBE_DEFER)
			dev_err(dev, "Failed to lookup dfe reset\n");
// 		return PTR_ERR(dfe);
	}

	tc = devm_reset_control_get(dev, "tc");
	if (IS_ERR(tc)) {
		if (PTR_ERR(tc) != -EPROBE_DEFER)
			dev_err(dev, "Failed to lookup tc reset\n");
// 		return PTR_ERR(tc);
	}

	reset_control_assert(dsp);
	udelay(1000);
	reset_control_assert(dfe);
	udelay(1000);
	reset_control_assert(tc);
	udelay(1000);
	*PP32_SRST &= ~0x000303CF;
	udelay(1000);
	*PP32_SRST |= 0x000303CF;
	udelay(1000);
}

static inline void init_pdma(void)
{
    IFX_REG_W32(0x00000001, PDMA_CFG);
    IFX_REG_W32(0x00082C00, PDMA_RX_CTX_CFG);
    IFX_REG_W32(0x00081B00, PDMA_TX_CTX_CFG);
    IFX_REG_W32(0x02040604, PDMA_RX_MAX_LEN_REG);
    IFX_REG_W32(0x000F003F, PDMA_RX_DELAY_CFG);

    IFX_REG_W32(0x00000011, SAR_MODE_CFG);
    IFX_REG_W32(0x00082A00, SAR_RX_CTX_CFG);
    IFX_REG_W32(0x00082E00, SAR_TX_CTX_CFG);
    IFX_REG_W32(0x00001021, SAR_POLY_CFG_SET0);
    IFX_REG_W32(0x1EDC6F41, SAR_POLY_CFG_SET1);
    IFX_REG_W32(0x04C11DB7, SAR_POLY_CFG_SET2);
    IFX_REG_W32(0x00000F3E, SAR_CRC_SIZE_CFG);

    IFX_REG_W32(0x01001900, SAR_PDMA_RX_CMDBUF_CFG);
    IFX_REG_W32(0x01001A00, SAR_PDMA_TX_CMDBUF_CFG);
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
    IFX_REG_W32(0x00010040, SFSM_CFG0);
    IFX_REG_W32(0x00010040, SFSM_CFG1);
    IFX_REG_W32(0x00020000, SFSM_PGCNT0);
    IFX_REG_W32(0x00020000, SFSM_PGCNT1);
    IFX_REG_W32(0x00000000, DREG_AT_IDLE0);
    IFX_REG_W32(0x00000000, DREG_AT_IDLE1);
    IFX_REG_W32(0x00000000, DREG_AR_IDLE0);
    IFX_REG_W32(0x00000000, DREG_AR_IDLE1);
    IFX_REG_W32(0x0000080C, DREG_B0_LADR);
    IFX_REG_W32(0x0000080C, DREG_B1_LADR);

    IFX_REG_W32(0x000001F0, DREG_AR_CFG0);
    IFX_REG_W32(0x000001F0, DREG_AR_CFG1);
    IFX_REG_W32(0x000001E0, DREG_AT_CFG0);
    IFX_REG_W32(0x000001E0, DREG_AT_CFG1);

    /*  clear sync state    */
    //IFX_REG_W32(0, SFSM_STATE0);
    //IFX_REG_W32(0, SFSM_STATE1);

    IFX_REG_W32_MASK(0, 1 << 14, SFSM_CFG0);    //  enable SFSM storing
    IFX_REG_W32_MASK(0, 1 << 14, SFSM_CFG1);

    IFX_REG_W32_MASK(0, 1 << 15, SFSM_CFG0);    //  HW keep the IDLE cells in RTHA buffer
    IFX_REG_W32_MASK(0, 1 << 15, SFSM_CFG1);

    IFX_REG_W32(0xF0D10000, FFSM_IDLE_HEAD_BC0);
    IFX_REG_W32(0xF0D10000, FFSM_IDLE_HEAD_BC1);
    IFX_REG_W32(0x00030028, FFSM_CFG0);         //  Force_idle
    IFX_REG_W32(0x00030028, FFSM_CFG1);
}

static inline void clear_share_buffer(void)
{
    volatile u32 *p;
    unsigned int i;

    p = SB_RAM0_ADDR(0);
    for ( i = 0; i < SB_RAM0_DWLEN + SB_RAM1_DWLEN + SB_RAM2_DWLEN + SB_RAM3_DWLEN; i++ )
        IFX_REG_W32(0, p++);

    p = SB_RAM6_ADDR(0);
    for ( i = 0; i < SB_RAM6_DWLEN; i++ )
        IFX_REG_W32(0, p++);
}

/*
 *  Description:
 *    Download PPE firmware binary code.
 *  Input:
 *    pp32      --- int, which pp32 core
 *    src       --- u32 *, binary code buffer
 *    dword_len --- unsigned int, binary code length in DWORD (32-bit)
 *  Output:
 *    int       --- 0:    Success
 *                  else:           Error Code
 */
static inline int pp32_download_code(int pp32, u32 *code_src, unsigned int code_dword_len, u32 *data_src, unsigned int data_dword_len)
{
    unsigned int clr, set;
    volatile u32 *dest;

    if ( code_src == 0 || ((unsigned long)code_src & 0x03) != 0
        || data_src == 0 || ((unsigned long)data_src & 0x03) != 0 )
        return -1;

    clr = pp32 ? 0xF0 : 0x0F;
    if ( code_dword_len <= CDM_CODE_MEMORYn_DWLEN(0) )
        set = pp32 ? (3 << 6): (2 << 2);
    else
        set = 0x00;
    IFX_REG_W32_MASK(clr, set, CDM_CFG);

    /*  copy code   */
    dest = CDM_CODE_MEMORY(pp32, 0);
    while ( code_dword_len-- > 0 )
        IFX_REG_W32(*code_src++, dest++);

    /*  copy data   */
    dest = CDM_DATA_MEMORY(pp32, 0);
    while ( data_dword_len-- > 0 )
        IFX_REG_W32(*data_src++, dest++);

    return 0;
}



/*
 * ####################################
 *           Global Function
 * ####################################
 */

void ifx_ptm_get_fw_ver(unsigned int *major, unsigned int *mid, unsigned int *minor)
{
    ASSERT(major != NULL, "pointer is NULL");
    ASSERT(minor != NULL, "pointer is NULL");

    if ( *(volatile unsigned int *)FW_VER_ID_NEW == 0 ) {
        *major = FW_VER_ID->major;
        *mid   = ~0;
        *minor = FW_VER_ID->minor;
    }
    else {
        *major = FW_VER_ID_NEW->major;
        *mid   = FW_VER_ID_NEW->middle;
        *minor = FW_VER_ID_NEW->minor;
    }
}

void ifx_ptm_init_chip(struct platform_device *pdev)
{
    init_pmu();

    reset_ppe(pdev);

    init_pdma();

    init_mailbox();

    init_atm_tc();

    clear_share_buffer();
}

void ifx_ptm_uninit_chip(void)
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
int ifx_pp32_start(int pp32)
{
    unsigned int mask = 1 << (pp32 << 4);
    int ret;

    /*  download firmware   */
    ret = pp32_download_code(pp32, firmware_binary_code, sizeof(firmware_binary_code) / sizeof(*firmware_binary_code), firmware_binary_data, sizeof(firmware_binary_data) / sizeof(*firmware_binary_data));
    if ( ret != 0 )
        return ret;

    /*  run PP32    */
    IFX_REG_W32_MASK(mask, 0, PP32_FREEZE);

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
void ifx_pp32_stop(int pp32)
{
    unsigned int mask = 1 << (pp32 << 4);

    /*  halt PP32   */
    IFX_REG_W32_MASK(0, mask, PP32_FREEZE);
}
