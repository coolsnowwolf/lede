/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_ar9.c
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
#include "ifxmips_ptm_adsl.h"
#include "ifxmips_ptm_fw_ar9.h"

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
#define EMA_CMD_BASE_ADDR    (0x00001B80 << 2)
#define EMA_DATA_BUF_LEN     0x0100
#define EMA_DATA_BASE_ADDR   (0x00001C00 << 2)
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
	ltq_pmu_enable(IFX_PMU_MODULE_PPE_SLL01 |
		IFX_PMU_MODULE_PPE_TC |
		IFX_PMU_MODULE_PPE_EMA |
		IFX_PMU_MODULE_TPE |
		IFX_PMU_MODULE_DSL_DFE);

}

static inline void uninit_pmu(void)
{
	ltq_pmu_disable(IFX_PMU_MODULE_PPE_SLL01 |
		IFX_PMU_MODULE_PPE_TC |
		IFX_PMU_MODULE_PPE_EMA |
		IFX_PMU_MODULE_TPE |
		IFX_PMU_MODULE_DSL_DFE);

}

static inline void reset_ppe(struct platform_device *pdev)
{
#ifdef MODULE
    //  reset PPE
//    ifx_rcu_rst(IFX_RCU_DOMAIN_PPE, IFX_RCU_MODULE_PTM);
#endif
}

static inline void init_ema(void)
{
    //  Configure share buffer master selection
    IFX_REG_W32(1, SB_MST_PRI0);
    IFX_REG_W32(1, SB_MST_PRI1);

    //  EMA Settings
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
    IFX_REG_W32(0x0,        RFBI_CFG);
    IFX_REG_W32(0x1800,     SFSM_DBA0);
    IFX_REG_W32(0x1921,     SFSM_DBA1);
    IFX_REG_W32(0x1A42,     SFSM_CBA0);
    IFX_REG_W32(0x1A53,     SFSM_CBA1);
    IFX_REG_W32(0x14011,    SFSM_CFG0);
    IFX_REG_W32(0x14011,    SFSM_CFG1);
    IFX_REG_W32(0x1000,     FFSM_DBA0);
    IFX_REG_W32(0x1700,     FFSM_DBA1);
    IFX_REG_W32(0x3000C,    FFSM_CFG0);
    IFX_REG_W32(0x3000C,    FFSM_CFG1);
    IFX_REG_W32(0xF0D10000, FFSM_IDLE_HEAD_BC0);
    IFX_REG_W32(0xF0D10000, FFSM_IDLE_HEAD_BC1);

    /*
     *  0. Backup port2 value to temp
     *  1. Disable CPU port2 in switch (link and learning)
     *  2. wait for a while
     *  3. Configure DM register and counter
     *  4. restore temp to CPU port2 in switch
     *  This code will cause network to stop working if there are heavy
     *  traffic during bootup. This part should be moved to switch and use
     *  the same code as ATM
     */
    {
        int i;
        u32 temp;

        temp = IFX_REG_R32(SW_P2_CTL);

        IFX_REG_W32(0x40020000, SW_P2_CTL);
        for (i = 0; i < 200; i++)
            udelay(2000);

        IFX_REG_W32(0x00007028, DM_RXCFG);
        IFX_REG_W32(0x00007028, DS_RXCFG);

        IFX_REG_W32(0x00001100, DM_RXDB);
        IFX_REG_W32(0x00001100, DS_RXDB);

        IFX_REG_W32(0x00001600, DM_RXCB);
        IFX_REG_W32(0x00001600, DS_RXCB);

        /*
         * For dynamic, must reset these counters,
         * For once initialization, don't need to reset these counters
         */
        IFX_REG_W32(0x0, DM_RXPGCNT);
        IFX_REG_W32(0x0, DS_RXPGCNT);
        IFX_REG_W32(0x0, DM_RXPKTCNT);

        IFX_REG_W32_MASK(0, 0x80000000, DM_RXCFG);
        IFX_REG_W32_MASK(0, 0x8000, DS_RXCFG);

        udelay(2000);
        IFX_REG_W32(temp, SW_P2_CTL);
        udelay(2000);
    }
}

