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
 * Feature : LUT related functions
 *
 */


#ifndef _RTL8367C_ASICDRV_LUT_H_
#define _RTL8367C_ASICDRV_LUT_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_LUT_AGETIMERMAX        (7)
#define RTL8367C_LUT_AGESPEEDMAX        (3)
#define RTL8367C_LUT_LEARNLIMITMAX      (0x1040)
#define RTL8367C_LUT_ADDRMAX            (0x103F)
#define RTL8367C_LUT_IPMCGRP_TABLE_MAX  (0x3F)
#define RTL8367C_LUT_ENTRY_SIZE         (6)
#define RTL8367C_LUT_BUSY_CHECK_NO      (10)

#define RTL8367C_LUT_TABLE_SIZE         (6)

enum RTL8367C_LUTHASHMETHOD{

    LUTHASHMETHOD_SVL=0,
    LUTHASHMETHOD_IVL,
    LUTHASHMETHOD_END,
};


enum RTL8367C_LRNOVERACT{

    LRNOVERACT_FORWARD=0,
    LRNOVERACT_DROP,
    LRNOVERACT_TRAP,
    LRNOVERACT_END,
};

enum RTL8367C_LUTREADMETHOD{

    LUTREADMETHOD_MAC =0,
    LUTREADMETHOD_ADDRESS,
    LUTREADMETHOD_NEXT_ADDRESS,
    LUTREADMETHOD_NEXT_L2UC,
    LUTREADMETHOD_NEXT_L2MC,
    LUTREADMETHOD_NEXT_L3MC,
    LUTREADMETHOD_NEXT_L2L3MC,
    LUTREADMETHOD_NEXT_L2UCSPA,
};

enum RTL8367C_FLUSHMODE
{
    FLUSHMDOE_PORT = 0,
    FLUSHMDOE_VID,
    FLUSHMDOE_FID,
    FLUSHMDOE_END,
};

enum RTL8367C_FLUSHTYPE
{
    FLUSHTYPE_DYNAMIC = 0,
    FLUSHTYPE_BOTH,
    FLUSHTYPE_END,
};


typedef struct LUTTABLE{

    ipaddr_t sip;
    ipaddr_t dip;
    ether_addr_t mac;
    rtk_uint16 ivl_svl:1;
    rtk_uint16 cvid_fid:12;
    rtk_uint16 fid:4;
    rtk_uint16 efid:3;

    rtk_uint16 nosalearn:1;
    rtk_uint16 da_block:1;
    rtk_uint16 sa_block:1;
    rtk_uint16 auth:1;
    rtk_uint16 lut_pri:3;
    rtk_uint16 sa_en:1;
    rtk_uint16 fwd_en:1;
    rtk_uint16 mbr:11;
    rtk_uint16 spa:4;
    rtk_uint16 age:3;
    rtk_uint16 l3lookup:1;
    rtk_uint16 igmp_asic:1;
    rtk_uint16 igmpidx:8;

    rtk_uint16 lookup_hit:1;
    rtk_uint16 lookup_busy:1;
    rtk_uint16 address:13;

    rtk_uint16 l3vidlookup:1;
    rtk_uint16 l3_vid:12;

    rtk_uint16 wait_time;

}rtl8367c_luttb;

extern ret_t rtl8367c_setAsicLutIpMulticastLookup(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicLutIpMulticastLookup(rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicLutIpMulticastVidLookup(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicLutIpMulticastVidLookup(rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicLutAgeTimerSpeed(rtk_uint32 timer, rtk_uint32 speed);
extern ret_t rtl8367c_getAsicLutAgeTimerSpeed(rtk_uint32* pTimer, rtk_uint32* pSpeed);
extern ret_t rtl8367c_setAsicLutCamTbUsage(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicLutCamTbUsage(rtk_uint32* pEnabled);
extern ret_t rtl8367c_getAsicLutCamType(rtk_uint32* pType);
extern ret_t rtl8367c_setAsicLutLearnLimitNo(rtk_uint32 port, rtk_uint32 number);
extern ret_t rtl8367c_getAsicLutLearnLimitNo(rtk_uint32 port, rtk_uint32* pNumber);
extern ret_t rtl8367c_setAsicSystemLutLearnLimitNo(rtk_uint32 number);
extern ret_t rtl8367c_getAsicSystemLutLearnLimitNo(rtk_uint32 *pNumber);
extern ret_t rtl8367c_setAsicLutLearnOverAct(rtk_uint32 action);
extern ret_t rtl8367c_getAsicLutLearnOverAct(rtk_uint32* pAction);
extern ret_t rtl8367c_setAsicSystemLutLearnOverAct(rtk_uint32 action);
extern ret_t rtl8367c_getAsicSystemLutLearnOverAct(rtk_uint32 *pAction);
extern ret_t rtl8367c_setAsicSystemLutLearnPortMask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicSystemLutLearnPortMask(rtk_uint32 *pPortmask);
extern ret_t rtl8367c_setAsicL2LookupTb(rtl8367c_luttb *pL2Table);
extern ret_t rtl8367c_getAsicL2LookupTb(rtk_uint32 method, rtl8367c_luttb *pL2Table);
extern ret_t rtl8367c_getAsicLutLearnNo(rtk_uint32 port, rtk_uint32* pNumber);
extern ret_t rtl8367c_setAsicLutIpLookupMethod(rtk_uint32 type);
extern ret_t rtl8367c_getAsicLutIpLookupMethod(rtk_uint32* pType);
extern ret_t rtl8367c_setAsicLutForceFlush(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicLutForceFlushStatus(rtk_uint32 *pPortmask);
extern ret_t rtl8367c_setAsicLutFlushMode(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicLutFlushMode(rtk_uint32* pMode);
extern ret_t rtl8367c_setAsicLutFlushType(rtk_uint32 type);
extern ret_t rtl8367c_getAsicLutFlushType(rtk_uint32* pType);
extern ret_t rtl8367c_setAsicLutFlushVid(rtk_uint32 vid);
extern ret_t rtl8367c_getAsicLutFlushVid(rtk_uint32* pVid);
extern ret_t rtl8367c_setAsicLutFlushFid(rtk_uint32 fid);
extern ret_t rtl8367c_getAsicLutFlushFid(rtk_uint32* pFid);
extern ret_t rtl8367c_setAsicLutDisableAging(rtk_uint32 port, rtk_uint32 disabled);
extern ret_t rtl8367c_getAsicLutDisableAging(rtk_uint32 port, rtk_uint32 *pDisabled);
extern ret_t rtl8367c_setAsicLutIPMCGroup(rtk_uint32 index, ipaddr_t group_addr, rtk_uint32 vid, rtk_uint32 pmask, rtk_uint32 valid);
extern ret_t rtl8367c_getAsicLutIPMCGroup(rtk_uint32 index, ipaddr_t *pGroup_addr, rtk_uint32 *pVid, rtk_uint32 *pPmask, rtk_uint32 *pValid);
extern ret_t rtl8367c_setAsicLutLinkDownForceAging(rtk_uint32 enable);
extern ret_t rtl8367c_getAsicLutLinkDownForceAging(rtk_uint32 *pEnable);
extern ret_t rtl8367c_setAsicLutFlushAll(void);
extern ret_t rtl8367c_getAsicLutFlushAllStatus(rtk_uint32 *pBusyStatus);
extern ret_t rtl8367c_setAsicLutIpmcFwdRouterPort(rtk_uint32 enable);
extern ret_t rtl8367c_getAsicLutIpmcFwdRouterPort(rtk_uint32 *pEnable);

#endif /*_RTL8367C_ASICDRV_LUT_H_*/

