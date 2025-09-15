/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_adsl.h
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver header file (core functions for Danube/Amazon-SE/
**                AR9)
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
** 17 JUN 2009  Xu Liang        Init Version
*******************************************************************************/

#ifndef IFXMIPS_PTM_ADSL_H
#define IFXMIPS_PTM_ADSL_H



#include <linux/version.h>
#include <linux/netdevice.h>
#include <lantiq_ptm.h>
#include "ifxmips_ptm_common.h"
#include "ifxmips_ptm_ppe_common.h"
#include "ifxmips_ptm_fw_regs_adsl.h"

#define CONFIG_IFXMIPS_DSL_CPE_MEI
#define INT_NUM_IM2_IRL24	(INT_NUM_IM2_IRL0 + 24)

#define IFX_REG_W32(_v, _r)               __raw_writel((_v), (volatile unsigned int *)(_r))
#define IFX_REG_R32(_r)                    __raw_readl((volatile unsigned int *)(_r))
#define IFX_REG_W32_MASK(_clr, _set, _r)   IFX_REG_W32((IFX_REG_R32((_r)) & ~(_clr)) | (_set), (_r))
#define SET_BITS(x, msb, lsb, value)    (((x) & ~(((1 << ((msb) + 1)) - 1) ^ ((1 << (lsb)) - 1))) | (((value) & ((1 << (1 + (msb) - (lsb))) - 1)) << (lsb)))




/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  Constant Definition
 */
#define ETH_WATCHDOG_TIMEOUT            (2 * HZ)

/*
 *  DMA RX/TX Channel Parameters
 */
#define MAX_ITF_NUMBER                  2
#define MAX_RX_DMA_CHANNEL_NUMBER       MAX_ITF_NUMBER
#define MAX_TX_DMA_CHANNEL_NUMBER       MAX_ITF_NUMBER
#define DATA_BUFFER_ALIGNMENT           EMA_ALIGNMENT
#define DESC_ALIGNMENT                  8

/*
 *  Ethernet Frame Definitions
 */
#define ETH_MAC_HEADER_LENGTH           14
#define ETH_CRC_LENGTH                  4
#define ETH_MIN_FRAME_LENGTH            64
#define ETH_MAX_FRAME_LENGTH            (1518 + 4 * 2)

/*
 *  RX Frame Definitions
 */
#define RX_HEAD_MAC_ADDR_ALIGNMENT      2
#define RX_TAIL_CRC_LENGTH              0   //  PTM firmware does not have ethernet frame CRC
                                            //  The len in descriptor doesn't include ETH_CRC
                                            //  because ETH_CRC may not present in some configuration



/*
 * ####################################
 *              Data Type
 * ####################################
 */

struct ptm_itf {
    volatile struct rx_descriptor  *rx_desc;
    unsigned int                    rx_desc_pos;

    volatile struct tx_descriptor  *tx_desc;
    unsigned int                    tx_desc_pos;
    struct sk_buff                **tx_skb;

    struct net_device_stats         stats;

    struct napi_struct              napi;
};

struct ptm_priv_data {
    struct ptm_itf                  itf[MAX_ITF_NUMBER];

    void                           *rx_desc_base;
    void                           *tx_desc_base;
    void                           *tx_skb_base;
};



/*
 * ####################################
 *             Declaration
 * ####################################
 */

extern unsigned int ifx_ptm_dbg_enable;

extern void ifx_ptm_get_fw_ver(unsigned int *major, unsigned int *minor);

extern void ifx_ptm_init_chip(struct platform_device *pdev);
extern void ifx_ptm_uninit_chip(void);

extern int ifx_pp32_start(int pp32);
extern void ifx_pp32_stop(int pp32);

extern void ifx_reset_ppe(void);

extern int ifx_ptm_proc_read_regs(char *page, char **start, off_t off, int count, int *eof, void *data);



#endif  //  IFXMIPS_PTM_ADSL_H