static inline void clear_share_buffer(void)
{
    volatile u32 *p = SB_RAM0_ADDR(0);
    unsigned int i;

    for ( i = 0; i < SB_RAM0_DWLEN + SB_RAM1_DWLEN + SB_RAM2_DWLEN + SB_RAM3_DWLEN + SB_RAM4_DWLEN; i++ )
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

void ifx_ptm_get_fw_ver(unsigned int *major, unsigned int *minor)
{
    ASSERT(major != NULL, "pointer is NULL");
    ASSERT(minor != NULL, "pointer is NULL");

    *major = FW_VER_ID->major;
    *minor = FW_VER_ID->minor;
}

void ifx_ptm_init_chip(struct platform_device *pdev)
{
    init_pmu();

    reset_ppe(pdev);

    init_ema();

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
    int ret;

    /*  download firmware   */
    ret = pp32_download_code(firmware_binary_code, sizeof(firmware_binary_code) / sizeof(*firmware_binary_code), firmware_binary_data, sizeof(firmware_binary_data) / sizeof(*firmware_binary_data));
    if ( ret != 0 )
        return ret;

    /*  run PP32    */
    IFX_REG_W32(DBG_CTRL_RESTART, PP32_DBG_CTRL(0));

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
    /*  halt PP32   */
    IFX_REG_W32(DBG_CTRL_STOP, PP32_DBG_CTRL(0));
}

int ifx_ptm_proc_read_regs(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + off + len, "EMA:\n");
    len += sprintf(page + off + len, "  SB_MST_PRI0 - 0x%08X, SB_MST_PRI1 - 0x%08X\n", IFX_REG_R32(SB_MST_PRI0), IFX_REG_R32(SB_MST_PRI1));
    len += sprintf(page + off + len, "  EMA_CMDCFG  - 0x%08X, EMA_DATACFG - 0x%08X\n", IFX_REG_R32(EMA_CMDCFG), IFX_REG_R32(EMA_DATACFG));
    len += sprintf(page + off + len, "  EMA_IER     - 0x%08X, EMA_CFG     - 0x%08X\n", IFX_REG_R32(EMA_IER), IFX_REG_R32(EMA_CFG));

    len += sprintf(page + off + len, "Mailbox:\n");
    len += sprintf(page + off + len, "  MBOX_IGU1_IER - 0x%08X, MBOX_IGU1_ISR - 0x%08X\n", IFX_REG_R32(MBOX_IGU1_IER), IFX_REG_R32(MBOX_IGU1_ISR));
    len += sprintf(page + off + len, "  MBOX_IGU3_IER - 0x%08X, MBOX_IGU3_ISR - 0x%08X\n", IFX_REG_R32(MBOX_IGU3_IER), IFX_REG_R32(MBOX_IGU3_ISR));

    len += sprintf(page + off + len, "TC:\n");
    len += sprintf(page + off + len, "  RFBI_CFG  - 0x%08X\n", IFX_REG_R32(RFBI_CFG));
    len += sprintf(page + off + len, "  SFSM_DBA0 - 0x%08X, SFSM_CBA0 - 0x%08X, SFSM_CFG0 - 0x%08X\n", IFX_REG_R32(SFSM_DBA0), IFX_REG_R32(SFSM_CBA0), IFX_REG_R32(SFSM_CFG0));
    len += sprintf(page + off + len, "  SFSM_DBA1 - 0x%08X, SFSM_CBA1 - 0x%08X, SFSM_CFG1 - 0x%08X\n", IFX_REG_R32(SFSM_DBA1), IFX_REG_R32(SFSM_CBA1), IFX_REG_R32(SFSM_CFG1));
    len += sprintf(page + off + len, "  FFSM_DBA0 - 0x%08X, FFSM_CFG0 - 0x%08X, IDLE_HEAD - 0x%08X\n", IFX_REG_R32(FFSM_DBA0), IFX_REG_R32(FFSM_CFG0), IFX_REG_R32(FFSM_IDLE_HEAD_BC0));
    len += sprintf(page + off + len, "  FFSM_DBA1 - 0x%08X, FFSM_CFG1 - 0x%08X, IDLE_HEAD - 0x%08X\n", IFX_REG_R32(FFSM_DBA1), IFX_REG_R32(FFSM_CFG1), IFX_REG_R32(FFSM_IDLE_HEAD_BC1));

    len += sprintf(page + off + len, "DPlus:\n");
    len += sprintf(page + off + len, "  DM_RXDB    - 0x%08X, DM_RXCB     - 0x%08X, DM_RXCFG - 0x%08X\n", IFX_REG_R32(DM_RXDB), IFX_REG_R32(DM_RXCB), IFX_REG_R32(DM_RXCFG));
    len += sprintf(page + off + len, "  DM_RXPGCNT - 0x%08X, DM_RXPKTCNT - 0x%08X\n", IFX_REG_R32(DM_RXPGCNT), IFX_REG_R32(DM_RXPKTCNT));
    len += sprintf(page + off + len, "  DS_RXDB    - 0x%08X, DS_RXCB     - 0x%08X, DS_RXCFG - 0x%08X\n", IFX_REG_R32(DS_RXDB), IFX_REG_R32(DS_RXCB), IFX_REG_R32(DS_RXCFG));
    len += sprintf(page + off + len, "  DS_RXPGCNT - 0x%08X\n", IFX_REG_R32(DS_RXPGCNT));

    *eof = 1;

    return len;
}
