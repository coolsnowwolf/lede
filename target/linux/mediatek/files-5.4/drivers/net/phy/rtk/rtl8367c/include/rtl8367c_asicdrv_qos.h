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
 * Feature : Qos related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_QOS_H_
#define _RTL8367C_ASICDRV_QOS_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_DECISIONPRIMAX    0xFF

/* enum Priority Selection Types */
enum PRIDECISION
{
    PRIDEC_PORT = 0,
    PRIDEC_ACL,
    PRIDEC_DSCP,
    PRIDEC_1Q,
    PRIDEC_1AD,
    PRIDEC_CVLAN,
    PRIDEC_DA,
    PRIDEC_SA,
    PRIDEC_END,
};

/* enum Priority Selection Index */
enum RTL8367C_PRIDEC_TABLE
{
    PRIDEC_IDX0 = 0,
    PRIDEC_IDX1,
    PRIDEC_IDX_END,
};

enum RTL8367C_DOT1P_PRISEL
{
    DOT1P_PRISEL_USER =  0,
    DOT1P_PRISEL_TAG,
    DOT1P_PRISEL_END
};

enum RTL8367C_DSCP_PRISEL
{
    DSCP_PRISEL_INTERNAL =  0,
    DSCP_PRISEL_DSCP,
    DSCP_PRISEL_USER ,
    DSCP_PRISEL_END
};


extern ret_t rtl8367c_setAsicRemarkingDot1pAbility(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicRemarkingDot1pAbility(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicRemarkingDot1pParameter(rtk_uint32 priority, rtk_uint32 newPriority );
extern ret_t rtl8367c_getAsicRemarkingDot1pParameter(rtk_uint32 priority, rtk_uint32 *pNewPriority );
extern  ret_t rtl8367c_setAsicRemarkingDot1pSrc(rtk_uint32 type);
extern  ret_t rtl8367c_getAsicRemarkingDot1pSrc(rtk_uint32 *pType);
extern ret_t rtl8367c_setAsicRemarkingDscpAbility(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicRemarkingDscpAbility(rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicRemarkingDscpParameter(rtk_uint32 priority, rtk_uint32 newDscp );
extern ret_t rtl8367c_getAsicRemarkingDscpParameter(rtk_uint32 priority, rtk_uint32* pNewDscp );

extern ret_t rtl8367c_setAsicPriorityDot1qRemapping(rtk_uint32 srcpriority, rtk_uint32 priority );
extern ret_t rtl8367c_getAsicPriorityDot1qRemapping(rtk_uint32 srcpriority, rtk_uint32 *pPriority );
extern ret_t rtl8367c_setAsicPriorityDscpBased(rtk_uint32 dscp, rtk_uint32 priority );
extern ret_t rtl8367c_getAsicPriorityDscpBased(rtk_uint32 dscp, rtk_uint32 *pPriority );
extern ret_t rtl8367c_setAsicPriorityPortBased(rtk_uint32 port, rtk_uint32 priority );
extern ret_t rtl8367c_getAsicPriorityPortBased(rtk_uint32 port, rtk_uint32 *pPriority );
extern ret_t rtl8367c_setAsicPriorityDecision(rtk_uint32 index, rtk_uint32 prisrc, rtk_uint32 decisionPri);
extern ret_t rtl8367c_getAsicPriorityDecision(rtk_uint32 index, rtk_uint32 prisrc, rtk_uint32* pDecisionPri);
extern ret_t rtl8367c_setAsicPriorityToQIDMappingTable(rtk_uint32 qnum, rtk_uint32 priority, rtk_uint32 qid );
extern ret_t rtl8367c_getAsicPriorityToQIDMappingTable(rtk_uint32 qnum, rtk_uint32 priority, rtk_uint32* pQid);
extern ret_t rtl8367c_setAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 qnum );
extern ret_t rtl8367c_getAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 *pQnum );

extern ret_t rtl8367c_setAsicRemarkingDscpSrc(rtk_uint32 type);
extern ret_t rtl8367c_getAsicRemarkingDscpSrc(rtk_uint32 *pType);
extern ret_t rtl8367c_setAsicRemarkingDscp2Dscp(rtk_uint32 dscp, rtk_uint32 rmkDscp);
extern ret_t rtl8367c_getAsicRemarkingDscp2Dscp(rtk_uint32 dscp, rtk_uint32 *pRmkDscp);

extern ret_t rtl8367c_setAsicPortPriorityDecisionIndex(rtk_uint32 port, rtk_uint32 index );
extern ret_t rtl8367c_getAsicPortPriorityDecisionIndex(rtk_uint32 port, rtk_uint32 *pIndex );

#endif /*#ifndef _RTL8367C_ASICDRV_QOS_H_*/

