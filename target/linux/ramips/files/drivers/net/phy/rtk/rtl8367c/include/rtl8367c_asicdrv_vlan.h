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
 * Feature : VLAN related functions
 *
 */


#ifndef _RTL8367C_ASICDRV_VLAN_H_
#define _RTL8367C_ASICDRV_VLAN_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <rtl8367c_asicdrv.h>

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define RTL8367C_PROTOVLAN_GIDX_MAX 3
#define RTL8367C_PROTOVLAN_GROUPNO  4

#define RTL8367C_VLAN_BUSY_CHECK_NO     (10)

#define RTL8367C_VLAN_MBRCFG_LEN    (4)
#define RTL8367C_VLAN_4KTABLE_LEN   (3)

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef struct  VLANCONFIGUSER
{
    rtk_uint16  evid;
    rtk_uint16  mbr;
    rtk_uint16  fid_msti;
    rtk_uint16  envlanpol;
    rtk_uint16  meteridx;
    rtk_uint16  vbpen;
    rtk_uint16  vbpri;
}rtl8367c_vlanconfiguser;

typedef struct  USER_VLANTABLE{

    rtk_uint16  vid;
    rtk_uint16  mbr;
    rtk_uint16  untag;
    rtk_uint16  fid_msti;
    rtk_uint16  envlanpol;
    rtk_uint16  meteridx;
    rtk_uint16  vbpen;
    rtk_uint16  vbpri;
    rtk_uint16  ivl_svl;

}rtl8367c_user_vlan4kentry;

typedef enum
{
    FRAME_TYPE_BOTH = 0,
    FRAME_TYPE_TAGGED_ONLY,
    FRAME_TYPE_UNTAGGED_ONLY,
    FRAME_TYPE_MAX_BOUND
} rtl8367c_accframetype;

typedef enum
{
    EG_TAG_MODE_ORI = 0,
    EG_TAG_MODE_KEEP,
    EG_TAG_MODE_PRI_TAG,
    EG_TAG_MODE_REAL_KEEP,
    EG_TAG_MODE_END
} rtl8367c_egtagmode;

typedef enum
{
    PPVLAN_FRAME_TYPE_ETHERNET = 0,
    PPVLAN_FRAME_TYPE_LLC,
    PPVLAN_FRAME_TYPE_RFC1042,
    PPVLAN_FRAME_TYPE_END
} rtl8367c_provlan_frametype;

enum RTL8367C_STPST
{
    STPST_DISABLED = 0,
    STPST_BLOCKING,
    STPST_LEARNING,
    STPST_FORWARDING
};

enum RTL8367C_RESVIDACT
{
    RES_VID_ACT_UNTAG = 0,
    RES_VID_ACT_TAG,
    RES_VID_ACT_END
};

typedef struct
{
    rtl8367c_provlan_frametype  frameType;
    rtk_uint32                      etherType;
} rtl8367c_protocolgdatacfg;

typedef struct
{
    rtk_uint32 valid;
    rtk_uint32 vlan_idx;
    rtk_uint32 priority;
} rtl8367c_protocolvlancfg;

extern ret_t rtl8367c_setAsicVlanMemberConfig(rtk_uint32 index, rtl8367c_vlanconfiguser *pVlanCg);
extern ret_t rtl8367c_getAsicVlanMemberConfig(rtk_uint32 index, rtl8367c_vlanconfiguser *pVlanCg);
extern ret_t rtl8367c_setAsicVlan4kEntry(rtl8367c_user_vlan4kentry *pVlan4kEntry );
extern ret_t rtl8367c_getAsicVlan4kEntry(rtl8367c_user_vlan4kentry *pVlan4kEntry );
extern ret_t rtl8367c_setAsicVlanAccpetFrameType(rtk_uint32 port, rtl8367c_accframetype frameType);
extern ret_t rtl8367c_getAsicVlanAccpetFrameType(rtk_uint32 port, rtl8367c_accframetype *pFrameType);
extern ret_t rtl8367c_setAsicVlanIngressFilter(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicVlanIngressFilter(rtk_uint32 port, rtk_uint32 *pEnable);
extern ret_t rtl8367c_setAsicVlanEgressTagMode(rtk_uint32 port, rtl8367c_egtagmode tagMode);
extern ret_t rtl8367c_getAsicVlanEgressTagMode(rtk_uint32 port, rtl8367c_egtagmode *pTagMode);
extern ret_t rtl8367c_setAsicVlanPortBasedVID(rtk_uint32 port, rtk_uint32 index, rtk_uint32 pri);
extern ret_t rtl8367c_getAsicVlanPortBasedVID(rtk_uint32 port, rtk_uint32 *pIndex, rtk_uint32 *pPri);
extern ret_t rtl8367c_setAsicVlanProtocolBasedGroupData(rtk_uint32 index, rtl8367c_protocolgdatacfg *pPbCfg);
extern ret_t rtl8367c_getAsicVlanProtocolBasedGroupData(rtk_uint32 index, rtl8367c_protocolgdatacfg *pPbCfg);
extern ret_t rtl8367c_setAsicVlanPortAndProtocolBased(rtk_uint32 port, rtk_uint32 index, rtl8367c_protocolvlancfg *pPpbCfg);
extern ret_t rtl8367c_getAsicVlanPortAndProtocolBased(rtk_uint32 port, rtk_uint32 index, rtl8367c_protocolvlancfg *pPpbCfg);
extern ret_t rtl8367c_setAsicVlanFilter(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicVlanFilter(rtk_uint32* pEnabled);

extern ret_t rtl8367c_setAsicPortBasedFid(rtk_uint32 port, rtk_uint32 fid);
extern ret_t rtl8367c_getAsicPortBasedFid(rtk_uint32 port, rtk_uint32* pFid);
extern ret_t rtl8367c_setAsicPortBasedFidEn(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicPortBasedFidEn(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicSpanningTreeStatus(rtk_uint32 port, rtk_uint32 msti, rtk_uint32 state);
extern ret_t rtl8367c_getAsicSpanningTreeStatus(rtk_uint32 port, rtk_uint32 msti, rtk_uint32* pState);
extern ret_t rtl8367c_setAsicVlanUntagDscpPriorityEn(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicVlanUntagDscpPriorityEn(rtk_uint32* enabled);
extern ret_t rtl8367c_setAsicVlanTransparent(rtk_uint32 port, rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicVlanTransparent(rtk_uint32 port, rtk_uint32 *pPortmask);
extern ret_t rtl8367c_setAsicVlanEgressKeep(rtk_uint32 port, rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicVlanEgressKeep(rtk_uint32 port, rtk_uint32* pPortmask);
extern ret_t rtl8367c_setReservedVidAction(rtk_uint32 vid0Action, rtk_uint32 vid4095Action);
extern ret_t rtl8367c_getReservedVidAction(rtk_uint32 *pVid0Action, rtk_uint32 *pVid4095Action);
extern ret_t rtl8367c_setRealKeepRemarkEn(rtk_uint32 enabled);
extern ret_t rtl8367c_getRealKeepRemarkEn(rtk_uint32 *pEnabled);
extern ret_t rtl8367c_resetVlan(void);

#endif /*#ifndef _RTL8367C_ASICDRV_VLAN_H_*/

