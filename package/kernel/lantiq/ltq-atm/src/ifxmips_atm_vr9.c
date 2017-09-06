/******************************************************************************
**
** FILE NAME    : ifxmips_atm_vr9.c
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
#include <asm/delay.h>

#include "ifxmips_atm_core.h"
#include "ifxmips_atm_fw_vr9.h"

#ifdef CONFIG_VR9

#include <lantiq_soc.h>

#define IFX_PMU_MODULE_PPE_SLL01  BIT(19)
#define IFX_PMU_MODULE_PPE_TC     BIT(21)
#define IFX_PMU_MODULE_PPE_EMA    BIT(22)
#define IFX_PMU_MODULE_PPE_QSB    BIT(18)
#define IFX_PMU_MODULE_AHBS       BIT(13)
#define IFX_PMU_MODULE_DSL_DFE    BIT(9)

static inline void vr9_reset_ppe(void)
{
/*#ifdef MODULE
	//  reset PPE
	ifx_rcu_rst(IFX_RCU_DOMAIN_DSLDFE, IFX_RCU_MODULE_ATM);
	udelay(1000);
	ifx_rcu_rst(IFX_RCU_DOMAIN_DSLTC, IFX_RCU_MODULE_ATM);
	udelay(1000);
	ifx_rcu_rst(IFX_RCU_DOMAIN_PPE, IFX_RCU_MODULE_ATM);
	udelay(1000);
	*PP32_SRST &= ~0x000303CF;
	udelay(1000);
	*PP32_SRST |= 0x000303CF;
	udelay(1000);
#endif*/
}

static inline int vr9_pp32_download_code(int pp32, u32 *code_src, unsigned int code_dword_len, u32 *data_src, unsigned int data_dword_len)
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

	dest = CDM_CODE_MEMORY(pp32, 0);
	while ( code_dword_len-- > 0 )
		IFX_REG_W32(*code_src++, dest++);

	dest = CDM_DATA_MEMORY(pp32, 0);
	while ( data_dword_len-- > 0 )
		IFX_REG_W32(*data_src++, dest++);

	return 0;
}

static void vr9_fw_ver(unsigned int *major, unsigned int *minor)
{

    *major = FW_VER_ID->major;
    *minor = FW_VER_ID->minor;
}

static void vr9_init(void)
{
	volatile u32 *p;
	unsigned int i;

	/* setup pmu */
	ltq_pmu_enable(IFX_PMU_MODULE_PPE_SLL01 |
		IFX_PMU_MODULE_PPE_TC |
		IFX_PMU_MODULE_PPE_EMA |
		IFX_PMU_MODULE_PPE_QSB |
		IFX_PMU_MODULE_AHBS |
		IFX_PMU_MODULE_DSL_DFE);

	vr9_reset_ppe();

	/* pdma init */
	IFX_REG_W32(0x08,       PDMA_CFG);
	IFX_REG_W32(0x00203580, SAR_PDMA_RX_CMDBUF_CFG);
	IFX_REG_W32(0x004035A0, SAR_PDMA_RX_FW_CMDBUF_CFG);

	/* mailbox init */
	IFX_REG_W32(0xFFFFFFFF, MBOX_IGU1_ISRC);
	IFX_REG_W32(0x00000000, MBOX_IGU1_IER);
	IFX_REG_W32(0xFFFFFFFF, MBOX_IGU3_ISRC);
	IFX_REG_W32(0x00000000, MBOX_IGU3_IER);

	/* tc init - clear sync state */
	*SFSM_STATE0 = 0;
	*SFSM_STATE1 = 0;

	/* init shared buffer */
	p = SB_RAM0_ADDR(0);
	for ( i = 0; i < SB_RAM0_DWLEN + SB_RAM1_DWLEN + SB_RAM2_DWLEN + SB_RAM3_DWLEN; i++ )
		IFX_REG_W32(0, p++);

	p = SB_RAM6_ADDR(0);
	for ( i = 0; i < SB_RAM6_DWLEN; i++ )
		IFX_REG_W32(0, p++);
}

static void vr9_shutdown(void)
{
}

static int vr9_start(int pp32)
{
	unsigned int mask = 1 << (pp32 << 4);
	int ret;

	/*  download firmware   */
	ret = vr9_pp32_download_code(pp32,
		vr9_fw_bin, sizeof(vr9_fw_bin) / sizeof(*vr9_fw_bin),
		vr9_fw_data, sizeof(vr9_fw_data) / sizeof(*vr9_fw_data));
	if ( ret != 0 )
		return ret;

	/*  run PP32    */
	IFX_REG_W32_MASK(mask, 0, PP32_FREEZE);

	/*  idle for a while to let PP32 init itself    */
	udelay(10);

	return 0;
}

static void vr9_stop(int pp32)
{
	unsigned int mask = 1 << (pp32 << 4);

	IFX_REG_W32_MASK(0, mask, PP32_FREEZE);
}

struct ltq_atm_ops vr9_ops = {
	.init = vr9_init,
	.shutdown = vr9_shutdown,
	.start = vr9_start,
	.stop = vr9_stop,
	.fw_ver = vr9_fw_ver,
};

#endif
