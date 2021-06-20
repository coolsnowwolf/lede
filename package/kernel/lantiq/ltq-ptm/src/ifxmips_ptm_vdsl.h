/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_vdsl.h
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver header file (core functions for VR9)
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

#ifndef IFXMIPS_PTM_VDSL_H
#define IFXMIPS_PTM_VDSL_H

#include <linux/version.h>
#include <linux/netdevice.h>
#include <lantiq_ptm.h>
#include "ifxmips_ptm_common.h"
#include "ifxmips_ptm_ppe_common.h"
#include "ifxmips_ptm_fw_regs_vdsl.h"

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
#define ETH_WATCHDOG_TIMEOUT            (10 * HZ)

/*
 *  DMA RX/TX Channel Parameters
 */
#define MAX_ITF_NUMBER                  1
#define MAX_RX_DMA_CHANNEL_NUMBER       1
#define MAX_TX_DMA_CHANNEL_NUMBER       1
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
#define RX_MAX_BUFFER_SIZE              (1600 + RX_HEAD_MAC_ADDR_ALIGNMENT)
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
    unsigned int                    rx_desc_pos;

    unsigned int                    tx_desc_pos;

    unsigned int                    tx_swap_desc_pos;

    struct net_device_stats         stats;

    struct napi_struct              napi;
};

struct ptm_priv_data {
    struct ptm_itf                  itf[MAX_ITF_NUMBER];
};



/*
 * ####################################
 *             Declaration
 * ####################################
 */

extern unsigned int ifx_ptm_dbg_enable;

extern void ifx_ptm_get_fw_ver(unsigned int *major, unsigned int *mid, unsigned int *minor);

extern void ifx_ptm_init_chip(struct platform_device *pdev);
extern void ifx_ptm_uninit_chip(void);

extern int ifx_pp32_start(int pp32);
extern void ifx_pp32_stop(int pp32);

extern void ifx_reset_ppe(void);



#endif  //  IFXMIPS_PTM_VDSL_H
