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
 * Feature : Ethernet AV related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_EAV_H_
#define _RTL8367C_ASICDRV_EAV_H_

#include <rtl8367c_asicdrv.h>

typedef enum RTL8367C_PTP_TIME_CMD_E
{
    PTP_TIME_READ = 0,
    PTP_TIME_WRITE,
    PTP_TIME_INC,
    PTP_TIME_DEC,
    PTP_TIME_CMD_END
}RTL8367C_PTP_TIME_CMD;

typedef enum RTL8367C_PTP_TIME_ADJ_E
{
    PTP_TIME_ADJ_INC = 0,
    PTP_TIME_ADJ_DEC,
    PTP_TIME_ADJ_END
}RTL8367C_PTP_TIME_ADJ;

typedef enum RTL8367C_PTP_TIME_CTRL_E
{
    PTP_TIME_CTRL_STOP = 0,
    PTP_TIME_CTRL_START,
    PTP_TIME_CTRL_END
}RTL8367C_PTP_TIME_CTRL;

typedef enum RTL8367C_PTP_INTR_IMRS_E
{
    PTP_IMRS_TX_SYNC,
    PTP_IMRS_TX_DELAY_REQ,
    PTP_IMRS_TX_PDELAY_REQ,
    PTP_IMRS_TX_PDELAY_RESP,
    PTP_IMRS_RX_SYNC,
    PTP_IMRS_RX_DELAY_REQ,
    PTP_IMRS_RX_PDELAY_REQ,
    PTP_IMRS_RX_PDELAY_RESP,
    PTP_IMRS_END,
}RTL8367C_PTP_INTR_IMRS;


typedef enum RTL8367C_PTP_PKT_TYPE_E
{
    PTP_PKT_TYPE_TX_SYNC,
    PTP_PKT_TYPE_TX_DELAY_REQ,
    PTP_PKT_TYPE_TX_PDELAY_REQ,
    PTP_PKT_TYPE_TX_PDELAY_RESP,
    PTP_PKT_TYPE_RX_SYNC,
    PTP_PKT_TYPE_RX_DELAY_REQ,
    PTP_PKT_TYPE_RX_PDELAY_REQ,
    PTP_PKT_TYPE_RX_PDELAY_RESP,
    PTP_PKT_TYPE_END,
}RTL8367C_PTP_PKT_TYPE;

typedef struct  rtl8367c_ptp_time_stamp_s{
    rtk_uint32 sequence_id;
    rtk_uint32 second;
    rtk_uint32 nano_second;
}rtl8367c_ptp_time_stamp_t;

#define RTL8367C_PTP_INTR_MASK        0xFF

#define RTL8367C_PTP_PORT_MASK        0x3FF

extern ret_t rtl8367c_setAsicEavMacAddress(ether_addr_t mac);
extern ret_t rtl8367c_getAsicEavMacAddress(ether_addr_t *pMac);
extern ret_t rtl8367c_setAsicEavTpid(rtk_uint32 outerTag, rtk_uint32 innerTag);
extern ret_t rtl8367c_getAsicEavTpid(rtk_uint32* pOuterTag, rtk_uint32* pInnerTag);
extern ret_t rtl8367c_setAsicEavSysTime(rtk_uint32 second, rtk_uint32 nanoSecond);
extern ret_t rtl8367c_getAsicEavSysTime(rtk_uint32* pSecond, rtk_uint32* pNanoSecond);
extern ret_t rtl8367c_setAsicEavSysTimeAdjust(rtk_uint32 type, rtk_uint32 second, rtk_uint32 nanoSecond);
extern ret_t rtl8367c_setAsicEavSysTimeCtrl(rtk_uint32 control);
extern ret_t rtl8367c_getAsicEavSysTimeCtrl(rtk_uint32* pControl);
extern ret_t rtl8367c_setAsicEavInterruptMask(rtk_uint32 imr);
extern ret_t rtl8367c_getAsicEavInterruptMask(rtk_uint32* pImr);
extern ret_t rtl8367c_getAsicEavInterruptStatus(rtk_uint32* pIms);
extern ret_t rtl8367c_setAsicEavPortInterruptStatus(rtk_uint32 port, rtk_uint32 ims);
extern ret_t rtl8367c_getAsicEavPortInterruptStatus(rtk_uint32 port, rtk_uint32* pIms);
extern ret_t rtl8367c_setAsicEavPortEnable(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicEavPortEnable(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367c_getAsicEavPortTimeStamp(rtk_uint32 port, rtk_uint32 type, rtl8367c_ptp_time_stamp_t* timeStamp);

extern ret_t rtl8367c_setAsicEavTrap(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicEavTrap(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsicEavEnable(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicEavEnable(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsicEavPriRemapping(rtk_uint32 srcpriority, rtk_uint32 priority);
extern ret_t rtl8367c_getAsicEavPriRemapping(rtk_uint32 srcpriority, rtk_uint32 *pPriority);

#endif /*#ifndef _RTL8367C_ASICDRV_EAV_H_*/

