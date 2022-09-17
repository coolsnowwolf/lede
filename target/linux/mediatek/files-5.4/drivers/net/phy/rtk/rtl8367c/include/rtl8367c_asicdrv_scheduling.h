/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76306 $
 * $Date: 2017-03-08 15:13:58 +0800 (¶g¤T, 08 ¤T¤ë 2017) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : Packet Scheduling related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_SCHEDULING_H_
#define _RTL8367C_ASICDRV_SCHEDULING_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_QWEIGHTMAX    0x7F
#define RTL8367C_PORT_QUEUE_METER_INDEX_MAX    7

/* enum for queue type */
enum QUEUETYPE
{
    QTYPE_STRICT = 0,
    QTYPE_WFQ,
};
extern ret_t rtl8367c_setAsicLeakyBucketParameter(rtk_uint32 tick, rtk_uint32 token);
extern ret_t rtl8367c_getAsicLeakyBucketParameter(rtk_uint32 *tick, rtk_uint32 *token);
extern ret_t rtl8367c_setAsicAprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 apridx);
extern ret_t rtl8367c_getAsicAprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *apridx);
extern ret_t rtl8367c_setAsicPprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 ppridx);
extern ret_t rtl8367c_getAsicPprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *ppridx);
extern ret_t rtl8367c_setAsicAprEnable(rtk_uint32 port, rtk_uint32 aprEnable);
extern ret_t rtl8367c_getAsicAprEnable(rtk_uint32 port, rtk_uint32 *aprEnable);
extern ret_t rtl8367c_setAsicPprEnable(rtk_uint32 port, rtk_uint32 pprEnable);
extern ret_t rtl8367c_getAsicPprEnable(rtk_uint32 port, rtk_uint32 *pprEnable);

extern ret_t rtl8367c_setAsicWFQWeight(rtk_uint32, rtk_uint32 queueid, rtk_uint32 weight );
extern ret_t rtl8367c_getAsicWFQWeight(rtk_uint32, rtk_uint32 queueid, rtk_uint32 *weight );
extern ret_t rtl8367c_setAsicWFQBurstSize(rtk_uint32 burstsize);
extern ret_t rtl8367c_getAsicWFQBurstSize(rtk_uint32 *burstsize);

extern ret_t rtl8367c_setAsicQueueType(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 queueType);
extern ret_t rtl8367c_getAsicQueueType(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *queueType);
extern ret_t rtl8367c_setAsicQueueRate(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 ppridx, rtk_uint32 apridx );
extern ret_t rtl8367c_getAsicQueueRate(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* ppridx, rtk_uint32* apridx );
extern ret_t rtl8367c_setAsicPortEgressRate(rtk_uint32 port, rtk_uint32 rate);
extern ret_t rtl8367c_getAsicPortEgressRate(rtk_uint32 port, rtk_uint32 *rate);
extern ret_t rtl8367c_setAsicPortEgressRateIfg(rtk_uint32 ifg);
extern ret_t rtl8367c_getAsicPortEgressRateIfg(rtk_uint32 *ifg);

#endif /*_RTL8367C_ASICDRV_SCHEDULING_H_*/

