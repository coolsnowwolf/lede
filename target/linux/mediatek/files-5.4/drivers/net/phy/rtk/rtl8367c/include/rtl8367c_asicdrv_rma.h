/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 64716 $
 * $Date: 2015-12-31 16:31:55 +0800 (週四, 31 十二月 2015) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : RMA related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_RMA_H_
#define _RTL8367C_ASICDRV_RMA_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_RMAMAX                     0x2F

enum RTL8367C_RMAOP
{
    RMAOP_FORWARD = 0,
    RMAOP_TRAP_TO_CPU,
    RMAOP_DROP,
    RMAOP_FORWARD_EXCLUDE_CPU,
    RMAOP_END
};


typedef struct  rtl8367c_rma_s{

    rtk_uint16 operation;
    rtk_uint16 discard_storm_filter;
    rtk_uint16 trap_priority;
    rtk_uint16 keep_format;
    rtk_uint16 vlan_leaky;
    rtk_uint16 portiso_leaky;

}rtl8367c_rma_t;


extern ret_t rtl8367c_setAsicRma(rtk_uint32 index, rtl8367c_rma_t* pRmacfg);
extern ret_t rtl8367c_getAsicRma(rtk_uint32 index, rtl8367c_rma_t* pRmacfg);
extern ret_t rtl8367c_setAsicRmaCdp(rtl8367c_rma_t* pRmacfg);
extern ret_t rtl8367c_getAsicRmaCdp(rtl8367c_rma_t* pRmacfg);
extern ret_t rtl8367c_setAsicRmaCsstp(rtl8367c_rma_t* pRmacfg);
extern ret_t rtl8367c_getAsicRmaCsstp(rtl8367c_rma_t* pRmacfg);
extern ret_t rtl8367c_setAsicRmaLldp(rtk_uint32 enabled, rtl8367c_rma_t* pRmacfg);
extern ret_t rtl8367c_getAsicRmaLldp(rtk_uint32 *pEnabled, rtl8367c_rma_t* pRmacfg);

#endif /*#ifndef _RTL8367C_ASICDRV_RMA_H_*/

