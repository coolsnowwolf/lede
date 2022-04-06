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
 * Feature : Flow control related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_FC_H_
#define _RTL8367C_ASICDRV_FC_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_PAGE_NUMBER    0x600


enum FLOW_CONTROL_TYPE
{
    FC_EGRESS = 0,
    FC_INGRESS,
};

enum FC_JUMBO_SIZE
{
    FC_JUMBO_SIZE_3K = 0,
    FC_JUMBO_SIZE_4K,
    FC_JUMBO_SIZE_6K,
    FC_JUMBO_SIZE_9K,
    FC_JUMBO_SIZE_END,

};


extern ret_t rtl8367c_setAsicFlowControlSelect(rtk_uint32 select);
extern ret_t rtl8367c_getAsicFlowControlSelect(rtk_uint32 *pSelect);
extern ret_t rtl8367c_setAsicFlowControlJumboMode(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicFlowControlJumboMode(rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicFlowControlJumboModeSize(rtk_uint32 size);
extern ret_t rtl8367c_getAsicFlowControlJumboModeSize(rtk_uint32* pSize);
extern ret_t rtl8367c_setAsicFlowControlQueueEgressEnable(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicFlowControlQueueEgressEnable(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicFlowControlDropAll(rtk_uint32 dropall);
extern ret_t rtl8367c_getAsicFlowControlDropAll(rtk_uint32* pDropall);
extern ret_t rtl8367c_setAsicFlowControlPauseAllThreshold(rtk_uint32 threshold);
extern ret_t rtl8367c_getAsicFlowControlPauseAllThreshold(rtk_uint32 *pThreshold);
extern ret_t rtl8367c_setAsicFlowControlSystemThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlSystemThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlSharedThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlSharedThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlPortThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlPortThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlPortPrivateThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlPortPrivateThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlSystemDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlSystemDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlSharedDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlSharedDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlPortDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlPortDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlPortPrivateDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlPortPrivateDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlSystemJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlSystemJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlSharedJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlSharedJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlPortJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlPortJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367c_setAsicFlowControlPortPrivateJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367c_getAsicFlowControlPortPrivateJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);

extern ret_t rtl8367c_setAsicEgressFlowControlPortDropGap(rtk_uint32 gap);
extern ret_t rtl8367c_getAsicEgressFlowControlPortDropGap(rtk_uint32 *pGap);
extern ret_t rtl8367c_setAsicEgressFlowControlQueueDropGap(rtk_uint32 gap);
extern ret_t rtl8367c_getAsicEgressFlowControlQueueDropGap(rtk_uint32 *pGap);
extern ret_t rtl8367c_setAsicEgressFlowControlPortDropThreshold(rtk_uint32 port, rtk_uint32 threshold);
extern ret_t rtl8367c_getAsicEgressFlowControlPortDropThreshold(rtk_uint32 port, rtk_uint32 *pThreshold);
extern ret_t rtl8367c_setAsicEgressFlowControlQueueDropThreshold(rtk_uint32 qid, rtk_uint32 threshold);
extern ret_t rtl8367c_getAsicEgressFlowControlQueueDropThreshold(rtk_uint32 qid, rtk_uint32 *pThreshold);
extern ret_t rtl8367c_getAsicEgressQueueEmptyPortMask(rtk_uint32 *pPortmask);
extern ret_t rtl8367c_getAsicTotalPage(rtk_uint32 *pPageCount);
extern ret_t rtl8367c_getAsicPulbicPage(rtk_uint32 *pPageCount);
extern ret_t rtl8367c_getAsicMaxTotalPage(rtk_uint32 *pPageCount);
extern ret_t rtl8367c_getAsicMaxPulbicPage(rtk_uint32 *pPageCount);
extern ret_t rtl8367c_getAsicPortPage(rtk_uint32 port, rtk_uint32 *pPageCount);
extern ret_t rtl8367c_getAsicPortPageMax(rtk_uint32 port, rtk_uint32 *pPageCount);
extern ret_t rtl8367c_setAsicFlowControlEgressPortIndep(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367c_getAsicFlowControlEgressPortIndep(rtk_uint32 port, rtk_uint32 *pEnable);

#endif /*_RTL8367C_ASICDRV_FC_H_*/

