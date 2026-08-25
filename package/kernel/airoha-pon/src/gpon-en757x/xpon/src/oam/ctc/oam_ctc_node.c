/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved. MediaTeK's source   code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

    Module Name:
    oam_ctc_node.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Lightman        2012/7/5        Create
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if_ether.h>

#include "libepon.h"
#include "epon_oam.h"
#include "ctc/oam_ctc_node.h"
#include "ctc/oam_ctc_alarm.h"
#include "ctc/oam_ext_cmn.h"
#include "ctc/oam_ext_mgr.h"
#include "ctc/oam_sys_api.h"
#include "ctc/oam_ctc_dspch.h"

#ifdef TCSUPPORT_EPON_MAPPING
#include "libeponmap.h"
#endif

#ifdef TCSUPPORT_MT7530_SWITCH_API
#define u8  unsigned char 
#define u16 unsigned short
#define u32 unsigned int
#include "mtkswitch_api.h"
#endif

#ifdef TCSUPPORT_XPON_IGMP
#include "lib_xpon_igmp.h"
#endif

#ifdef TCSUPPORT_PON_VLAN
#include "lib_pon_vlan.h"
#endif

#ifdef TCSUPPORT_EPON_OAM_CUC
#include "ctc/oam_cuc_init.h"
#endif

#ifdef TCSUPPORT_PONMGR
#include "api/mgr_api.h"
#include "epon_sys_api.h"
#endif


extern OamCtcDB_t gCtcDB;
extern ExtHandler_t gCtcHandler;
/*
 * CTC Attributes and actions table.
 */
OamParamNode_t oamParamTable[]=
{
    //  {"Name", objectType, branch, leaf, length, GET, SET}
    {"aPhyAdminState",                      OBJ_BIT_PORT,   OAM_STD_ATTR,   OAM_LF_PHYADMINSTATE,           0x04,   oamGetPhyAdminState,            NULL},
    {"aAutoNegAdminState",                  OBJ_BIT_PORT,   OAM_STD_ATTR,   OAM_LF_AUTONEGADMINSTATE,       0x04,   oamGetAutoNegAdminState,        NULL},
    {"aAutoNegLocalTechnologyAbility",      OBJ_BIT_PORT,   OAM_STD_ATTR,   OAM_LF_AUTONEGLOCALTECHABILITY, 0xFF,   oamGetAutoNegLocalTechAbility,  NULL},
    {"aAutoNegAdvertisedTechnologyAbility", OBJ_BIT_PORT,   OAM_STD_ATTR,   OAM_LF_AUTONEGADVTECHABILITY,   0xFF,   oamGetAutoNegAdvTechAbility,    NULL},
    {"aFECAbility",                         OBJ_BIT_ONU,    OAM_STD_ATTR,   OAM_LF_FECABILITY,              0x04,   oamGetFecAbility,               NULL},
    {"aFECmode",                            OBJ_BIT_ONU,    OAM_STD_ATTR,   OAM_LF_FECMODE,                 0x04,   oamGetFecMode,                  oamSetFecMode},
    
    {"ONUSN",                       OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_ONU_SN,                      0x26,   oamGetOnuSN,                        NULL},
    {"FirmwareVer",                 OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_FIRMWAREVER,                 0xFF,   oamGetFirmwareVer,                  NULL},
    {"ChipsetID",                   OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_CHIPSET_ID,                  0x08,   oamGetChipsetID,                    NULL},
    {"ONUCapabilities1",            OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_ONU_CAPABILITIES_1,          0x1A,   oamGetOnuCapabilities1,         NULL},
    {"ONUCapabilities2",            OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_ONU_CAPABILITIES_2,          0xFF,   oamGetOnuCapabilities2,         NULL},
    {"ONUCapabilities3",            OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_ONU_CAPABILITIES_3,          0x03,   oamGetOnuCapabilities3,         NULL},
    {"PowerSavingCapabilities",     OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_POWERSAVINGCAPABILITIES,     0x02,   oamGetPowerSavingCapabilities,      NULL},
    {"PowerSavingConfig",           OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_POWERSAVINGCONFIG,           0x07,   oamGetPowerSavingConfig,            oamSetPowerSavingConfig},
    {"ProtectionParameters",        OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_PROTECTIONPARAMETERS,        0x04,   oamGetProtectionParameters,     oamSetProtectionParameters},
    {"OpticalTransceiverDiagnosis", OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_OPTICALTRANSCEIVERDIAGNOSIS, 0x0A,   oamGetOpticalTransceiverDiagnosis,  NULL},
    {"ServiceSLA",                  OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_SERVICE_SLA,                 0xFF,   oamGetServiceSLA,                   oamSetServiceSLA},
    {"HoldoverConfig",              OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_HOLDOVERCONFIG,              0x08,   oamGetHoldoverConfig,               oamSetHoldoverConfig},
    {"ActivePON_IFAdminstate",      OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_ACTIVE_PON_IFADMINSTATE,     0x01,   oamGetActivePONIFAdminstate,        oamSetActivePONIFAdminstate},
    {"EthLinkState",                OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_ETHLINKSTATE,                0x01,   oamGetEthLinkState,             NULL},
    {"EthPortPause",                OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_ETHPORT_PAUSE,               0x01,   oamGetEthPortPause,             oamSetEthPortPause},
    {"EthPortUSPolicing",           OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_ETHPORTUS_POLICING,          0xFF,   oamGetEthPortUSPolicing,            oamSetEthPortUSPolicing},
#ifdef TCSUPPORT_VOIP
    {"VoIPPort",                    OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_VOIP_PORT,                   0x01,   oamGetVoIPPort,                 oamSetVoIPPort},
#endif
    {"E1Port",                      OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_E1_PORT,                     0x01,   oamGetE1Port,                       oamSetE1Port},
    {"EthPortDSRateLimiting",       OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_ETHPORTDS_RATELIMITING,      0xFF,   oamGetEthPortDSRateLimiting,        oamSetEthPortDSRateLimiting},
    {"PortLoopDetect",              OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_PORTLOOPDETECT,              0x04,   NULL,                               oamSetPortLoopDetect},
    {"PortDisableLooped",           OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_PORTDISABLELOOPED,           0x04,   NULL,                               oamSetPortDisableLooped},
    {"PortMACAgingTime",            OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_PORTMACAGINGTIME,            0x04,   oamGetPortMacAgingTime,         oamSetPortMacAgingTime},

    {"VLAN",                        OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_VLAN,                        0xFF,   oamGetVlan,                     oamSetVlan},
    {"ClassificationMarking",       OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_CLASSIFICATION_MARKING,      0xFF,   oamGetClassificationMarking,        oamSetClassificationMarking},
    {"AddDelMulticastVLAN",         OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_ADD_DEL_MULTICAST_VLAN,      0xFF,   oamGetAddDelMulticastVlan,          oamSetAddDelMulticastVlan},
    {"MulticastTagOper",            OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_MULTICASTTAGOPER,            0xFF,   oamGetMulticastTagOper,         oamSetMulticastTagOper},
    {"MulticastSwitch",             OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_MULTICASTSWITCH,             0x01,   oamGetMulticastSwitch,              oamSetMulticastSwitch},
    {"MulticastControl",            OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_MULTICASTCONTROL,            0xFF,   oamGetMulticastControl,         oamSetMulticastControl},
    {"GroupNumMax",                 OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_GROUP_NUM_MAX,               0x01,   oamGetGroupNumMax,                  oamSetGroupNumMax},
    {"aFastLeaveAbility",           OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_FASTLEAVEABILITY,            0xFF,   oamGetFastLeaveAbility,         NULL},
    {"aFastLeaveAdminState",        OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_FASTLEAVEADMINSTATE,         0x04,   oamGetFastLeaveAdminState,          NULL},

    {"LLIDQueueConfig_V3",             OBJ_BIT_LLID,        OAM_EXT_ATTR,   OAM_LF_LLIDQUEUECONFIG_V3,              0xFF,   oamGetLlidQueueConfig,              oamSetLlidQueueConfig},
//  {"LLIDQueueConfig_V2",      OBJ_BIT_LLID,       OAM_EXT_ATTR,   OAM_LF_LLIDQUEUECONFIG_V2,              0xFF,   oamGetLlidQueueConfig,          oamSetLlidQueueConfig},
    {"MxUMngGlobalParameter",       OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_MXUMNGGLOBALPARAMETER,       0xFF,   oamGetMxUMngGlobalParameter,        oamSetMxUMngGlobalParameter},
    {"MxUMngSNMPParameter",         OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_MXUMNGSNMPPARAMETER,         0xFF,   oamGetMxUMngSNMPParameter,      oamSetMxUMngSNMPParameter},
    {"ONUTxPowerSupplyControl",     OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_ONUTXPOWERSUPPLYCONTROL,     0x0A,   NULL,                               oamSetONUTxPowerSupplyControl},
#ifdef TCSUPPORT_VOIP
    {"IADInfo",                     OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_IADINFO,                     0x48,   oamGetIADInfo,                      NULL},
    {"GlobalParameterConfig",       OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_GLOBALPARAMETERCONFIG,       0x54,   oamGetGlobalParameterConfig,        oamSetGlobalParameterConfig},
    {"SIPParameterConfig",          OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_SIPPARAMETERCONFIG,          0x2D,   oamGetSIPParameterConfig,           oamSetSIPParameterConfig},
    {"SIPUserParameterConfig",      OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_SIPUSERPARAMETERCONFIG,      0x40,   oamGetSIPUserParameterConfig,       oamSetSIPUserParameterConfig},
    {"FaxModemConfig",              OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_FAX_MODEM_CONFIG,            0x02,   oamGetFaxModemConfig,               oamSetFaxModemConfig},
    {"POTSStatus",                  OBJ_BIT_PORT,   OAM_EXT_ATTR,   OAM_LF_POTSSTATUS,                  0x0C,   oamGetPOTSStatus,                   NULL},
#endif

    {"AlarmAdminState",              OBJ_BIT_ALL,   OAM_EXT_ATTR,   OAM_LF_ALARMADMINSTATE,             0x06,   oamGetAlarmAdminState,              oamSetAlarmAdminState},
    {"AlarmThreshold",               OBJ_BIT_ALL,   OAM_EXT_ATTR,   OAM_LF_ALARMTHRESHOLD,              0x0A,   oamGetAlarmThresholds,              oamSetAlarmThresholds},
#ifdef TCSUPPORT_PMMGR
    {"PerformanceMonitorStatus",     OBJ_BIT_PORT|OBJ_BIT_PONIF,    OAM_EXT_ATTR,   OAM_LF_PFMCSTATUS,  0x06,   oamGetPerformanceStatus,            oamSetPerformanceStatus},
    {"PerformanceMonitorCurrentData",OBJ_BIT_PORT|OBJ_BIT_PONIF,    OAM_EXT_ATTR,   OAM_LF_PFMCDATACURR,0x80,   oamGetPerformanceDataCurrent,       oamSetPerformanceDataCurrent},
    {"PerformanceMonitorHistoryData",OBJ_BIT_PORT|OBJ_BIT_PONIF,    OAM_EXT_ATTR,   OAM_LF_PFMCDATAHSTRY,0x80,  oamGetPerformanceDataHistory,       NULL},
#endif
/*  
#ifdef TCSUPPORT_VOIP
    {"H.248ParameterConfig",         OBJ_BIT_ONU,  OAM_EXT_ATTR,    OAM_LF_H248_PARAMETERCONFIG,        0x54,   oamGetH248ParameterConfig,          oamSetH248ParameterConfig},
    {"H.248UserTIDInfo",             OBJ_BIT_PORT, OAM_EXT_ATTR,    OAM_LF_H248_USERTIDINFO,            0x20,   oamGetH248UserTIDInfo,              oamSetH248UserTIDInfo},
    {"H.248RTPTIDConfig",            OBJ_BIT_ONU,  OAM_EXT_ATTR,    OAM_LF_H248_RTP_TIDCONFIG,          0x1B,   NULL,                               oamSetH248RTPTIDConfig},
    {"H.248RTPTIDInfo",              OBJ_BIT_ONU,  OAM_EXT_ATTR,    OAM_LF_H248_RTP_TIDINFO,            0x21,   oamGetH248RTPTIDInfo,               NULL},
    {"H.248IADOperationStatus",      OBJ_BIT_ONU,  OAM_EXT_ATTR,    OAM_LF_H248IADOPERATIONSTATUS,      0x04,   oamGetH248IADOperationStatus,       NULL},
#endif
*/  
    {"acPhyAdminControl",            OBJ_BIT_PORT,  OAM_STD_ACT,    OAM_LF_PHYADMINCONTROL,             0x04,   NULL,   oamSetPhyAdminControl},
    {"acAutoNegRestartAutoConfig",   OBJ_BIT_PORT,  OAM_STD_ACT,    OAM_LF_AUTONEGRESTARTAUTOCONFIG,    0x00,   NULL,   oamSetAutoNegRestartAutoConfig},
    {"acAutoNegAdminControl",        OBJ_BIT_PORT,  OAM_STD_ACT,    OAM_LF_AUTONEGADMINCONTROL,         0x04,   NULL,   oamSetAutoNegAdminControl},
    
    {"ResetOnu",                     OBJ_BIT_ONU,  OAM_EXT_ACT, OAM_LF_RESETONU,                0x00,   NULL,   oamSetResetOnu},
    {"SleepControl",                 OBJ_BIT_ONU,  OAM_EXT_ACT, OAM_LF_SLEEPCONTROL,            0x0E,   NULL,   oamSetSleepControl},
    {"acMultiLLIDAdminControl",      OBJ_BIT_ONU,  OAM_EXT_ACT, OAM_LF_MULTILLIDADMINCONTROL,   0x04,   NULL,   oamSetMultiLlidAdminControl},
    {"acFastLeaveAdminControl",      OBJ_BIT_ONU,  OAM_EXT_ACT, OAM_LF_FASTLEAVEADMINCONTROL,   0x04,   NULL,   oamSetFastLeaveAdminControl},
#ifdef TCSUPPORT_VOIP
    {"IADOperation",                 OBJ_BIT_ONU,  OAM_EXT_ACT, OAM_LF_IADOPERATION,            0x04,   NULL,   oamSetIADOperation},
    {"SIPDigitMap",                  OBJ_BIT_ONU,  OAM_EXT_ACT, OAM_LF_SIPDIGITMAP,             0xFF,   NULL,   oamSetSIPDigitMap},
#endif
    {"", 0, 0, 0, 0, NULL, NULL}
};


/*----------------------------------------------------------------------------*/

OamParamNode_Ptr findOamParamNodeByBranchLeaf(u_char branch, u_short leaf){
    int i;
    OamParamNode_Ptr opnp = oamParamTable;
    int num = sizeof(oamParamTable) / sizeof(oamParamTable[0]);
#ifdef TCSUPPORT_EPON_OAM_CUC
    if (gCtcHandler.usedVerion[0] >= 0xc1){
        return findCucOamNodeByBranchLeaf(branch, leaf);
    }
#endif
    for (i = 0; i < num; ++i, ++opnp){
        if ( (leaf == opnp->leaf) &&(branch == opnp->branch))
            return opnp;
    }
    return NULL;
}


inline int isOamAttribActBranch(u_char branch)
{
    return ((branch == OAM_STD_ATTR) ||
            (branch == OAM_EXT_ATTR) ||
            (branch == OAM_STD_ACT)  ||
            (branch == OAM_EXT_ACT) );
}

inline int isOamObjBranch(u_char branch)
{
    return ((branch == OAM_OBJECT_BRANCH_V1) ||
            (branch == OAM_OBJECT_BRANCH_V2) );
}

inline int isNeedObject(OamParamNode_Ptr opnp)
{
    return !(opnp->objectType&OBJ_BIT_ONU);
}

/* detect the OamParamNode_Ptr is the OamObject_Ptr's. */
inline int isObjectBranchLeaf(OamObject_Ptr oop, OamParamNode_Ptr opnp)
{
    switch(oop->leaf){
        case OBJ_LF_PORT:
            if (opnp->objectType & OBJ_BIT_PORT)
                return TRUE;
        break;
        case OBJ_LF_LLID:
            if (opnp->objectType & OBJ_BIT_LLID)
                return TRUE;
        break;
        case OBJ_LF_PONIF:
            if (opnp->objectType & OBJ_BIT_PONIF)
                return TRUE;
        break;
    }
    return FALSE;
}
int isOamPortAll(OamObject_Ptr oop)
{
    if (   (oop->leaf == OBJ_LF_PORT) 
       && (    ( (oop->branch == OAM_OBJECT_BRANCH_V1 ) && (oop->index == GET_PORT_ALL_V1))
             || ( (oop->branch == OAM_OBJECT_BRANCH_V2) && ((oop->index&0xFFFF) ==GET_PORT_ALL_V2) )
            )
     )
        return TRUE;
    return FALSE;
}


char nodeObjName[25] = {0};
inline char* nodeTypeName(u_char type)
{
    if ((type & OBJ_BIT_ALL) == OBJ_BIT_ALL){
        strcpy(nodeObjName, "ONU|PORT|PONIF|LLID");
    }else if (type & OBJ_BIT_PORT){
        if (type & OBJ_BIT_PONIF)
            strcpy(nodeObjName, "PORT|PONIF");
        else
            strcpy(nodeObjName, "PORT"); 
    }else if (type & OBJ_BIT_PONIF){
        strcpy(nodeObjName, "PONIF");
    }else if (type & OBJ_BIT_LLID){
        strcpy(nodeObjName, "LLID");
    }else if (type & OBJ_BIT_ONU){
        strcpy(nodeObjName, "ONU");
    }else{
        strcpy(nodeObjName, "UNKOWN");
    }
    return nodeObjName;
}
inline char* objTypeName(OamObject_Ptr pObj)
{
    if (pObj->branch == OAM_OBJECT_BRANCH_V1){
        if (pObj->index == OBJ_LF_PON_START_V1)
            strcpy(nodeObjName, "PON IF");
        else if (pObj->index < OBJ_LF_VOIP_START_V1)
            strcpy(nodeObjName, "PORT_ETHER");
        else if (pObj->index < OBJ_LF_E1_START_V1)
            strcpy(nodeObjName, "PORT_VOIP");
        else if (pObj->index < OBJ_LF_OTHER_START_V1)
            strcpy(nodeObjName, "PORT_E1");
        else if (pObj->index == OBJ_LF_ALL_START_V1)
            strcpy(nodeObjName, "PORT_ALL");
        else
            strcpy(nodeObjName, "PORT_OTHER");
    }else if (pObj->branch == OAM_OBJECT_BRANCH_V2){
        if (pObj->leaf == OBJ_LF_PORT){
            u_char type = pObj->index >> 24;
            u_short index = pObj->index;
            switch(type){
                case 1:
                    if (index == 0xffff)
                        strcpy(nodeObjName, "PORT_ETHER_ALL");
                    else
                        strcpy(nodeObjName, "PORT_ETHER");
                    break;
                case 2:
                    strcpy(nodeObjName, "PORT_VOIP");
                    break;
                case 3:
                    strcpy(nodeObjName, "PORT_ADSL2+");
                    break;
                case 4:
                    strcpy(nodeObjName, "PORT_VDSL2");
                    break;
                case 5:
                    strcpy(nodeObjName, "PORT_E1");
                    break;
                default:
                    strcpy(nodeObjName, "PORT_OTHER");
            }
        }else if (pObj->leaf == OBJ_LF_LLID){
            strcpy(nodeObjName, "LLID");
        }else if (pObj->leaf == OBJ_LF_PONIF){
            strcpy(nodeObjName, "PON IF");
        }else
            strcpy(nodeObjName,"Unknow");
    }
    return nodeObjName;
}

/* CTC EPON Spec : Port Type define */
inline u_char sysObjType(OamObject_Ptr pObj)
{
    if (pObj->branch == OAM_OBJECT_BRANCH_V1){
        if (pObj->index == OBJ_LF_PON_START_V1)
            return PORT_PON;
        else if (pObj->index < OBJ_LF_VOIP_START_V1)
            return PORT_ETHER;
        else if (pObj->index < OBJ_LF_E1_START_V1)
            return PORT_VOIP;
        else if (pObj->index < OBJ_LF_OTHER_START_V1)
            return PORT_E1;
        else if (pObj->index == OBJ_LF_ALL_START_V1)
            return PORT_ALL;
        else
            return PORT_OTHER;
    }else if (pObj->branch == OAM_OBJECT_BRANCH_V2){
        if (pObj->leaf == OBJ_LF_PORT){
            u_char type = pObj->index >> 24;
            if (type > 0 && type < 6)
                return type;
            return PORT_OTHER;
        }else if (pObj->leaf == OBJ_LF_LLID){
            return PORT_LLID;
        }else if (pObj->leaf == OBJ_LF_PONIF){
            return PORT_PON;
        }
    }
    return PORT_OTHER;
}


/*
 * This function must change with the system's port num changed.
 */
u_char sysObjIdx(OamObject_Ptr pObj, u_char objType)
{
    int idx = pObj->index;
    if (pObj->branch == OAM_OBJECT_BRANCH_V1){
        switch(objType){
            case PORT_PON:
                if (idx == OBJ_LF_PON_START_V1)
                    return idx;
            break;
            case PORT_ETHER:
                if (idx >= OBJ_LF_ETH_START_V1 && idx < OBJ_LF_ETH_START_V1+gCtcDB.dev_info.lan_num)
                    return idx - OBJ_LF_ETH_START_V1;
            break;
            case PORT_VOIP:
                if (idx >= OBJ_LF_VOIP_START_V1 && idx < OBJ_LF_VOIP_START_V1+gCtcDB.dev_info.voip_num)
                    return idx - OBJ_LF_VOIP_START_V1;
            break;
        }
    }else if (pObj->branch == OAM_OBJECT_BRANCH_V2){
        if (pObj->leaf == OBJ_LF_PORT){
            if (objType == (idx >> 24)){
                u_char lidx = idx & 0x0000FFFF;
                switch(objType){
                    case PORT_ETHER:
                        if (lidx >= OBJ_LF_ETH_START_V2 && lidx < OBJ_LF_ETH_START_V2+gCtcDB.dev_info.lan_num)
                            return lidx - OBJ_LF_ETH_START_V2;
                    break;
                    case PORT_VOIP:
                        if (lidx >= OBJ_LF_VOIP_START_V2 && lidx < OBJ_LF_VOIP_START_V2+gCtcDB.dev_info.voip_num)
                            return lidx - OBJ_LF_VOIP_START_V2;
                    break;
                }
            }
        }else if (pObj->leaf == OBJ_LF_PONIF){
            if (objType == PORT_PON && idx < gCtcDB.dev_info.pon_if_num)
                return idx;
        }else if (pObj->leaf == OBJ_LF_LLID){
            if (objType == PORT_LLID && idx < gCtcDB.dev_info.max_llid_num)
                return idx;
        }
    }
    return PORT_ERROR;
}

/*----------------------------------------------------------------------------*/

void oamClearSelfConfig(){
    // reset alarm disable;
    oamDisableAllAlarm();
}

int oamResetSystemParam(u_char llid)
{
    // reset ponvlan 4 port to transparent mode;
    // reset xpon_igmp to snooping mode;
    // reset eponmap to disable;
    // reset fec to disable;
    // reset ether port enable, auto neg enable
    // reset hwnat rules
    int ethPortNum = gCtcDB.dev_info.lan_num, i;
    
    if (llid == 0){
#ifdef TCSUPPORT_XPON_IGMP
        /*igmp_set_fwdmode(0); // snooping mode
        igmp_clear_fwdentry();
        igmp_set_fastleave_ctc(0);
        for (i = 0; i < ethPortNum; i++){
            igmp_clear_portvlan(i+1);
            igmp_set_max_playgroup(i+1, 0); // don't limit
            igmp_set_tagstrip(i+1, TAG_STRIPED_NO);
        }*/
            igmp_set_epon_mode();
#endif
#ifdef TCSUPPORT_PMMGR
            reinitEponPmmgr();
#endif
#ifdef TCSUPPORT_PON_VLAN
        {
            pon_vlan_rule rule = {0};

        /* set cpu vlan port transport */
            rule.filter_outer_tpid = 0;
            rule.filter_outer_dei = 2;
            rule.filter_outer_pri = 8;
            rule.filter_outer_vid = 4096;
            rule.filter_inner_tpid = 0;
            rule.filter_inner_dei = 2;
            rule.filter_inner_pri = 8;
            rule.filter_inner_vid = 4096;
            rule.tag_num = 0;
            rule.filter_ethertype = 0;
            rule.treatment_method = 0; // transparent
            rule.rule_priority = 0;         
            // add none tag : transparent
            pon_add_vlan_rule(0, rule);
            // add 1 tag : transparent
            rule.tag_num = 1;
            pon_add_vlan_rule(0, rule);
            // add 2 tags : transparent
            rule.tag_num = 2;
            pon_add_vlan_rule(0, rule);
    
            pon_clean_vlan_rule(40);// clean default rule which is for gpon
            pon_clean_hwnat();
        }
#endif
#ifdef TCSUPPORT_EPON_MAPPING
            eponmapReset(); // disable
#endif
        for (i = 0; i < ethPortNum; i++){
        #ifdef TCSUPPORT_PON_VLAN
            eponClearVlanRules(i);
            eponSetVlanTransparent(i);
        #endif
        #ifdef TCSUPPORT_MT7530_SWITCH_API
            macMT7530SetPortActive(i, 1); // enable
            macMT7530PortAutoNegActive(i, 1); // enable
        #endif
        }
    }
    // disable fec
    eponApiSetLlidFec(llid, 0);
    
    return 0;
}

/*********************GET/SET function of every node********************/

/* --------------------------Port attributions and actions-------------------------- */
// #OK
u_char oamGetPhyAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u8 portState = 0;
	int  ret = 0;
	int  stateValue = 0;
    u_char portId = sysObjIdx(pObj, PORT_ETHER);

    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;
	
    ret = macMT7530GetPortAdmin(&portState,portId);
    if (ret == -1){
        return VAR_NO_RESOURCE;
    }
    *pLength = 4;
	if(portState == POWER_ON){
		portState = ENABLED;
	}
	else if(portState == POWER_DOWN){
		portState = DISABLED;
	}
	stateValue = portState;
    setU32(pValue, stateValue);
    return SUCCESS;
}
// #OK
u_char oamGetAutoNegAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{   
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    int  portAutoNegState;

    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    portAutoNegState = macMT7530GetPortAutoNegState(portId);// return -1:error , 0:disable , 1:enable

    if (portAutoNegState == FAIL){
        return VAR_NO_RESOURCE;
    }
    *pLength = 4;
    setU32(pValue, portAutoNegState+1); // 1:disable; 2:enable
    return SUCCESS;
}

//@TODO
u_char oamGetAutoNegLocalTechAbility(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_char cpu = gCtcDB.cpu_type, num = 4;

    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    OamAutoNegLocalTechAbility_Ptr pAutoNegAblty = (OamAutoNegLocalTechAbility_Ptr)pValue;

    pAutoNegAblty->enumTable[0] = htonl(NEG_10BASE_T);
    pAutoNegAblty->enumTable[1] = htonl(NEG_10BASE_TFD);
    pAutoNegAblty->enumTable[2] = htonl(NEG_100BASE_T2);
    pAutoNegAblty->enumTable[3] = htonl(NEG_100BASE_T2FD);

    if ((isMT7525G(cpu) || isMT7520G(cpu) || isMT7520S(cpu))
        || ((isMT7525F(cpu) || isMT7520F(cpu)) && (portId == gCtcDB.dev_info.lan_num-1))){
        
        pAutoNegAblty->enumTable[4] = htonl(NEG_1000BASE_T);
        pAutoNegAblty->enumTable[5] = htonl(NEG_1000BASE_TFD);
        num = 6;
    }
    pAutoNegAblty->enumNum      = htonl(num);

    *pLength = 4+4 * num;

    return SUCCESS;
}

//@TODO
u_char oamGetAutoNegAdvTechAbility(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_char cpu = gCtcDB.cpu_type, num = 4;

    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    OamAutoNegLocalTechAbility_Ptr pAutoNegAblty = (OamAutoNegLocalTechAbility_Ptr)pValue;

    pAutoNegAblty->enumTable[0] = htonl(NEG_10BASE_T);
    pAutoNegAblty->enumTable[1] = htonl(NEG_10BASE_TFD);
    pAutoNegAblty->enumTable[2] = htonl(NEG_100BASE_T2);
    pAutoNegAblty->enumTable[3] = htonl(NEG_100BASE_T2FD);

    if ((isMT7525G(cpu) || isMT7520G(cpu) || isMT7520S(cpu))
        || ((isMT7525F(cpu) || isMT7520F(cpu)) && (portId == gCtcDB.dev_info.lan_num-1))){
        
        pAutoNegAblty->enumTable[4] = htonl(NEG_1000BASE_T);
        pAutoNegAblty->enumTable[5] = htonl(NEG_1000BASE_TFD);
        num = 6;
    }
    pAutoNegAblty->enumNum      = htonl(num);

    *pLength = 4+4 * num;

    return SUCCESS;
}

// #OK
u_char oamGetEthPortPause(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    int   state;
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    state = macMT7530GetPortPause(portId);
    *pLength = 1;
    if (state != FAIL)
        setU8(pValue, state);
    else
        return VAR_NO_RESOURCE;
    return SUCCESS;
}
// #OK
u_char oamSetEthPortPause(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_char state  = GetU8(pValue);
    int   ret;
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    switch(state){
        case DISACTIVE:
        case ACTIVE:
        ret = macMT7530SetPortPause(portId, state);
        gCtcDB.port[portId].flow_ctrl_en = state;
        break;
        default:
        return RET_ONLY_SKIP;
    }

    if (ret == FAIL)
        return VAR_NO_RESOURCE;

    return SET_OK;
}

// #OK
u_char oamGetEthLinkState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
#define ETH_PORT_DISABLED       0
#define ETH_PORT_LISTENING      1
#define ETH_PORT_LEARNING       2
#define ETH_PORT_FORWARDING 3
#define ETH_PORT_BLOCKING       4
#define ETH_PORT_LINKDOWN       5
#define ETH_PORT_STPOFF     6

#define ETH_LINK_DOWN 0
#define ETH_LINK_UP      1
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    int   state;

    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    #ifndef TCSUPPORT_MT7530_SWITCH_API 
    state = macMT7530GetPortEthLinkState(portId);
    #else
    state = macMT7530GetPortStatus(portId);
    #endif
    
    if (state == FAIL)
        return VAR_NO_RESOURCE;
    else if (state == ETH_PORT_DISABLED || state == ETH_PORT_LINKDOWN || 
             state == ETH_PORT_STPOFF || state == ETH_PORT_BLOCKING)
        state = ETH_LINK_DOWN;
    else
        state = ETH_LINK_UP;
    
    setU8(pValue, state);

    *pLength = 1;
    return SUCCESS;
}

// #OK
u_char oamGetEthPortUSPolicing(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    int   ret;
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    ret = macMT7530GetPortPolicingEnable(portId);

    if (ret == FAIL)
        return VAR_NO_RESOURCE;
    else if (ret == DISACTIVE){
        setU8(pValue, DISACTIVE);
        *pLength = 1;       
    }else if (ret == ACTIVE){
        u_int cir, cbs, ebs;
        OamEthPortUSPolicing_Ptr oepup = (OamEthPortUSPolicing_Ptr)pValue;

        ret = macMT7530GetPortPolicing(portId, &cir, &cbs, &ebs);
        if (ret == FAIL)
            return VAR_NO_RESOURCE;

        oepup->actived = ACTIVE;
        *pLength = sizeof(OamEthPortUSPolicing_t);

        oepup->portCIR[0] = (u_char)(cir>>16);
        oepup->portCIR[1] = (u_char)(cir>>8);
        oepup->portCIR[2] = (u_char)cir;

        oepup->portCBS[0] = (u_char)(cbs>>16);
        oepup->portCBS[1] = (u_char)(cbs>>8);
        oepup->portCBS[2] = (u_char)cbs;

        oepup->portEBS[0] = (u_char)(ebs>>16);
        oepup->portEBS[1] = (u_char)(ebs>>8);
        oepup->portEBS[2] = (u_char)ebs;
    }
    return SUCCESS;
}
// #OK
u_char oamSetEthPortUSPolicing(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char enable, portId = sysObjIdx(pObj, PORT_ETHER);
    int   ret;
    u_int cir, cbs, ebs;


    if ((portId == PORT_ERROR) || (length < 1))
        return VAR_BAD_PARAMETERS;

    enable = GetU8(pValue);

    if (enable == DISACTIVE){
        ret = macMT7530SetPortPolicingEnable(portId, DISACTIVE);
    }else if (enable == ACTIVE){
        if (length < sizeof(OamEthPortUSPolicing_t))
            return VAR_BAD_PARAMETERS;
        
        ret = macMT7530SetPortPolicingEnable(portId, ACTIVE);
    }else{
        return RET_ONLY_SKIP;  // only skip
    }

    if (FAIL == ret)
        return VAR_NO_RESOURCE;

    gCtcDB.port[portId].us_policing_en = enable;
    if (enable == ACTIVE){
        OamEthPortUSPolicing_Ptr oepup = (OamEthPortUSPolicing_Ptr)pValue;

        cir  = oepup->portCIR[2] + (oepup->portCIR[1]<<8) + (oepup->portCIR[0]<<16);
        cbs = oepup->portCBS[2] + (oepup->portCBS[1]<<8) + (oepup->portCBS[0]<<16);
        ebs = oepup->portEBS[2] + (oepup->portEBS[1]<<8) + (oepup->portEBS[0]<<16);

        ret = macMT7530SetPortPolicing(portId, cir, cbs, ebs);
        if (ret == FAIL)
            return VAR_NO_RESOURCE;

        gCtcDB.port[portId].us_cir = cir;
        gCtcDB.port[portId].us_cbs = cbs;
        gCtcDB.port[portId].us_ebs = ebs;
    }

    return SET_OK;
}

// #OK
u_char oamGetEthPortDSRateLimiting(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    int   ret;
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    ret = macMT7530GetPortRatelimitingEnable(portId);

    if (ret == FAIL){
        return VAR_NO_RESOURCE;
    }else if (ret == DISACTIVE){
        setU8(pValue, DISACTIVE);
        *pLength = 1;
    }else if (ret == ACTIVE){
        u_int cir, pir;
        OamEthPortDSRateLimiting_Ptr oepdrlp = (OamEthPortDSRateLimiting_Ptr)pValue;

        ret = macMT7530GetPortRatelimiting(portId, &cir, &pir);
        if (FAIL == ret){
            return VAR_NO_RESOURCE;
        }

        *pLength = sizeof(OamEthPortDSRateLimiting_t);
        oepdrlp->actived = ACTIVE;
        oepdrlp->portCIR[0] = (u_char)(cir>>16);
        oepdrlp->portCIR[1] = (u_char)(cir>>8);
        oepdrlp->portCIR[2] = (u_char)cir;

        oepdrlp->portPIR[0] = (u_char)(pir>>16);
        oepdrlp->portPIR[1] = (u_char)(pir>>8);
        oepdrlp->portPIR[2] = (u_char)pir;
    }

    return SUCCESS;
}
// #OK
u_char oamSetEthPortDSRateLimiting(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char enable, portId = sysObjIdx(pObj, PORT_ETHER);
    u_int cir, pir;
    int   ret;
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    if (length < 1)
        return VAR_BAD_PARAMETERS;

    enable = GetU8(pValue);

    if (enable == DISACTIVE){
        ret = macMT7530SetPortRatelimitingEnable(portId, DISACTIVE);
    }else if (enable == ACTIVE){
        if (length < sizeof(OamEthPortDSRateLimiting_t))
            return VAR_BAD_PARAMETERS;
        
        ret = macMT7530SetPortRatelimitingEnable(portId, ACTIVE);
    }else{
        return RET_ONLY_SKIP;  // only skip
    }

    if (FAIL == ret)
        return VAR_NO_RESOURCE;

    gCtcDB.port[portId].ds_ratelimit_en = enable;
    if (enable == ACTIVE){
        OamEthPortDSRateLimiting_Ptr oepdrlp = (OamEthPortDSRateLimiting_Ptr)pValue;

        cir = oepdrlp->portCIR[2] + (oepdrlp->portCIR[1]<<8) + (oepdrlp->portCIR[0]<<16);
        pir = oepdrlp->portPIR[2] + (oepdrlp->portPIR[1]<<8) + (oepdrlp->portPIR[0]<<16);

        ret = macMT7530SetPortRatelimiting(portId, cir, pir);
        if (ret == FAIL)
            return VAR_NO_RESOURCE;
        gCtcDB.port[portId].ds_cir = cir;
        gCtcDB.port[portId].ds_pir = pir;
    }

    return SET_OK;
}

// #OK
u_char oamSetPortLoopDetect(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    //TODO: not only ether port
    int   ret    = 0;
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_int  active = GetU32(pValue);
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    if (active == DISABLED){
        ret = macMT7530SetPortLoopDetect(portId, DISACTIVE);
        // == STP off per port
    }else if (active == ENABLED){
        ret = macMT7530SetPortLoopDetect(portId, ACTIVE);
        // == STP on per port
    }else{
        return RET_ONLY_SKIP;
    }

#if !defined(TCSUPPORT_CT_PON_GD)
    if (ret == FAIL)
        return VAR_NO_RESOURCE;
#endif

    gCtcDB.port[portId].loopdetect = active-1;

    return SET_OK;
}

u_int  gPortLoopedStatus = 0; 
u_int  gPortLoopedPortId = 0; 
int doOamSetPortDisableLooped(){
	int   ret	 = 0;

	if (gPortLoopedPortId == PORT_ERROR)
	   return VAR_BAD_PARAMETERS;

	if (gPortLoopedStatus == DISACTIVE){
	   ret = macMT7530SetPortDisableLooped(gPortLoopedPortId, DISACTIVE);
	}else if (gPortLoopedStatus == ACTIVE){
	   ret = macMT7530SetPortDisableLooped(gPortLoopedPortId, ACTIVE);
	}else{
	   return RET_ONLY_SKIP;
	}

	if (ret == FAIL)
	   return VAR_NO_RESOURCE;

	return 0;
}

// #OK
u_char oamSetPortDisableLooped(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    //TODO: not only ether port
    int   ret    = 0;
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_int  active = GetU32(pValue);
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

#if 1
	gPortLoopedStatus = active;
	gPortLoopedPortId = portId;
	postDBSetSyncMsg(DB_SYNC_CTC_PORT_LOOPED_CONFIG);
#else
    if (active == DISACTIVE){
        ret = macMT7530SetPortDisableLooped(portId, DISACTIVE);
    }else if (active == ACTIVE){
        ret = macMT7530SetPortDisableLooped(portId, ACTIVE);
    }else{
        return RET_ONLY_SKIP;
    }

    if (ret == FAIL)
        return VAR_NO_RESOURCE;
#endif

    gCtcDB.port[portId].disable_looped = active;

    return SET_OK;
}

// #OK
u_char oamSetPhyAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    int   ret       = 0;
    u_char portId    = sysObjIdx(pObj, PORT_ETHER);
    u_int  portState = GetU32(pValue);
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    if (portState == ENABLED){
		ret = macMT7530SetPortAdmin(POWER_ON,portId);
    }else if (portState == DISABLED){
		ret = macMT7530SetPortAdmin(POWER_DOWN,portId);
    }else{
        return RET_ONLY_SKIP;// OR skip only?
    }
    if (ret == FAIL)
        return VAR_NO_RESOURCE;

    gCtcDB.port[portId].phy_ctrl_en = portState-1;

    return SET_OK;
}

// #OK
u_char oamSetAutoNegRestartAutoConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;
    
    if (macMT7530PortAutoNegRestartAutoConfig(portId) == FAIL)
    {
        return VAR_NO_RESOURCE;
    }
    return SET_OK;
}

// #OK
u_char oamSetAutoNegAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    int   ret    = 0;
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_int  active = GetU32(pValue);
    u_char *speed = NULL;
	u_char *duplex = NULL;
	
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    if (active == DISABLED){
        ret = macMT7530PortAutoNegActive(portId, DISACTIVE);
		speed = "100";
		duplex = "Full";
		if (SUCCESS != macMT7530SetMaxBitRate(portId, speed))
			return VAR_NO_RESOURCE;
		if (SUCCESS != macMT7530SetDuplexMode(portId, duplex))
			return VAR_NO_RESOURCE;
    }else if (active == ENABLED){
        ret = macMT7530PortAutoNegActive(portId, ACTIVE);
    }else{
        return RET_ONLY_SKIP;
    }

    if (ret == FAIL)
        return VAR_NO_RESOURCE;

    gCtcDB.port[portId].auto_neg_en = active-1;

    return SET_OK;
}
/* --------------------------Port attributions and actions end---------------------- */


/* --------------------------ONU attributions and actions--------------------------- */
// #OK
u_char oamGetFecAbility(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char fecAbility = 1;// unknown, the onu is initializing.

    fecAbility = gCtcDB.dev_info.fec_ability;
    *pLength = 4;
    setU32(pValue, fecAbility);
    return SUCCESS;
}

// #OK
u_char oamGetFecMode(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    int fecMode = 1;// unknown, the onu is initializing.

    fecMode = eponApiGetLlidFec( llidIdx);
    if (fecMode == 0){
        fecMode = 3; //disable
    }else if (fecMode == 1){
        fecMode = 2; // enable
    }else {
        return VAR_NO_RESOURCE;
    }
    
    *pLength = 4;
    setU32(pValue, fecMode);
    return SUCCESS;
}
// #OK
u_char oamSetFecMode(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_int fecMode = GetU32(pValue);
    
    if (fecMode == 2){ // enable fec
        eponApiSetLlidFec(llidIdx, 1);
        gCtcDB.llid[llidIdx].fec_en = 1;
    }else if (fecMode == 3){ // disable fec
        eponApiSetLlidFec(llidIdx, 0);
        gCtcDB.llid[llidIdx].fec_en = 0;
    }else
        return VAR_BAD_PARAMETERS;

    return SET_OK;
}

// #OK
u_char oamGetOnuSN(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamOnuSN_Ptr oosp = (OamOnuSN_Ptr)pValue;

    memcpy(oosp->vendorID, gCtcDB.dev_info.vendor_id, 4);
    memcpy(oosp->onuModel, gCtcDB.dev_info.model, 4);

    memcpy(oosp->onuID, eponOam.eponOamLlid[0].macAddr, 6);

    memcpy(oosp->hwVersion, gCtcDB.dev_info.hw_ver, 8);
    memcpy(oosp->swVersion, gCtcDB.dev_info.sw_ver, 16);

    *pLength = sizeof(OamOnuSN_t);
    
    // if the onu support ctc version V3.0 must add attr ExtModel (except CU version 0xC1 & 0xC2)
    if (gCtcHandler.usedVerion[llidIdx] >= 0x30 && gCtcHandler.usedVerion[llidIdx] < 0xc1){
        eponOamExtDbg(DBG_OAM_L4, "Current Use Version : 0x%2X\n", gCtcHandler.usedVerion[llidIdx]);
        memcpy((void *)(pValue+sizeof(OamOnuSN_t)), gCtcDB.dev_info.ext_model, 16);
        *pLength += 16;
    }
    return SUCCESS;
}
// #OK
u_char oamGetFirmwareVer(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char *ptr = pValue;
    int len = strlen(gCtcDB.dev_info.fw_ver);

    len = MIN(len, 127);

    strncpy((char*)ptr, gCtcDB.dev_info.fw_ver, len);

    *pLength = len;
    return SUCCESS;
}

u_char oamGetOnuCapabilities1(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    int i ;
    OamOnuCapabilities_1_Ptr oocp = (OamOnuCapabilities_1_Ptr)pValue;

    oocp->GEPortNum = gCtcDB.dev_info.ge_num;
    memset(oocp->GEPortBitMap, 0, 8);
    for (i = 0; i < oocp->GEPortNum; i++){
        oocp->GEPortBitMap[7-(i/8)] |= 0x01<<(i%8);
    }

    oocp->FEPortNum = gCtcDB.dev_info.fe_num;
    memset(oocp->FEPortBitMap, 0, 8);
    for (i = 0; i < oocp->FEPortNum; i++){
        oocp->FEPortBitMap[7-(i/8)] |= 0x01<<(i%8);
    }

#ifndef TCSUPPORT_VOIP
    oocp->POTSPortNum = 0;
#else
    oocp->POTSPortNum = gCtcDB.dev_info.voip_num;
#endif

    oocp->E1PortNum = gCtcDB.dev_info.tdm_num;
    
    oocp->USQueuesNum = gCtcDB.dev_info.us_queue_num;
    oocp->queueMaxPerPortUS = gCtcDB.dev_info.us_queue_max_per_port;
    oocp->DSQueuesNum = gCtcDB.dev_info.ds_queue_num;
    oocp->queueMaxPerPortDS = gCtcDB.dev_info.ds_queue_max_per_port;

    oocp->batteryBackup = gCtcDB.dev_info.battery_backup;

    oocp->serviceSupported  = 0;
    if (oocp->GEPortNum > 0)
        oocp->serviceSupported |= SUPPORT_ETH_GE;
    if (oocp->FEPortNum > 0)
        oocp->serviceSupported |= SUPPORT_ETH_FE;
    if (oocp->POTSPortNum > 0)
        oocp->serviceSupported |= SUPPORT_VOIP;
    if (oocp->E1PortNum > 0)
        oocp->serviceSupported |= SUPPORT_TDMCES;

    *pLength = sizeof(OamOnuCapabilities_1_t);

    eponOamExtDbg(DBG_OAM_L2, ">> %s CTC Used Version: %02x\n",__FUNCTION__, gCtcHandler.usedVerion[llidIdx]);
    if (0){// (gCtcHandler.usedVerion[llidIdx] == 0x21){
        u_char  llidSrvQueue = 0, llidNum = 1, srvQ = 1;
        
        *(pValue + *pLength) = gCtcDB.dev_info.adsl_num;
        *(pValue + *pLength + 1) = gCtcDB.dev_info.vdsl_num;
        
        llidNum = gCtcDB.dev_info.max_llid_num;
        if (llidNum == 1){
            llidSrvQueue |= (1<<7); // support ServiceDBA
            srvQ = gCtcDB.dev_info.service_sla_num;
            llidSrvQueue |= srvQ;
        }else{
            llidSrvQueue |= llidNum;
        }
        *(pValue + *pLength + 2) = llidSrvQueue;

        *pLength += 3;
    }
    
    return SUCCESS;
}
#if 0
void oamModifyOnutype(OamOnuCapabilities_2_Ptr oocp)
{
	char buf[48] = {0};
	memset(buf, 0, sizeof(buf));
	if( 0 == tcapi_get("Oam_CTC", "onutype", buf) ){
		if (0 == strcmp(buf, "hgu")){
			oocp->onuType = ONU_TYPE_HGU;
			tcdbg_printf("\n onutype report modify HGU!\n") ;
		}else{
			oocp->onuType = ONU_TYPE_SFU;
			tcdbg_printf("\n onutype report modify SFU!\n") ;
		}
	}
}
#endif
//@TODO: need to update these values with system
u_char oamGetOnuCapabilities2(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamOnuCapabilities_2_Ptr oocp = (OamOnuCapabilities_2_Ptr)pValue;
    u_char ifTypeNum;
    u_char *batteryBackup_p = NULL;
    u_char i, num = 0;
    u_short portNum;

    // if the onu support ctc version v2.0, should return 0x86
    if (gCtcHandler.usedVerion[llidIdx] == 0x20)
        return VAR_BAD_PARAMETERS;

    if (eponOamCfg.isHgu){
        oocp->onuType = ONU_TYPE_HGU;
    }else{
        oocp->onuType = ONU_TYPE_SFU;
    }
	/* Use cmd to config eponOamCfg.isHgu */
/*	oamModifyOnutype(oocp);*/

    oocp->multiLLD = gCtcDB.dev_info.max_llid_num;
    oocp->protectionType = gCtcDB.dev_info.protect_type;
    oocp->PONIFNum = gCtcDB.dev_info.pon_if_num;

    oocp->slotNum = 0;
    
    // GE/ FE/ VOIP/ USB/ WLAN/ ADSL2/ VDSL2/ TDM
    ifTypeNum = ONUGetActiveIfTypeNum(); 
    oocp->ifTypeNum = ifTypeNum;
    for(i = 0; i < IF_TYPE_MAX; i++){
        portNum = ONUGetActiveIfPortNum(i);
        if (portNum > 0){
            oocp->interfaces[num].interfaceType = htonl(i);
            oocp->interfaces[num++].portNum = htons(portNum);
        }
    }

    batteryBackup_p = pValue + sizeof(OamOnuCapabilities_2_t) + ifTypeNum * sizeof(Interface_t);
    *batteryBackup_p = gCtcDB.dev_info.battery_backup;

    *pLength = sizeof(OamOnuCapabilities_2_t) + ifTypeNum * sizeof(Interface_t) + 1;
    return SUCCESS;
}

// #OK
u_char oamGetOnuCapabilities3(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamOnuCapabilities_3_Ptr oocp = (OamOnuCapabilities_3_Ptr)pValue;

    oocp->IPv6Supported = gCtcDB.dev_info.ipv6_spt;
    oocp->OnuPowerSupplyCtrl = gCtcDB.dev_info.power_ctrl_type;
#ifndef TCSUPPORT_EPON_OAM_CUC
    oocp->serviceSLA = gCtcDB.dev_info.service_sla_num;
#endif
    *pLength = sizeof(OamOnuCapabilities_3_t);
    
    return SUCCESS;
}

// #OK
u_char oamGetPowerSavingCapabilities(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamPowerSavingCapabilities_Ptr oocp = (OamPowerSavingCapabilities_Ptr)pValue;

    oocp->sleepMode = gCtcDB.dev_info.powersaving_cap;
    oocp->supportEarlyWake = gCtcDB.dev_info.powersaving_early_wakeup_spt;

    *pLength = sizeof(OamPowerSavingCapabilities_t);
    return SUCCESS;
}

// #OK
u_char oamGetPowerSavingConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamPowerSavingConfig_Ptr pscp = (OamPowerSavingConfig_Ptr)pValue;
    long long maxDuration = 0;
    
#if 1
    if (!gCtcDB.dev_info.powersaving_early_wakeup_spt)
        pscp->earlyWakeEnable = 0xFF; // not support
    else{
        if (!gCtcDB.cfg_info.powersaving_early_wakeup_en)
            pscp->earlyWakeEnable = 1;
        else
            pscp->earlyWakeEnable = 0;
    }

    memset(pscp->maxSleepDuration, 0, 6);
    maxDuration = gCtcDB.cfg_info.powersaving_max_duration;
    pscp->maxSleepDuration[0] = (maxDuration>>40) & 0xFF;
    pscp->maxSleepDuration[1] = (maxDuration>>32) & 0xFF;
    pscp->maxSleepDuration[2] = (maxDuration>>24) & 0xFF;
    pscp->maxSleepDuration[3] = (maxDuration>>16) & 0xFF;
    pscp->maxSleepDuration[4] = (maxDuration>>8)  & 0xFF;
    pscp->maxSleepDuration[5] = (maxDuration)       & 0xFF;
#else
    eponPsIoctl_t eponPsData;
    eponPsData.cmdType = EPON_PS_CMD_CFG_GET;
    if (ERROR == eponApiCtlPs(&eponPsData))
        return VAR_NO_RESOURCE;
    pscp->earlyWakeEnable = eponPsData.earlyWakeup;
    memcpy(pscp->maxSleepDuration, eponPsData.sleepDurationMax, 6);
#endif
    *pLength = 0x07;
    return SUCCESS;
}
// #OK
u_char oamSetPowerSavingConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    long long maxDuration = 0;
    OamPowerSavingConfig_Ptr pscp = (OamPowerSavingConfig_Ptr)pValue;
#ifdef TCSUPPORT_EPON_CTC_POWERSAVING
    eponPsIoctl_t eponPsData;
#endif

    if (pscp->earlyWakeEnable == 0){ // enabled
        gCtcDB.cfg_info.powersaving_early_wakeup_en = TRUE;
    }else if (pscp->earlyWakeEnable == 1){ // disabled
        gCtcDB.cfg_info.powersaving_early_wakeup_en = FALSE;
    }else if (pscp->earlyWakeEnable == 0xFF){ // not support early wake-up
        gCtcDB.cfg_info.powersaving_early_wakeup_en = FALSE;
    }else{
        return RET_ONLY_SKIP;
    }

    maxDuration |= ((long long)pscp->maxSleepDuration[0]) <<  40;
    maxDuration |= ((long long)pscp->maxSleepDuration[1]) <<  32;
    maxDuration |= ((long long)pscp->maxSleepDuration[2]) <<  24;
    maxDuration |= ((long long)pscp->maxSleepDuration[3]) <<  16;
    maxDuration |= ((long long)pscp->maxSleepDuration[4]) <<  8;
    maxDuration |= ((long long)pscp->maxSleepDuration[5]);
    gCtcDB.cfg_info.powersaving_max_duration = maxDuration;

#ifdef TCSUPPORT_EPON_CTC_POWERSAVING
    eponPsData.cmdType = EPON_PS_CMD_CFG_SET;
    eponPsData.earlyWakeup = pscp->earlyWakeEnable;
    memcpy(eponPsData.sleepDurationMax, pscp->maxSleepDuration, 6);
    if (ERROR == eponApiCtlPs(&eponPsData))
        return VAR_NO_RESOURCE;
#endif

    return SET_OK;
}

// #OK
u_char oamGetPortMacAgingTime(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_int agingTime = 0x0000012C;

    if (FAIL == macMT7530GetAgeTime(&agingTime))
        return VAR_NO_RESOURCE;

    *pLength = 4;
    setU32(pValue, agingTime);
    return SUCCESS;
}
// #OK
u_char oamSetPortMacAgingTime(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_int agingTime = GetU32(pValue);

    if (FAIL == macMT7530SetAgeTime(agingTime))
        return VAR_NO_RESOURCE;
    
    gCtcDB.cfg_info.port_aging_time = agingTime;

    return SET_OK;
}
/* --------------------------ONU attributions and actions end----------------------- */


/* --------------------------PON attributions and actions--------------------------- */
//@TODO
u_char oamGetServiceSLA(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    char buf[20] = {0}, attr[40] = {0};
    u_char i = 0;
    OamServiceSLA_Ptr pSLA = (OamServiceSLA_Ptr)pValue;
    OamSLAQueueService_Ptr pQueue = pSLA->services;

    pSLA->operateDBA = gCtcDB.cfg_info.sla.enable;

    if (pSLA->operateDBA == DISACTIVE){
        *pLength = 1;
        return SUCCESS;
    }

    pSLA->bestEffortSchedulingScheme = gCtcDB.cfg_info.sla.sch_scheme;
    pSLA->highPriorityBoundary = gCtcDB.cfg_info.sla.high_pri_bound;
    pSLA->cycleLength = htonl(gCtcDB.cfg_info.sla.cycle_len);
    pSLA->servicesNum = gCtcDB.cfg_info.sla.srvs_num;

    for (; i < pSLA->servicesNum; ++i, pQueue++)
    {
        pQueue->queueNum = gCtcDB.cfg_info.sla.srvs[i].queue_id;
        pQueue->fixedPktSize = htons(gCtcDB.cfg_info.sla.srvs[i].fixed_pkt_size);
        pQueue->fixedBandwidth = htons(gCtcDB.cfg_info.sla.srvs[i].fixed_bw);
        pQueue->guaranteedBandwidth = htons(gCtcDB.cfg_info.sla.srvs[i].assured_bw);
        pQueue->bestEffortBandwidth = htons(gCtcDB.cfg_info.sla.srvs[i].best_bw);
        pQueue->WRRWeight = gCtcDB.cfg_info.sla.srvs[i].wrr_wight;
    }

    *pLength = sizeof(OamServiceSLA_t) + sizeof(OamSLAQueueService_t) * pSLA->servicesNum;
    return SUCCESS;
}
//@TODO
u_char oamSetServiceSLA(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    char buf[20] = {0}, attr[40] = {0};
    u_char i = 0;
    OamServiceSLA_Ptr pSLA = (OamServiceSLA_Ptr)pValue;
    OamSLAQueueService_Ptr pQueue = pSLA->services;
    int needLen = sizeof(OamServiceSLA_t) + sizeof(OamSLAQueueService_t) * pSLA->servicesNum;
#ifdef TCSUPPORT_EPON_MAPPING
    QueueWeight_t queueWeights[LLID_QUEUE_NUM_MAX];
#endif /*TCSUPPORT_EPON_MAPPING */
#ifdef TCSUPPORT_PONMGR
    unsigned short mask = 0;
    MACRO_CMD_STRUCT(PWanTrtcmConfig) setTrTcmParamEntry ;
    MACRO_CMD_STRUCT(PWanCongestConfig) setTrTcmModeEntry ;
#endif

    gCtcDB.cfg_info.sla.enable = pSLA->operateDBA;
    if (pSLA->operateDBA == DISACTIVE){
        
        #ifdef TCSUPPORT_PONMGR
        setTrTcmModeEntry.TrtcmMode = 0;
        mask = MASK_CONGEST_TRTCM_MODE;
        
        if(PonApi_PWanCongestConfig_Set(mask, &setTrTcmModeEntry) != EXEC_OK) {
            return VAR_NO_RESOURCE ;
        }
        #endif //TCSUPPORT_PONMGR
    
        /*for mapping rules exist, not clear qos rule.*/
        //system("echo pwan mapqos 0 > /proc/gpon/debug");
        return SET_OK;
    }

    if (length < needLen){
        return VAR_BAD_PARAMETERS;
    }

    mask = MASK_TRTCM_PARAMETERS;
    gCtcDB.cfg_info.sla.sch_scheme = pSLA->bestEffortSchedulingScheme;
    gCtcDB.cfg_info.sla.high_pri_bound = pSLA->highPriorityBoundary;
    gCtcDB.cfg_info.sla.cycle_len= ntohl(pSLA->cycleLength);
    gCtcDB.cfg_info.sla.srvs_num= pSLA->servicesNum;

    // clear old param
    for (i =0; i < 8; i++){
        setTrTcmParamEntry.Trtcm[0].TSIdx = i;
        setTrTcmParamEntry.Trtcm[0].CIRValue = 0 ; //(HW unit is 64Kbps)
        setTrTcmParamEntry.Trtcm[0].CBSUnit = 0; // (HW unit is 128Byte def)
        setTrTcmParamEntry.Trtcm[0].PIRValue = 0 ;  //(HW unit is 64Kbps)
        setTrTcmParamEntry.Trtcm[0].PBSUnit =  0;
        
        if(PonApi_PWanTrtcmConfig_Set(mask, &setTrTcmParamEntry) != EXEC_OK) {
            return VAR_NO_RESOURCE ;
        }
    }

    for (i=0; i < pSLA->servicesNum; ++i, pQueue++)
    {
         gCtcDB.cfg_info.sla.srvs[i].queue_id = pQueue->queueNum;
         gCtcDB.cfg_info.sla.srvs[i].fixed_pkt_size = ntohs(pQueue->fixedPktSize);
         gCtcDB.cfg_info.sla.srvs[i].fixed_bw = ntohs(pQueue->fixedBandwidth);
         gCtcDB.cfg_info.sla.srvs[i].assured_bw = ntohs(pQueue->guaranteedBandwidth);
         gCtcDB.cfg_info.sla.srvs[i].best_bw = ntohs(pQueue->bestEffortBandwidth);
         gCtcDB.cfg_info.sla.srvs[i].wrr_wight = pQueue->WRRWeight;

         setTrTcmParamEntry.Trtcm[0].TSIdx = pQueue->queueNum;
         setTrTcmParamEntry.Trtcm[0].CIRValue = gCtcDB.cfg_info.sla.srvs[i].fixed_bw + gCtcDB.cfg_info.sla.srvs[i].assured_bw;
         setTrTcmParamEntry.Trtcm[0].CBSUnit = 200*setTrTcmParamEntry.Trtcm[0].CIRValue/128;
         setTrTcmParamEntry.Trtcm[0].PIRValue = setTrTcmParamEntry.Trtcm[0].CIRValue + gCtcDB.cfg_info.sla.srvs[i].best_bw;
         setTrTcmParamEntry.Trtcm[0].PBSUnit = 400*setTrTcmParamEntry.Trtcm[0].PIRValue/128;
         if(PonApi_PWanTrtcmConfig_Set(mask, &setTrTcmParamEntry) != EXEC_OK) {
            return VAR_NO_RESOURCE;
         }

#ifdef TCSUPPORT_EPON_MAPPING
        queueWeights[i].priQueue = pQueue->queueNum;
        queueWeights[i].weight = pQueue->WRRWeight;
        //queueWeights[i].tse = 1;
        if (pSLA->bestEffortSchedulingScheme == 2 && pSLA->highPriorityBoundary<= pQueue->WRRWeight)
            queueWeights[i].weight = 0; // sp+wrr mode: weight exceed high boundary
#endif /*TCSUPPORT_EPON_MAPPING */
    }   

    setTrTcmModeEntry.TrtcmMode = 1;
    mask = MASK_CONGEST_TRTCM_MODE;
    
    if(PonApi_PWanCongestConfig_Set(mask, &setTrTcmModeEntry) != EXEC_OK) {
        return VAR_NO_RESOURCE ;
    }
    
    // need to process highPriorityBoundary
#ifdef TCSUPPORT_EPON_MAPPING
    if (FAIL == eponmapSetLlidQueueMapping(0, pSLA->servicesNum, queueWeights))
        return VAR_NO_RESOURCE;
#endif /*TCSUPPORT_EPON_MAPPING */

    system("echo pwan qos 1 > /proc/gpon/debug");
    return SET_OK;
}

#ifndef TCSUPPORT_EPON_MAPPING
u_char oamGetLlidQueueConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    return VAR_NO_RESOURCE;
}
u_char oamSetLlidQueueConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    return VAR_NO_RESOURCE;
}
#else
//@TODO
u_char oamGetLlidQueueConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    // if the system is multi llid , the object is llid.
    u_char i = 0, queueNum, llid;
    OamLLIDQueueConfig_Ptr pLQC = (OamLLIDQueueConfig_Ptr)pValue;
    QueueWeight_t queueWeights[LLID_QUEUE_NUM_MAX];
    
    if (pObj == NULL){
        //llid = 0;
        llid = llidIdx;
    }else{
        llid = sysObjIdx(pObj, PORT_LLID);
        if (PORT_ERROR == llid)
            return VAR_BAD_PARAMETERS;
        
        if (llidIdx > 0 && llid == 0) // ZTE C300 use each LLID cfg this node.
            llid = llidIdx;     
    }

    if (FAIL == eponmapGetLlidQueueMapping(llid, &queueNum, queueWeights))
        return VAR_NO_RESOURCE;

    if (queueNum == 0 || queueNum > LLID_QUEUE_NUM_MAX){
        return VAR_NO_RESOURCE;
    }
    pLQC->num = queueNum;

    for(; i < queueNum; i++){
        pLQC->entries[i].queueIdx = htons(queueWeights[i].priQueue);
        pLQC->entries[i].WRRWeight = htons(queueWeights[i].weight);
    }

    *pLength = queueNum * sizeof(OamLLIDQueueConfigEntry_t) + sizeof(OamLLIDQueueConfig_t);
    return SUCCESS;
}
//@TODO
u_char oamSetLlidQueueConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char i = 0, llid, queueNum;
    OamLLIDQueueConfig_Ptr pLQC = (OamLLIDQueueConfig_Ptr)pValue;
    QueueWeight_t queueWeights[LLID_QUEUE_NUM_MAX];

    if (length < 1)
        return VAR_BAD_PARAMETERS;

    if (pObj == NULL){
        //llid = 0;
        llid = llidIdx;
    }else{
    //2 TODO: must translate llid to llidindex
        llid = sysObjIdx(pObj, PORT_LLID);
        if (PORT_ERROR == llid)
            return VAR_BAD_PARAMETERS;

        if (llidIdx > 0 && llid == 0) // ZTE C300 use each LLID cfg this node.
            llid = llidIdx;
    }

    if (pLQC->num > LLID_QUEUE_NUM_MAX || pLQC->num < 1){
        return VAR_BAD_PARAMETERS;
    }

    if (length < (1+pLQC->num*sizeof(OamLLIDQueueConfigEntry_t)))
        return VAR_BAD_PARAMETERS;

    queueNum = pLQC->num;
    for (; i < queueNum; ++i){
        queueWeights[i].priQueue = ntohs(pLQC->entries[i].queueIdx);
        queueWeights[i].weight   = ntohs(pLQC->entries[i].WRRWeight);
        //queueWeights[i].tse = 0;
    }

    if (FAIL == eponmapSetLlidQueueMapping(llid, queueNum, queueWeights))
        return VAR_NO_RESOURCE;

    return SET_OK;
}
#endif
// #OK
u_char oamGetProtectionParameters(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    /*
    if (FAIL == eponApiGetMpcpTime(&TlosMAC)){
        return VAR_NO_RESOURCE;
    }
    */

    setU16(pValue, gCtcDB.cfg_info.pon_los_optical_time);
    setU16(pValue+2, gCtcDB.cfg_info.pon_los_mac_time);

    *pLength = 4;
    return SUCCESS;
}
// #OK
u_char oamSetProtectionParameters(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    OamProtectionParameters_Ptr ppp = (OamProtectionParameters_Ptr)pValue;
    
    ppp->TlosOptical = ntohs(ppp->TlosOptical);
    ppp->TlosMAC     = ntohs(ppp->TlosMAC);

    gCtcDB.cfg_info.pon_los_optical_time = ppp->TlosOptical;
    gCtcDB.cfg_info.pon_los_mac_time = ppp->TlosMAC;

    if (FAIL == eponApiSetOnuMpcpTime(ppp->TlosMAC)){
        return VAR_NO_RESOURCE;
    }

    return SET_OK;
}

// #OK
u_char oamGetChipsetID(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamChipsetID_Ptr ocip = (OamChipsetID_Ptr)pValue;

    ocip->vendorID  = htons(gCtcDB.dev_info.pon_vendor_id);
    ocip->chipModel = htons(gCtcDB.dev_info.pon_chip_model);
    ocip->revision  = gCtcDB.dev_info.pon_revision;

    memcpy(ocip->versionDate, gCtcDB.dev_info.pon_ver_date, 3);

    *pLength = sizeof(OamChipsetID_t);
    return SUCCESS;
}

// #OK
u_char oamGetOpticalTransceiverDiagnosis(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamOpticalTransceiverDiagnosis_Ptr ootdp = (OamOpticalTransceiverDiagnosis_Ptr)pValue;

    if (FAIL == epon_sys_get_transceiver_info(ootdp)){
        return VAR_NO_RESOURCE;
    }
    ootdp->transceiverTemperature = htons(ootdp->transceiverTemperature);
    ootdp->supplyVoltage = htons(ootdp->supplyVoltage);
    ootdp->txBiasCurrent = htons(ootdp->txBiasCurrent);
    ootdp->rxPower = htons(ootdp->rxPower);
    ootdp->txPower = htons(ootdp->txPower);

    *pLength = sizeof(OamOpticalTransceiverDiagnosis_t);

    return SUCCESS;
}

// #OK
u_char oamGetHoldoverConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamHoldoverConfig_t *pCfg = (OamHoldoverConfig_t*)pValue;

    pCfg->state = htonl((u_int)gCtcDB.cfg_info.pon_holdover_state);
    pCfg->time  = htonl(gCtcDB.cfg_info.pon_holdover_time);
    
    *pLength = sizeof(OamHoldoverConfig_t);

    return SUCCESS;
}
// #OK
u_char oamSetHoldoverConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    OamHoldoverConfig_t *pCfg = (OamHoldoverConfig_t*)pValue;

    pCfg->state = ntohl(pCfg->state);
    pCfg->time  = ntohl(pCfg->time);
    
    if (50 > pCfg->time || 1000 < pCfg->time){
        return VAR_BAD_PARAMETERS;
    }

    gCtcDB.cfg_info.pon_holdover_state = (u_char)pCfg->state;
    gCtcDB.cfg_info.pon_holdover_time = pCfg->time;
    
    if (pCfg->state == HOLDOVER_DISACTIVATED || pCfg->state == HOLDOVER_ACTIVATED){
        if (FAIL == eponApiSetOnuHoldoverCfg(pCfg->state-1, pCfg->time))
            return VAR_BAD_PARAMETERS;
    }

    return SET_OK;
}

// #OK
u_char oamGetActivePONIFAdminstate(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
#if 0
    char buf[3] = {0};

    if (TCAPI_SUCCESS != tcapi_get(EPON_PON, "ActivePONIF", buf)){
        return VAR_NO_RESOURCE;
    }
    setU8(pValue, atoi(buf));

    *pLength = 1;
    return SUCCESS;
#else
    return VAR_BAD_PARAMETERS;
#endif
}
// #OK
u_char oamSetActivePONIFAdminstate(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
#if 0
    char buf[3] = {0};
    u_char activePonNo = GetU8(pValue), ponIfNum = 1;

    if (TCAPI_SUCCESS == tcapi_get(EPON_PON, "PONIFNum", buf)){
        ponIfNum = atoi(buf);
    }

    if ((ponIfNum - 1) < activePonNo){
        return VAR_NO_RESOURCE;
    }

    sprintf(buf, "%d", activePonNo);
    if (TCAPI_SUCCESS != tcapi_set(EPON_PON, "ActivePONIF", buf)){
        return VAR_NO_RESOURCE;
    }

    return RET_NOT_REPLY;
#else
    return VAR_BAD_PARAMETERS;
#endif
}

// #OK
u_char oamSetONUTxPowerSupplyControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    OamOnuTxPowerSupplyControl_Ptr ptxPSC = (OamOnuTxPowerSupplyControl_Ptr)pValue;
    OamOnuTxPowerSupplyControlDraft_Ptr pTxPSCD = (OamOnuTxPowerSupplyControlDraft_Ptr)pValue;
    u_char onuID[6] = {0}, brcstID[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u_char *pID = NULL;
    u_int   action = 0;
    
    eponOamGetOnuLanMac((char*)onuID);

    if (length == 0x0A || length == 0x0E){
        pID = pTxPSCD->onuID;
        action = ntohl(pTxPSCD->action);
    }else if (length == 0x0C){
        pID = ptxPSC->onuID;
        action = ntohs(ptxPSC->action);
    }
	eponOamExtDbg(DBG_OAM_L1, "TxPowerControl: ONUID:%.2x%.2x%.2x%.2x%.2x%.2x, action=%u\n", 
		pID[0],pID[1],pID[2],pID[3],pID[4],pID[5], action);  

    if ((pID!=NULL) && ((0==memcmp(pID, onuID, 6)) || (0==memcmp(pID, brcstID, 6))))
    {
        eponApiSetPhyPower(action);
    }
    
    return RET_NOT_REPLY;
}
// #OK
u_char oamSetMultiLlidAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_int llidNum = GetU32(pValue);
    u_int llidEnableMask = 0, i;
    
    if (llidNum <= EPON_LLID_MAX_NUM){
        gCtcDB.cfg_info.cur_llid_en_num = llidNum;
        
        for (i = 0; i < llidNum;i++){
            llidEnableMask |= (1<<i);
        }
        if (FAIL == eponCmdSetLlidMask(llidEnableMask))
        
        return SET_OK;
    }

    return VAR_BAD_PARAMETERS;
}
/* --------------------------PON attributions and actions end----------------------- */


/* --------------------------VLAN attributions and actions--------------------------- */
//@TODO
#ifndef TCSUPPORT_PON_VLAN
u_char oamGetVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
u_char oamSetVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
#else
u_char oamGetVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_char *dp = pValue;
    u_char mode;
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    if (eponOamCfg.isHgu){
        mode = VLAN_MODE_TRANSPARENT;
    }else{         
           mode = gCtcDB.port[portId].vlan.mode;
           if (mode > VLAN_MODE_TRUNK)
                return VAR_NO_RESOURCE;
    }
    
    setU8(dp, mode);
    PTR_MOVE(dp, 1);

    if (mode == VLAN_MODE_TRANSPARENT){
        *pLength = 1;
    }else if (mode == VLAN_MODE_TAG){
        u_int def_tag;
        
        if (FAIL == eponGetDefVlanTag(portId, &def_tag))
            return VAR_NO_RESOURCE;
            
        setU32(dp, def_tag);
            *pLength = 5;
    }else if (mode == VLAN_MODE_TRANSLATION){
        int trans_num = 0, i =0;
        u_int def_tag;
        u_int old_tags[MAX_VLAN_RULE_NUM_PER_PORT], new_tags[MAX_VLAN_RULE_NUM_PER_PORT];

        if (FAIL == eponGetDefVlanTag(portId, &def_tag)){
            return VAR_NO_RESOURCE;
        }

        setU32(dp, def_tag);
        PTR_MOVE(dp, 4);

        if ((trans_num = eponGetTranslateVlanTag(portId, MAX_VLAN_RULE_NUM_PER_PORT, old_tags, new_tags)) == FAIL){
            return VAR_NO_RESOURCE;
        }

        for (; i < trans_num; i++){
            setU32(dp, old_tags[i]);
            setU32(dp+4, new_tags[i]);
            PTR_MOVE(dp, 8);
        }

        *pLength = 1 + 4 + 8 * trans_num;
    }else if (mode == VLAN_MODE_TRUNK){
        int trunk_num = 0, i =0;
        u_int def_tag;
        u_int trunk_tags[MAX_VLAN_RULE_NUM_PER_PORT];

        if (FAIL == eponGetDefVlanTag(portId, &def_tag)){
            return VAR_NO_RESOURCE;
        }

        setU32(dp, def_tag);
        PTR_MOVE(dp, 4);

        if ((trunk_num = eponGetTrunkVlanTag(portId, MAX_VLAN_RULE_NUM_PER_PORT, trunk_tags)) == FAIL){
            return VAR_NO_RESOURCE;
        }

        for (; i < trunk_num; i++){
            setU32(dp, trunk_tags[i]);
            PTR_MOVE(dp, 4);
        }

        *pLength = 1 + 4 + 4 * trunk_num;
    }else if (mode == VLAN_MODE_N1AGGREGATION){
        return VAR_NO_RESOURCE;
    }else{
        return VAR_NO_RESOURCE;
    }

    return SUCCESS;
}
//@TODO
u_char oamSetVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char *dp = pValue;
    u_char mode = GetU8(dp);
    u_char portId = sysObjIdx(pObj, PORT_ETHER);

#ifdef TCSUPPORT_EPON_OAM_CUC
    char node[30] = {0}, attr[30] = {0}, buf[256] = {0};
#endif

    if (eponOamCfg.isHgu){
        return SET_OK;
    }
    
    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

      if (mode <= VLAN_MODE_TRUNK){
        gCtcDB.port[portId].vlan.mode = mode;
      }else{
            return VAR_BAD_PARAMETERS;
      }
      
        // clear all pre vlan rules
      if (FAIL == eponClearVlanRules(portId))
            return VAR_NO_RESOURCE;

    if (mode == VLAN_MODE_TRANSPARENT){
        if (FAIL == eponSetVlanTransparent(portId))
            return VAR_NO_RESOURCE;
    }else if (mode == VLAN_MODE_TAG){
        u_int def_tag = GetU32(dp+1);
        
            if (FAIL == eponSetDefVlanTag(portId, def_tag))
                return VAR_NO_RESOURCE;

        gCtcDB.port[portId].vlan.def_tag = def_tag;
        
             eponBlockAllVlanTag(portId);
    }else if (mode == VLAN_MODE_TRANSLATION){
            u_int old_tag, new_tag;
            u_int def_tag = GetU32(dp+1);
            int i = 0;

            if (FAIL == eponSetDefVlanTag(portId, def_tag))
                return VAR_NO_RESOURCE;

        gCtcDB.port[portId].vlan.def_tag = def_tag;
        gCtcDB.port[portId].vlan.num = (length-5)/8;

            for (; i < (length-5)/8; i ++){
                old_tag = GetU32(dp+5+i*8);
                new_tag = GetU32(dp+5+i*8+4);
                if (FAIL == eponAddTranslateVlanTag(portId, old_tag, new_tag))
                    return VAR_NO_RESOURCE;
           if (i < OAM_VLAN_RULE_MAX_NUM){
            gCtcDB.port[portId].vlan.old_tags[i] = old_tag;
            gCtcDB.port[portId].vlan.new_tags[i] = new_tag;
           }
            }
    }else if (mode == VLAN_MODE_TRUNK){
        u_int trunk_tag;
        u_int def_tag = GetU32(dp+1);
        int i = 0;
        
        if (FAIL == eponSetDefVlanTag(portId, def_tag))
            return VAR_NO_RESOURCE;
        
      gCtcDB.port[portId].vlan.def_tag = def_tag;
      gCtcDB.port[portId].vlan.num = (length-5)/4;

        for (; i < (length-5)/4; i ++){
            trunk_tag = GetU32(dp+5+i*4);
            if (FAIL == eponAddTrunkVlanTag(portId,trunk_tag))
                return VAR_NO_RESOURCE;
        if (i < OAM_VLAN_RULE_MAX_NUM){
            gCtcDB.port[portId].vlan.old_tags[i] = trunk_tag;
        }
        }
    }else if (mode == VLAN_MODE_N1AGGREGATION){
        // we are not support N:1 Vlan now.
        return VAR_NO_RESOURCE;
    }
	
#ifdef TCSUPPORT_PON_VLAN
	pon_clean_hwnat();
#endif


    return SET_OK;
}
/* --------------------------VLAN attributions and actions end----------------------- */
#endif


/* --------------------------Qos attributions and actions---------------------------- */
#ifndef TCSUPPORT_EPON_MAPPING
u_char oamGetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
u_char oamSetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
#else
//@TODO
u_char oamGetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_char *dp = pValue;
    u_char matchNum, i, j;
    OamCM_Ptr pClsfy = (OamCM_Ptr)pValue;
    QosResult_t result;
    QosMatchRule_t matchs[MAX_FIELD_NUM];
    OamQosRule_Ptr pRule;
    OamQosRuleEntry4_Ptr pEntry4;
    OamQosRuleEntry6_Ptr pEntry6;
    int portRulesNum = -1;

    if (eponOamCfg.isHgu){
        return VAR_BAD_PARAMETERS;
    }

    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;
    
    portRulesNum = eponmapGetClsfyRulesNum(portId);
    if (portRulesNum == FAIL)
        return VAR_NO_RESOURCE;
    eponOamExtDbg(DBG_OAM_L4, "portRulesNum: %d\n", portRulesNum);

    pClsfy->action = ACTION_LIST;
    pClsfy->rulesNum = portRulesNum;
    dp += sizeof(OamCM_t);
    
    for (i = 0; i < portRulesNum; ++i){
        pRule = (OamQosRule_Ptr)dp;
        matchNum = MAX_FIELD_NUM;
        if (0 == eponmapGetClsfyRule(portId, i, &result, &matchNum, matchs)){
            pRule->precedence = result.precedence;
            pRule->queueMapped = result.queueMapped;
            pRule->etherPriorityMark = result.priority;
            pRule->entriesNum = matchNum;
            pRule->length = 3;

            dp += sizeof(OamQosRule_t);
            for (j = 0; j < matchNum; ++j){
                if (matchs[j].field < FIELD_IPVER){
                    pRule->length += sizeof(OamQosRuleEntry4_t);
                    pEntry4 = (OamQosRuleEntry4_Ptr)dp;
                    pEntry4->field = matchs[j].field;
                    pEntry4->validOperator = matchs[j].op;
                    
                    memset(pEntry4->matchs, 0, ETH_ALEN);
                    switch(pEntry4->field){
                        case FIELD_SMAC:
                        case FIELD_DMAC:
                            memcpy(pEntry4->matchs, matchs[j].mac, ETH_ALEN);
                        break;
                        case FIELD_PBIT:
                        case FIELD_IPPROTO4:
                            pEntry4->matchs[ETH_ALEN-1] = matchs[j].v8;
                        break;                      
                        case FIELD_VLANID:
                        case FIELD_ETHTYPE:
                        case FIELD_SPORT:
                        case FIELD_DPORT:
                            pEntry4->matchs[ETH_ALEN-2] = matchs[j].v16 >> 8;
                            pEntry4->matchs[ETH_ALEN-1] = matchs[j].v16 & 0x00FF;
                        break;                      
                        case FIELD_SIP4:
                        case FIELD_DIP4:
                            pEntry4->matchs[ETH_ALEN-4] = matchs[j].ip4 >> 24;
                            pEntry4->matchs[ETH_ALEN-3] = matchs[j].ip4 >> 16;
                            pEntry4->matchs[ETH_ALEN-2] = matchs[j].ip4 >> 8;
                            pEntry4->matchs[ETH_ALEN-1] = matchs[j].ip4;
                        break;
                    }
                    dp += sizeof(OamQosRuleEntry4_t);
                }else{
                    pRule->length += sizeof(OamQosRuleEntry6_t);
                    pEntry6 = (OamQosRuleEntry6_Ptr)dp;
                    pEntry6->field = matchs[j].field;
                    pEntry6->validOperator = matchs[j].op;
                    
                    switch(pEntry6->field){
                        case FIELD_SIP6:
                        case FIELD_DIP6:
                            memcpy(pEntry6->matchs, matchs[j].ip6.s6_addr, 16);
                        break;
                        case FIELD_IPVER:
                        case FIELD_IPPROTO6:
                            memset(pEntry6->matchs, 0, 16);
                            pEntry6->matchs[15] = matchs[j].v8;
                        break;
                    }
                    dp += sizeof(OamQosRuleEntry6_t);
                }
            }
        }else
            return VAR_NO_RESOURCE;
    }

    *pLength = dp-pValue;
    return SUCCESS;
}
//@TODO
u_char oamSetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    u_char *dp = pValue;
    u_char rulesNum, matchsNum, field, i, j;
    OamCM_Ptr pClsfy = (OamCM_Ptr)pValue;
    QosResult_t result;
    QosMatchRule_t matchs[MAX_FIELD_NUM];

    OamQosRule_Ptr pRule;
    OamQosRuleEntry4_Ptr pEntry4;
    OamQosRuleEntry6_Ptr pEntry6;
    int ret = 0;

    if (eponOamCfg.isHgu){
        return VAR_BAD_PARAMETERS;
    }

    if (PORT_ERROR == portId)
        return VAR_BAD_PARAMETERS;

    if (*pValue == ACTION_CLEAR){
        if (0 == eponmapClearClsfyRules(portId)){
            system("echo pwan mapqos 0 > /proc/gpon/debug");
            return SET_OK;
        }

        return VAR_NO_RESOURCE;
    }

    length -= sizeof(OamCM_t);
    if (length < 0)
        return VAR_BAD_PARAMETERS;

    rulesNum = pClsfy->rulesNum;
    dp += sizeof(OamCM_t);

    // TODO: need process the length < the real need
    for (i = 0; i < rulesNum; ++i){
        pRule = (OamQosRule_Ptr)dp;

        length -= pRule->length + 1;
        if (length < 0)
            return VAR_BAD_PARAMETERS;

        dp += sizeof(OamQosRule_t);

        result.precedence  = pRule->precedence;
        result.priority    = pRule->etherPriorityMark;
        result.queueMapped = pRule->queueMapped;

        matchsNum = pRule->entriesNum;

        for (j = 0; j < matchsNum; ++j)
        {
            field = *dp;
            matchs[j].field = field;
            if (field < FIELD_IPVER){
                pEntry4 = (OamQosRuleEntry4_Ptr)dp;
                dp += sizeof(OamQosRuleEntry4_t);

                eponOamExtDbg(DBG_OAM_L4, "QosRule: matchs: %.2x%.2x%.2x%.2x%.2x%.2x\n", pEntry4->matchs[0],\
                    pEntry4->matchs[1],pEntry4->matchs[2],pEntry4->matchs[3],pEntry4->matchs[4],pEntry4->matchs[5]);
                switch(pEntry4->field){
                    case FIELD_SMAC:
                    case FIELD_DMAC:
                        memcpy(matchs[j].mac, pEntry4->matchs, 6);
                    break;
                    case FIELD_PBIT:
                    case FIELD_IPPROTO4:
                    case FIELD_IPDSCP4:
                    case FIELD_IPDSCP6:
                        matchs[j].v8 = pEntry4->matchs[5];
                    break;                      
                    case FIELD_VLANID:
                    case FIELD_ETHTYPE:
                    case FIELD_SPORT:
                    case FIELD_DPORT:                       
                        matchs[j].v16 = ((pEntry4->matchs[4] << 8) |
                                        pEntry4->matchs[5]);
                    break;                      
                    case FIELD_SIP4:
                    case FIELD_DIP4:
                        matchs[j].ip4 = (   (pEntry4->matchs[2] << 24)|
                                        (pEntry4->matchs[3] << 16)|
                                        (pEntry4->matchs[4] << 8)|
                                        (pEntry4->matchs[5]) );
                    break;
                }
                // memcpy(rules[j].match.value1, pEntry4->matchs, 6);
                matchs[j].op = pEntry4->validOperator;
            }else{
                pEntry6 = (OamQosRuleEntry6_Ptr)dp;
                dp += sizeof(OamQosRuleEntry6_t);

                switch(pEntry6->field){
                    case FIELD_SIP6:
                    case FIELD_DIP6:
                    case FIELD_SIP6PREX:
                    case FIELD_DIP6PREX:
                        memcpy(matchs[j].ip6.s6_addr, pEntry6->matchs, 16);
                    break;
                    case FIELD_IPVER:
                    case FIELD_IPPROTO6:
                            matchs[j].v8 = pEntry6->matchs[15];
                            break;
                    case FIELD_FLOWLABEL6:
                        matchs[j].ip4 = (   (pEntry6->matchs[13] << 16)|
                                        (pEntry6->matchs[14] << 8)|
                                        (pEntry6->matchs[15]) );
                    break;
                    default:
                        continue;
                }
                matchs[j].op = pEntry6->validOperator;
            }
        }       

        switch(pClsfy->action){
            case ACTION_ADD:
            ret = eponmapInsertClsfyRule(portId, &result, matchsNum, matchs);
            break;
            case ACTION_DELETE:
            ret = eponmapDeleteClsfyRule(portId, &result, matchsNum, matchs);
            break;
        }
        if (ret == FAIL){
            return VAR_NO_RESOURCE;
        }
    }

    system("echo pwan mapqos 1 > /proc/gpon/debug");

    return SET_OK;
}
#endif

/* --------------------------Qos attributions and actions end------------------------ */


/* -----------------------Multicast attributions and actions------------------------- */
//@TODO
u_char oamGetAddDelMulticastVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    OamMulticastVlan_Ptr mcvp = (OamMulticastVlan_Ptr) pValue;
    u_char i, num;
    int ret, vid1, vid2;

    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;
    
    ret = igmp_get_portvlan_cnt(portId+1);
    if (ret == FAIL)
        return VAR_NO_RESOURCE;

    num = ret;
    mcvp->action = ACTION_LIST;

    for (i = 0; i < num; i++){
        ret = igmp_get_portvlan_id(portId+1, i, &vid1, &vid2);
        if (ret == FAIL){
            return VAR_NO_RESOURCE;
        }
        mcvp->vids[i] = htons((u_short)vid1);
    }

    *pLength = 1 + 2*num;
    return SUCCESS;
}
//@TODO
u_char oamSetAddDelMulticastVlan(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    OamMulticastVlan_Ptr mcvp = (OamMulticastVlan_Ptr) pValue;
    int mcVidNum = (length - 1) / 2, i;

    if ((portId == PORT_ERROR) || (length < 1))
        return VAR_BAD_PARAMETERS;

    switch(mcvp->action){
    case ACTION_DELETE:
        for (i = 0; i < mcVidNum; i++){
            if (FAIL == igmp_del_portvlan(portId+1, ntohs(mcvp->vids[i]))){
                return VAR_NO_RESOURCE;
            }
            oam_ctc_db_mvlan_del(portId, ntohs(mcvp->vids[i]));
        }
        break;
    case ACTION_ADD:
        for (i = 0; i < mcVidNum; i++){
            if (FAIL == igmp_add_portvlan(portId+1, ntohs(mcvp->vids[i]))){
                return VAR_NO_RESOURCE;
            }           
            oam_ctc_db_mvlan_add(portId, ntohs(mcvp->vids[i]));
        }
        break;
    case ACTION_CLEAR:
        if (FAIL == igmp_clear_portvlan(portId+1)){
            return VAR_NO_RESOURCE;
        }
        gCtcDB.port[portId].mcast.trunk_num = 0;
        break;
    default:
        return VAR_BAD_PARAMETERS;
    }

    return SET_OK;
}

//@TODO
u_char oamGetMulticastTagOper(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    int ret;
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    OamMulticastTagOper_Ptr mctop = (OamMulticastTagOper_Ptr)pValue;

    if (portId == PORT_ERROR)
        return VAR_BAD_PARAMETERS;

    ret = igmp_get_tagstrip(portId+1);
    if (FAIL == ret)
        return VAR_NO_RESOURCE;
    mctop->tagStriped = (u_char)ret;

    if (mctop->tagStriped == TAG_STRIPED_SWITCH){
        u_char i = 0;
        int mcVid = 0, usrVid = 0;

        ret = igmp_get_portvlan_switchcnt(portId+1);
        if (FAIL == ret)
            return VAR_NO_RESOURCE;

        mctop->switchNum = (u_char)ret;
        for (; i < mctop->switchNum; i++){
            ret = igmp_get_portvlan_switchid(portId+1, i, &mcVid, &usrVid);
            if (FAIL == ret)
                return VAR_NO_RESOURCE;

            mctop->switchs[i].mcVlanId   = htons(mcVid);
            mctop->switchs[i].iptvVlanId = htons(usrVid);
        }
        *pLength = 2 + mctop->switchNum*4;
    }else{
        *pLength = 1;
    }
    return SUCCESS;
}
//@TODO
u_char oamSetMulticastTagOper(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    int   i;
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    OamMulticastTagOper_Ptr mctop = (OamMulticastTagOper_Ptr)pValue;

    if ((portId == PORT_ERROR) || (length < 1) || (mctop->tagStriped > TAG_STRIPED_SWITCH))
        return VAR_BAD_PARAMETERS;

    if (FAIL == igmp_set_tagstrip(portId+1, mctop->tagStriped))
        return VAR_NO_RESOURCE;

    gCtcDB.port[portId].mcast.tag_strip = mctop->tagStriped;

    if (mctop->tagStriped == TAG_STRIPED_SWITCH){
        OamVlanSwitch_Ptr mcvsp = NULL;
        
        if (length < (sizeof(OamMulticastTagOper_t) + mctop->switchNum*sizeof(OamVlanSwitch_t)))
            return VAR_BAD_PARAMETERS;
        
        /* TODO: need to clear before set */
        gCtcDB.port[portId].mcast.trans_num = mctop->switchNum;
        for  (i = 0; i < mctop->switchNum; i++){
            mcvsp = &(mctop->switchs[i]);
            if (FAIL == igmp_set_portvlan_switchid(portId+1, ntohs(mcvsp->mcVlanId), ntohs(mcvsp->iptvVlanId)))
                return VAR_NO_RESOURCE;

            gCtcDB.port[portId].mcast.mcast_vid[i] = ntohs(mcvsp->mcVlanId);
            gCtcDB.port[portId].mcast.user_vid[i] = ntohs(mcvsp->iptvVlanId);
        }
    }else{
        gCtcDB.port[portId].mcast.trans_num = 0;
        memset(gCtcDB.port[portId].mcast.mcast_vid, 0, sizeof(u_short)*OAM_VLAN_RULE_MAX_NUM);
        memset(gCtcDB.port[portId].mcast.user_vid, 0, sizeof(u_short)*OAM_VLAN_RULE_MAX_NUM);
    }
    return SET_OK;
}

//@OK
u_char oamGetMulticastSwitch(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    int ret = igmp_get_fwdmode();
    if (FAIL == ret)
        ret = gCtcDB.cfg_info.mcast.type;

    setU8(pValue, (u_char)ret);
    *pLength = 1;

    return SUCCESS;
}
//@OK
u_char oamSetMulticastSwitch(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char mcType = GetU8(pValue);

    if (mcType > MC_CTRL_TYPE_CTC)
        return VAR_BAD_PARAMETERS;
    if (igmp_get_fwdmode() != mcType){
        igmp_clear_fwdentry();
        igmp_set_fastleave_ctc(0);
        if (mcType == MC_CTRL_TYPE_CTC){
            int i = 0, portNum = 4;
            for (i = 0; i < portNum; i++){
                igmp_clear_portvlan(i+1);
                igmp_set_max_playgroup(i+1, 0); // don't limit
                igmp_set_tagstrip(i+1, TAG_STRIPED_NO);
            }
        }
    }
    
    if (FAIL == igmp_set_fwdmode(mcType))
        return VAR_NO_RESOURCE;

    gCtcDB.cfg_info.mcast.type = mcType;
    return SET_OK;
}

//@OK
u_char oamGetMulticastControl(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
#define MAX_MC_NUM_PER_VC 12    
    u_char num, *dp = pValue, type, len;
    int count, i, percount, perlen, entryNum;
    u_char   mac[16], ip[16];
    int vid, mcType, port;
    OamCtcBranchLeaf_Ptr pbl;
    OamMulticastControl_Ptr pCtrl;
    OamMCC1Entry_Ptr pEntry1;
    OamMCC2Entry_Ptr pEntry2;
    OamMCC3Entry_Ptr pEntry3;
    OamMCC4Entry_Ptr pEntry4;
    u_char client_ip[16];
    int join_time = 0;

    count = igmp_get_fwdentry_cnt();
    if (count < 1)
        return VAR_NO_RESOURCE;

    if (FAIL == igmp_get_fwdentry(0, &mcType, &port, &vid, mac, ip,client_ip,&join_time))
        return VAR_NO_RESOURCE;
    type = mcType;

    switch(type){
        case MULITCAST_TYPE_MAC:
        case MULITCAST_TYPE_MAC_VLANID:
        case MULITCAST_TYPE_IPV4_VLANID:
            perlen = sizeof(OamMCC1Entry_t);
        break;
        case MULITCAST_TYPE_MAC_SAIPV4:
            perlen = sizeof(OamMCC2Entry_t);
        break;
        case MULITCAST_TYPE_IPV6_VLANID:
            perlen = sizeof(OamMCC3Entry_t);
        break;
        case MULITCAST_TYPE_MAC_SAIPV6:
            perlen = sizeof(OamMCC4Entry_t);
        break;
        default:
            return VAR_NO_RESOURCE;
        break;
    }
    percount = MIN(MAX_MC_NUM_PER_VC, 128/perlen);

    num = 0;
    for (i = 0; i < count; i++){
        if (FAIL == igmp_get_fwdentry(i, &mcType, &port, &vid, mac, ip,client_ip,&join_time))
            return VAR_NO_RESOURCE;
        if (mcType != type){
            count --; // for first mcType is MAIN, remove other mcType entry.
            continue;
        }

        // each container only contain less equal than 12 items.
        if (i % percount == 0){
            if (num == count / percount)
                entryNum = count % percount;
            else
                entryNum = percount;

            pbl = (OamCtcBranchLeaf_Ptr)dp;
            pbl->branch = OAM_EXT_ATTR;
            pbl->leaf = htons(OAM_LF_MULTICASTCONTROL);
            PTR_MOVE(dp, sizeof(OamCtcBranchLeaf_t));

            len = sizeof(OamMulticastControl_t) + perlen*entryNum;
            setU8(dp, len);
            PTR_MOVE(dp, 1);

            num ++;
            pCtrl = (OamMulticastControl_Ptr)dp;
            pCtrl->action = ACTION_LIST;
            pCtrl->type = type;
            pCtrl->entriesNum = entryNum;

            PTR_MOVE(dp, sizeof(OamMulticastControl_t));
        }

        port -= 1;  // for offset port 0~3 to eth0.1~4
        switch(mcType){
            case MULITCAST_TYPE_MAC:
                pEntry1 = (OamMCC1Entry_Ptr)dp;
                pEntry1->portId = htons(port);
                pEntry1->vid = 0;
                memcpy(pEntry1->mac, mac, ETH_ALEN);
                PTR_MOVE(dp, sizeof(OamMCC1Entry_t));
            break;
            case MULITCAST_TYPE_MAC_VLANID:
                pEntry1 = (OamMCC1Entry_Ptr)dp;
                pEntry1->portId = htons(port);
                pEntry1->vid = htons(vid);
                memcpy(pEntry1->mac, mac, ETH_ALEN);
                PTR_MOVE(dp, sizeof(OamMCC1Entry_t));
            break;
            case MULITCAST_TYPE_IPV4_VLANID:
                pEntry1 = (OamMCC1Entry_Ptr)dp;
                pEntry1->portId = htons(port);
                pEntry1->vid = htons(vid);
                pEntry1->mac[0] = 0;
                pEntry1->mac[1] = 0;
                pEntry1->mac[2] = ip[0];
                pEntry1->mac[3] = ip[1];
                pEntry1->mac[4] = ip[2];
                pEntry1->mac[5] = ip[3];
                PTR_MOVE(dp, sizeof(OamMCC1Entry_t));
            break;
            case MULITCAST_TYPE_MAC_SAIPV4:
                pEntry2 = (OamMCC2Entry_Ptr)dp;
                pEntry2->portId = htons(port);
                memcpy(pEntry2->mac, mac, ETH_ALEN);
                //pEntry2->srcIp = htonl(ip4);
                memcpy(&(pEntry2->srcIp), ip, 4);
                PTR_MOVE(dp, sizeof(OamMCC2Entry_t));
            break;
            case MULITCAST_TYPE_IPV6_VLANID:
                pEntry3 = (OamMCC3Entry_Ptr)dp;
                pEntry3->portId = htons(port);
                pEntry3->vid = htons(vid);
                memcpy(pEntry3->ip6, mac, 16);
                PTR_MOVE(dp, sizeof(OamMCC3Entry_t));
            break;
            case MULITCAST_TYPE_MAC_SAIPV6:
                pEntry4 = (OamMCC4Entry_Ptr)dp;
                pEntry4->portId = htons(port);
                memcpy(pEntry4->mac, mac, ETH_ALEN);
                memcpy(pEntry4->srcIp, ip, 16);
                PTR_MOVE(dp, sizeof(OamMCC4Entry_t));
            break;
        }
    }

    *pLength = dp - pValue;
    return DIRECT_COPY;
}
//@TODO to detect length
u_char oamSetMulticastControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    //BUG : to fix the max 12 entrys
    int ret;
    OamMulticastControl_Ptr pMCCtrl = (OamMulticastControl_Ptr)pValue;
    u_char action = pMCCtrl->action;
    u_char i = 0;
    int offset = sizeof(OamMulticastControl_t);

    u_short portId, mcVid;
    u_char *gdaMac, *srcIpv6, srcIp4[16];
    int   perLen;
    u_char macNull[6],ipNull[16];

    if (length < 1)
        return VAR_BAD_PARAMETERS;

    if (action == ACTION_CLEAR){
        if (FAIL == igmp_clear_fwdentry())
            return VAR_NO_RESOURCE;
        return SET_OK;
    }

    if (length < sizeof(OamMulticastControl_t))
        return VAR_BAD_PARAMETERS;

    switch(pMCCtrl->type){
        case MULITCAST_TYPE_MAC:
        case MULITCAST_TYPE_MAC_VLANID:
        case MULITCAST_TYPE_IPV4_VLANID:
            perLen = sizeof(OamMCC1Entry_t);
        break;
        case MULITCAST_TYPE_MAC_SAIPV4:
            perLen = sizeof(OamMCC2Entry_t);
        break;
        case MULITCAST_TYPE_IPV6_VLANID:
            perLen = sizeof(OamMCC3Entry_t);
        break;
        case MULITCAST_TYPE_MAC_SAIPV6:
            perLen = sizeof(OamMCC4Entry_t);
        break;
        default:
            return VAR_BAD_PARAMETERS;
    }

    if (length < (sizeof(OamMulticastControl_t) + pMCCtrl->entriesNum*perLen))
        return VAR_BAD_PARAMETERS;

    memset(macNull, 0, 6);
    memset(ipNull, 0, 16);
    memset(srcIp4, 0, 16);
    for (; i < pMCCtrl->entriesNum; ++i)
    {
        switch(pMCCtrl->type){
            case MULITCAST_TYPE_MAC:
            case MULITCAST_TYPE_MAC_VLANID:
            case MULITCAST_TYPE_IPV4_VLANID:
            {
                portId = GetU16(pValue+offset);
                mcVid = GetU16(pValue+offset+2);
                if (pMCCtrl->type == MULITCAST_TYPE_IPV4_VLANID){
                    //memcpy(srcIp4, pValue+offset+4+2, 4);
                    //gdaMac = srcIp4;
                    gdaMac = pValue+offset+4+2;
                }else{
                    gdaMac = pValue+offset+4;
                }
                // TODO: the multicast api exist memory leak
                if (action == ACTION_ADD)
                    ret = igmp_add_fwdentry(pMCCtrl->type, portId, mcVid, gdaMac,ipNull);
                else
                    ret = igmp_del_fwdentry(pMCCtrl->type, portId, mcVid, gdaMac, ipNull);

                if (FAIL == ret)
                    return VAR_NO_RESOURCE;

                offset += 2+2+6;
            }
            break;
            case MULITCAST_TYPE_MAC_SAIPV4:
            {
                portId = GetU16(pValue+offset);
                gdaMac = pValue+offset+2;
                memcpy(srcIp4, pValue+offset+8, 4);
                
                if (action == ACTION_ADD)
                    ret = igmp_add_fwdentry(pMCCtrl->type, portId, 0, gdaMac, srcIp4);
                else
                    ret = igmp_del_fwdentry(pMCCtrl->type, portId, 0, gdaMac, srcIp4);

                if (FAIL == ret)
                    return VAR_NO_RESOURCE;

                offset += 2+6+4;
            }
            break;
            case MULITCAST_TYPE_IPV6_VLANID:
            {
                portId = GetU16(pValue+offset);
                mcVid = GetU16(pValue+offset+2);
                srcIpv6 = pValue+offset+4;

                if (action == ACTION_ADD)
                    ret = igmp_add_fwdentry(pMCCtrl->type, portId, mcVid, srcIpv6, ipNull);
                else
                    ret = igmp_del_fwdentry(pMCCtrl->type, portId, mcVid, srcIpv6, ipNull);

                if (FAIL == ret)
                    return VAR_NO_RESOURCE;

                offset += 2+2+16;
            }
            break;
            case MULITCAST_TYPE_MAC_SAIPV6:
            {
                portId = GetU16(pValue+offset);
                gdaMac = pValue+offset+2;
                srcIpv6 = pValue+offset+8;

                if (action == ACTION_ADD)
                    ret = igmp_add_fwdentry(pMCCtrl->type, portId, 0, gdaMac, srcIpv6);
                else
                    ret = igmp_del_fwdentry(pMCCtrl->type, portId, 0, gdaMac, srcIpv6);

                if (FAIL == ret)
                    return VAR_NO_RESOURCE;

                offset += 2+6+16;
            }
            break;
        }
    }

    return SET_OK;
}

//@OK
u_char oamGetGroupNumMax(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    int ret;
    u_char portId = sysObjIdx(pObj, PORT_ETHER);

    if (PORT_ERROR == portId)
        return VAR_BAD_PARAMETERS;

    ret = igmp_get_max_playgroup(portId+1);
    if (FAIL == ret){
        ret = gCtcDB.port[portId].mcast.max_grp_num;
    }

    setU8(pValue, (u_char)ret);
    *pLength = 1;
    return SUCCESS;
}
//@OK
u_char oamSetGroupNumMax(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char portId      = sysObjIdx(pObj, PORT_ETHER);
    u_char maxGroupNum = GetU8(pValue);
    
    if (PORT_ERROR == portId)
        return VAR_BAD_PARAMETERS;  

    if (FAIL == igmp_set_max_playgroup(portId+1, maxGroupNum)){
        return VAR_NO_RESOURCE;
    }

    gCtcDB.port[portId].mcast.max_grp_num = maxGroupNum;
    return SET_OK;
}

//@TODO
u_char oamGetFastLeaveAbility(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char supportNum = 4;
    OamFastLeaveAbility_Ptr fap = (OamFastLeaveAbility_Ptr)pValue;


    fap->mode[0] = htonl(0x00000000); // IGMP snooping Non-Fast-Leave
    fap->mode[1] = htonl(0x00000001); // IGMP snooping Fast-Leave
    fap->mode[2] = htonl(0x00000010); // CTC control  Non-Fast-Leave
    fap->mode[3] = htonl(0x00000011); // CTC control  Fast-Leave
    
    if (gCtcHandler.usedVerion[llidIdx] >= 0x30){
        fap->mode[4] = htonl(0x00000002); // MLD snooping Non-Fast-Leave
        fap->mode[5] = htonl(0x00000003); // MLD snooping Fast-Leave
        supportNum += 2;
    }
    
    fap->enumNum = htonl(supportNum);
    *pLength = 4 + 4 * supportNum;
    return SUCCESS;
}
//@TODO
u_char oamGetFastLeaveAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    int ret = igmp_get_fastleave_ctc();
    if (FAIL == ret){
        ret = gCtcDB.cfg_info.mcast.fast_leave;
    }

    setU32(pValue, (u_int)(ret+1));
    *pLength = 4;

    return SUCCESS;
}

//@TODO
// if MulticastSwitch and fastleave appears in same OAMPDU, It must be MulticastSwitch run first.
u_char oamSetFastLeaveAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
#if 1//def TCSUPPORT_RESET
extern LastWorkNode_t gLastWorkTable[MAX_LAST_WORK_NODES];
    u_char *pdata = NULL;

    // if (gLastWorkTable[FAST_LEAVE].flag == FALSE)
    {   
        eponOamExtDbg(DBG_OAM_L4, "%s: 1 gLastWorkTable[FAST_LEAVE].pData =%x process.\n", __FUNCTION__, gLastWorkTable[FAST_LEAVE].pData);
        if (gLastWorkTable[FAST_LEAVE].pData != NULL){
            free(gLastWorkTable[FAST_LEAVE].pData);
            gLastWorkTable[FAST_LEAVE].pData = NULL;
        }
        pdata = (u_char *)malloc(4);
        if (pdata == NULL)
            return VAR_NO_RESOURCE;

        gLastWorkTable[FAST_LEAVE].pData  = pdata;
        memcpy(gLastWorkTable[FAST_LEAVE].pData, pValue, 4);
        eponOamExtDbg(DBG_OAM_L4, "%s: 2 gLastWorkTable[FAST_LEAVE].pData=%x process.\n", __FUNCTION__, gLastWorkTable[FAST_LEAVE].pData);
        
        if (pObj != NULL)
            gLastWorkTable[FAST_LEAVE].obj = *pObj;
        
        gLastWorkTable[FAST_LEAVE].flag   = TRUE;
        gLastWorkTable[FAST_LEAVE].llidIdx = llidIdx;
        gLastWorkTable[FAST_LEAVE].length = length;
        gLastWorkTable[FAST_LEAVE].branchLeaf.branch = OAM_EXT_ACT;
        gLastWorkTable[FAST_LEAVE].branchLeaf.leaf   = OAM_LF_FASTLEAVEADMINCONTROL;

        eponOamExtDbg(DBG_OAM_L4, "%s: gLastWorkTable[FAST_LEAVE] process OK.\n", __FUNCTION__);
        
        return RET_NOT_REPLY;
    }
#else
        u_int active = GetU32(pValue);
        if (active < DISABLED || active > ENABLED)
            return VAR_BAD_PARAMETERS;

        if (FAIL == igmp_set_fastleave_ctc((u_char)(active-1)))
            return VAR_NO_RESOURCE;
        return SET_OK;
#endif
}

//@TODO
u_char oamResetFastLeaveAdminControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_int active = GetU32(pValue);
    if (active < DISABLED || active > ENABLED)
        return VAR_BAD_PARAMETERS;

    if (FAIL == igmp_set_fastleave_ctc((active-1)))
        return VAR_NO_RESOURCE;

    gCtcDB.cfg_info.mcast.fast_leave = active-1;

    return SET_OK;

}

/* -----------------------Multicast attributions and actions end---------------------- */



// #OK
u_char oamSetResetOnu(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    system("reboot");
    return RET_NOT_REPLY;
}

// #OK
u_char oamGetAlarmAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    //BUG: how to get the state ???  only use branch & leaf 
    // TODO: upload all alarm ?
    OamAlarmControl_Ptr pAlarmCtl = (OamAlarmControl_Ptr)pValue; 
    OamAlarmNode_Ptr pAlarmNode   = NULL;
    OamEventStatusEntry_t ese     = {0};
    u_short alarmIdStart, alarmIdEnd, objType, leaf = htons(OAM_LF_ALARMADMINSTATE);
    u_int   objInstance;
    u_char  portid;

    *pLength = 0;

    if (pObj == NULL){
        objInstance  = 0;
        objType      = OBJ_LF_ONU;
        alarmIdStart = ALARM_ID_EQUIPMENT;
        alarmIdEnd   = ALARM_ID_ONU_USED_MAX;
    }else{
        if ((portid = sysObjIdx(pObj, PORT_ETHER)) != PORT_ERROR){
            objInstance  = portid;
            objType      = OBJ_LF_PORT;
            alarmIdStart = ALARM_ID_ETHPORT_AUTONEGFAILURE;
            alarmIdEnd   = ALARM_ID_ETHPORT_USED_MAX;
        }else if ((portid = sysObjIdx(pObj, PORT_PON)) != PORT_ERROR){
            objInstance  = portid;
            objType      = OBJ_LF_PONIF;
            alarmIdStart = ALARM_ID_RXPOWERHIGH;
            alarmIdEnd   = ALARM_ID_PONIF_USED_MAX;
        }else{
            *pLength = 0;
            return VAR_BAD_PARAMETERS;
        }
    }

    while(alarmIdStart < alarmIdEnd){
        pAlarmNode = findAlarmNodeById(alarmIdStart);
        if (pAlarmNode){
            ese.alarmID     = alarmIdStart;
            ese.objectType  = objType;
            ese.instanceNum = objInstance;
            pAlarmNode->oamStatus(&ese, REQ_GET);

            pAlarmCtl->branch  = OAM_EXT_ATTR;
            pAlarmCtl->leaf    = leaf;
            pAlarmCtl->len     = 6;
            pAlarmCtl->alarmId = htons(alarmIdStart);
            pAlarmCtl->status  = htonl(ese.status+1);

            pAlarmCtl ++;
            *pLength += sizeof(OamAlarmControl_t); // 10
        }
        alarmIdStart ++;
    }

    return DIRECT_COPY;
}
// #OK
u_char oamSetAlarmAdminState(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    OamAlarmState_Ptr pAlarmState = (OamAlarmState_Ptr)pValue;
    OamAlarmNode_Ptr pAlarmNode   = findAlarmNodeById(ntohs(pAlarmState->alarmId));
    OamEventStatusEntry_t ese     = {0};

    if (!pAlarmNode){
        eponOamExtDbg(DBG_OAM_L4, ">> Alarm Node: %.4X not find!\n", ntohs(pAlarmState->alarmId));
        return VAR_BAD_PARAMETERS;
    }
    
    ese.alarmID = ntohs(pAlarmState->alarmId);
    ese.status  = ntohl(pAlarmState->status)-1;
    eponOamExtDbg(DBG_OAM_L4, ">> Alarm Node: %s, status: %u\n", pAlarmNode->name, ese.status);
    if (ese.status != 1 &&  ese.status != 0){
        eponOamExtDbg(DBG_OAM_L4, ">> Alarm Status: %u be skiped!\n", ese.status );
        return SET_OK;
    }
    
    if (pObj == NULL){
        if (ese.alarmID < ALARM_ID_ONU_USED_MAX){
            pAlarmNode->oamStatus(&ese, REQ_SET);
            return SET_OK;
        }
    }else{
        u_int portid;
        if ((portid = sysObjIdx(pObj, PORT_ETHER)) != PORT_ERROR){
            if (ese.alarmID < ALARM_ID_ETHPORT_USED_MAX && ese.alarmID >= ALARM_ID_ETHPORT_AUTONEGFAILURE){
                ese.objectType = OBJ_LF_PORT;
                ese.instanceNum = portid;
                pAlarmNode->oamStatus(&ese, REQ_SET);
                return SET_OK;
            }
        }else if ((portid = sysObjIdx(pObj, PORT_PON)) != PORT_ERROR){
            if (ese.alarmID < ALARM_ID_PONIF_USED_MAX && ese.alarmID >= ALARM_ID_RXPOWERHIGH){
                ese.objectType = OBJ_LF_PONIF;
                ese.instanceNum = portid;
                pAlarmNode->oamStatus(&ese, REQ_SET);
                return SET_OK;
            }
        }
    }

    return VAR_BAD_PARAMETERS;
}

// #OK
u_char oamGetAlarmThresholds(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    OamAlarmThresholdControl_Ptr pAlarmCtl = (OamAlarmThresholdControl_Ptr)pValue;
    OamAlarmNode_Ptr pAlarmNode            = NULL;
    OamEventThresholdEntry_t ese           = {0};
    u_short alarmIdStart, alarmIdEnd, objType, leaf = htons(OAM_LF_ALARMADMINSTATE);
    u_int   objInstance;
    u_char  portid;

    *pLength = 0;

    if (pObj == NULL){
        objInstance  = 0;
        objType      = OBJ_LF_ONU;
        alarmIdStart = ALARM_ID_EQUIPMENT;
        alarmIdEnd   = ALARM_ID_ONU_USED_MAX;
    }else{
        if ((portid = sysObjIdx(pObj, PORT_ETHER)) != PORT_ERROR){
            objInstance  = portid;
            objType      = OBJ_LF_PORT;
            alarmIdStart = ALARM_ID_ETHPORT_AUTONEGFAILURE;
            alarmIdEnd   = ALARM_ID_ETHPORT_USED_MAX;
        }else if ((portid = sysObjIdx(pObj, PORT_PON)) != PORT_ERROR){
            objInstance  = portid;
            objType      = OBJ_LF_PONIF;
            alarmIdStart = ALARM_ID_RXPOWERHIGH;
            alarmIdEnd   = ALARM_ID_PONIF_USED_MAX;
        }else{
            *pLength = 0;
            return VAR_BAD_PARAMETERS;
        }
    }

    while(alarmIdStart < alarmIdEnd){
        pAlarmNode = findAlarmNodeById(alarmIdStart);
        if (pAlarmNode){
            ese.alarmID     = alarmIdStart;
            ese.objectType  = objType;
            ese.instanceNum = objInstance;
            pAlarmNode->oamThreshold(&ese, REQ_GET);

            pAlarmCtl->branch  = OAM_EXT_ATTR;
            pAlarmCtl->leaf    = leaf;
            pAlarmCtl->len     = 10;
            pAlarmCtl->alarmId = htons(alarmIdStart);
            pAlarmCtl->alarm   = htonl(ese.setThreshold);
            pAlarmCtl->clear   = htonl(ese.clearThreshold);

            pAlarmCtl ++;
            *pLength += 10;
        }
        alarmIdStart ++;
    }

    return DIRECT_COPY;
}
// #OK
u_char oamSetAlarmThresholds(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    OamAlarmThreshold_Ptr pAlarmThreshold = (OamAlarmThreshold_Ptr)pValue;
    OamAlarmNode_Ptr pAlarmNode           = findAlarmNodeById(ntohs(pAlarmThreshold->alarmId));
    OamEventThresholdEntry_t ethe         = {0};

    if (!pAlarmNode)
        return VAR_BAD_PARAMETERS;

    ethe.alarmID        = ntohs(pAlarmThreshold->alarmId);
    ethe.setThreshold   = ntohl(pAlarmThreshold->alarm);
    ethe.clearThreshold = ntohl(pAlarmThreshold->clear);

    if (pObj == NULL){
        if (ethe.alarmID < ALARM_ID_ONU_USED_MAX){
            pAlarmNode->oamThreshold(&ethe, REQ_SET);
            return SET_OK;
        }
    }else{
        u_int portid;
        if ((portid = sysObjIdx(pObj, PORT_ETHER)) != PORT_ERROR){
            if (ethe.alarmID < ALARM_ID_ETHPORT_USED_MAX && ethe.alarmID >= ALARM_ID_ETHPORT_AUTONEGFAILURE){
                ethe.objectType = OBJ_LF_PORT;
                ethe.instanceNum = portid;
                pAlarmNode->oamThreshold(&ethe, REQ_SET);
                return SET_OK;
            }
        }else if ((portid = sysObjIdx(pObj, PORT_PON)) != PORT_ERROR){
            if (ethe.alarmID < ALARM_ID_PONIF_USED_MAX && ethe.alarmID >= ALARM_ID_RXPOWERHIGH){
                ethe.objectType = OBJ_LF_PONIF;
                ethe.instanceNum = portid;
                pAlarmNode->oamThreshold(&ethe, REQ_SET);
                return SET_OK;
            }
        }
    }

    return VAR_BAD_PARAMETERS;
}

/* -------------------------Perforemance Attributes and actions------------------------- */
#ifdef TCSUPPORT_PMMGR
// #OK
u_char oamGetPerformanceStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char ponif = sysObjIdx(pObj, PORT_PON);
    u_char portid = sysObjIdx(pObj, PORT_ETHER);

    if ((PORT_ERROR != ponif) || (PORT_ERROR != portid)){
        int ret;
        u_char port = portid;
        PMStatus_Ptr pPMStatus = (PMStatus_Ptr)pValue;

        if ( (OBJ_LF_PONIF == pObj->leaf) || 
             (pObj->index == OBJ_LF_PON_START_V1) ){
            port = PORT_ETHER_NUM + ponif; // PON IF 
        }

        ret = getPMEnable(port);
        if (FAIL == ret){
            return VAR_NO_RESOURCE;
        }
        pPMStatus->enable = htons(ret+1);

        ret = getPMCycleTime(port);
        if (FAIL == ret){
            return VAR_NO_RESOURCE;
        }
        pPMStatus->period = htonl(ret);

        *pLength = sizeof(PMStatus_t);
        return SUCCESS;
    }

    return VAR_BAD_PARAMETERS;
}
// #OK
u_char oamSetPerformanceStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char ponif = sysObjIdx(pObj, PORT_PON);
    u_char portid = sysObjIdx(pObj, PORT_ETHER);

    if ((PORT_ERROR != ponif) || (PORT_ERROR != portid)){
        int ret;
        u_char port = portid;
        PMStatus_Ptr pPMStatus = (PMStatus_Ptr)pValue;

        if ( (OBJ_LF_PONIF == pObj->leaf) || 
             (pObj->index == OBJ_LF_PON_START_V1) ){
            port = PORT_ETHER_NUM + ponif; // PON IF 
        }

        ret = setPMEnable(port, ntohs(pPMStatus->enable)-1);
        if (FAIL == ret){
            return VAR_NO_RESOURCE;
        }

        ret = setPMCycleTime(port, ntohl(pPMStatus->period));
        if (FAIL == ret){
            return VAR_NO_RESOURCE;
        }

        return SET_OK;
    }

    return VAR_BAD_PARAMETERS;
}

inline long long htonll(long long data)
{
#ifndef __BIG_ENDIAN    
    long h,l;
    l = data & 0xFFFFFFFF;
    h = (data>>32) & 0xFFFFFFFF;
    l = htonl(l);
    h = htonl(h);
    return (((long long)l << 32) | h);
#else
    return data;
#endif
}

inline long long ntohll(long long data)
{
#ifndef __BIG_ENDIAN    
    long h,l;
    l = data & 0xFFFFFFFF;
    h = (data>>32) & 0xFFFFFFFF;
    l = ntohl(l);
    h = ntohl(h);
    return (((long long)l << 32) | h);
#else
    return data;
#endif
}

inline void htonpm(PMCounters_Ptr pCnt)
{
    pCnt->downstreamDropEvents       = htonll(pCnt->downstreamDropEvents);
    pCnt->upstreamDropEvents         = htonll(pCnt->upstreamDropEvents);
    pCnt->downstreamOctets           = htonll(pCnt->downstreamOctets);
    pCnt->upstreamOctets             = htonll(pCnt->upstreamOctets);
    pCnt->downstreamFrames           = htonll(pCnt->downstreamFrames);
    pCnt->upstreamFrames             = htonll(pCnt->upstreamFrames);
    pCnt->downstreamBroadcastFrames  = htonll(pCnt->downstreamBroadcastFrames);
    pCnt->upstreamBroadcastFrames    = htonll(pCnt->upstreamBroadcastFrames);
    pCnt->downstreamMulticastFrames  = htonll(pCnt->downstreamMulticastFrames);
    pCnt->upstreamMulticastFrames    = htonll(pCnt->upstreamMulticastFrames);
    pCnt->downstreamCrcErroredFrames = htonll(pCnt->downstreamCrcErroredFrames);
    pCnt->upstreamCrcErroredFrames   = htonll(pCnt->upstreamCrcErroredFrames);
    pCnt->downstreamUndersizeFrames  = htonll(pCnt->downstreamUndersizeFrames);
    pCnt->upstreamUndersizeFrames    = htonll(pCnt->upstreamUndersizeFrames);
    pCnt->downstreamOversizeFrames   = htonll(pCnt->downstreamOversizeFrames);
    pCnt->upstreamOversizeFrames     = htonll(pCnt->upstreamOversizeFrames);
    pCnt->downstreamFragments        = htonll(pCnt->downstreamFragments);
    pCnt->upstreamFragments          = htonll(pCnt->upstreamFragments);
    pCnt->downstreamJabbers          = htonll(pCnt->downstreamJabbers);
    pCnt->upstreamJabbers            = htonll(pCnt->upstreamJabbers);
    pCnt->downstream64Frames         = htonll(pCnt->downstream64Frames);
    pCnt->upstream64Frames           = htonll(pCnt->upstream64Frames);
    pCnt->downstream127Frames        = htonll(pCnt->downstream127Frames);
    pCnt->upstream127Frames          = htonll(pCnt->upstream127Frames);
    pCnt->downstream255Frames        = htonll(pCnt->downstream255Frames);
    pCnt->upstream255Frames          = htonll(pCnt->upstream255Frames);
    pCnt->downstream511Frames        = htonll(pCnt->downstream511Frames);
    pCnt->upstream511Frames          = htonll(pCnt->upstream511Frames);
    pCnt->downstream1023Frames       = htonll(pCnt->downstream1023Frames);
    pCnt->upstream1023Frames         = htonll(pCnt->upstream1023Frames);
    pCnt->downstream1518Frames       = htonll(pCnt->downstream1518Frames);
    pCnt->upstream1518Frames         = htonll(pCnt->upstream1518Frames);
    pCnt->downstreamDiscards         = htonll(pCnt->downstreamDiscards);
    pCnt->upstreamDiscards           = htonll(pCnt->upstreamDiscards);
    pCnt->downstreamErrors           = htonll(pCnt->downstreamErrors);
    pCnt->upstreamErrors             = htonll(pCnt->upstreamErrors);
    pCnt->statusChangeTimes          = htonll(pCnt->statusChangeTimes);
}
inline void ntohpm(PMCounters_Ptr pCnt)
{
    pCnt->downstreamDropEvents       = ntohll(pCnt->downstreamDropEvents);
    pCnt->upstreamDropEvents         = ntohll(pCnt->upstreamDropEvents);
    pCnt->downstreamOctets           = ntohll(pCnt->downstreamOctets);
    pCnt->upstreamOctets             = ntohll(pCnt->upstreamOctets);
    pCnt->downstreamFrames           = ntohll(pCnt->downstreamFrames);
    pCnt->upstreamFrames             = ntohll(pCnt->upstreamFrames);
    pCnt->downstreamBroadcastFrames  = ntohll(pCnt->downstreamBroadcastFrames);
    pCnt->upstreamBroadcastFrames    = ntohll(pCnt->upstreamBroadcastFrames);
    pCnt->downstreamMulticastFrames  = ntohll(pCnt->downstreamMulticastFrames);
    pCnt->upstreamMulticastFrames    = ntohll(pCnt->upstreamMulticastFrames);
    pCnt->downstreamCrcErroredFrames = ntohll(pCnt->downstreamCrcErroredFrames);
    pCnt->upstreamCrcErroredFrames   = ntohll(pCnt->upstreamCrcErroredFrames);
    pCnt->downstreamUndersizeFrames  = ntohll(pCnt->downstreamUndersizeFrames);
    pCnt->upstreamUndersizeFrames    = ntohll(pCnt->upstreamUndersizeFrames);
    pCnt->downstreamOversizeFrames   = ntohll(pCnt->downstreamOversizeFrames);
    pCnt->upstreamOversizeFrames     = ntohll(pCnt->upstreamOversizeFrames);
    pCnt->downstreamFragments        = ntohll(pCnt->downstreamFragments);
    pCnt->upstreamFragments          = ntohll(pCnt->upstreamFragments);
    pCnt->downstreamJabbers          = ntohll(pCnt->downstreamJabbers);
    pCnt->upstreamJabbers            = ntohll(pCnt->upstreamJabbers);
    pCnt->downstream64Frames         = ntohll(pCnt->downstream64Frames);
    pCnt->upstream64Frames           = ntohll(pCnt->upstream64Frames);
    pCnt->downstream127Frames        = ntohll(pCnt->downstream127Frames);
    pCnt->upstream127Frames          = ntohll(pCnt->upstream127Frames);
    pCnt->downstream255Frames        = ntohll(pCnt->downstream255Frames);
    pCnt->upstream255Frames          = ntohll(pCnt->upstream255Frames);
    pCnt->downstream511Frames        = ntohll(pCnt->downstream511Frames);
    pCnt->upstream511Frames          = ntohll(pCnt->upstream511Frames);
    pCnt->downstream1023Frames       = ntohll(pCnt->downstream1023Frames);
    pCnt->upstream1023Frames         = ntohll(pCnt->upstream1023Frames);
    pCnt->downstream1518Frames       = ntohll(pCnt->downstream1518Frames);
    pCnt->upstream1518Frames         = ntohll(pCnt->upstream1518Frames);
    pCnt->downstreamDiscards         = ntohll(pCnt->downstreamDiscards);
    pCnt->upstreamDiscards           = ntohll(pCnt->upstreamDiscards);
    pCnt->downstreamErrors           = ntohll(pCnt->downstreamErrors);
    pCnt->upstreamErrors             = ntohll(pCnt->upstreamErrors);
    pCnt->statusChangeTimes          = ntohll(pCnt->statusChangeTimes);
}

// #OK
u_char oamGetPerformanceDataCurrent(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char ponif = sysObjIdx(pObj, PORT_PON);
    u_char portid = sysObjIdx(pObj, PORT_ETHER);

    if ((PORT_ERROR != ponif) || (PORT_ERROR != portid)){
        int ret;
        u_char port = portid;
        PMCounters_Ptr pPMCnt = (PMCounters_Ptr)pValue;

        if ( (OBJ_LF_PONIF == pObj->leaf) || 
             (pObj->index == OBJ_LF_PON_START_V1) ){
            port = PORT_ETHER_NUM + ponif; // PON IF 
            pPMCnt->statusChangeTimes = 0;
        }

        ret = getPMDataCurrent(port, pPMCnt);
        if (ret == FAIL)
            return VAR_NO_RESOURCE;

        htonpm(pPMCnt);
        *pLength = sizeof(PMCounters_t);
        return SUCCESS;
    }

    return VAR_BAD_PARAMETERS;
}
// #OK
u_char oamSetPerformanceDataCurrent(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char ponif = sysObjIdx(pObj, PORT_PON);
    u_char portid = sysObjIdx(pObj, PORT_ETHER);

    if ((PORT_ERROR != ponif) || (PORT_ERROR != portid)){
        int ret;
        u_char port = portid;
        PMCounters_Ptr pPMCnt = (PMCounters_Ptr)pValue;

        if ( (OBJ_LF_PONIF == pObj->leaf) || 
             (pObj->index == OBJ_LF_PON_START_V1) ){
            port = PORT_ETHER_NUM + ponif; // PON IF 
        }

        ntohpm(pPMCnt);
        ret = setPMDataCurrent(port, pPMCnt);
        if (ret == FAIL)
            return VAR_NO_RESOURCE;

        return SET_OK;
    }
    return VAR_BAD_PARAMETERS;
}
// #OK
u_char oamGetPerformanceDataHistory(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char ponif = sysObjIdx(pObj, PORT_PON);
    u_char portid = sysObjIdx(pObj, PORT_ETHER);

    if ((PORT_ERROR != ponif) || (PORT_ERROR != portid)){
        int ret;
        u_char port = portid;
        PMCounters_Ptr pPMCnt = (PMCounters_Ptr)pValue;

        if ( (OBJ_LF_PONIF == pObj->leaf) || 
             (pObj->index == OBJ_LF_PON_START_V1) ){
            port = PORT_ETHER_NUM + ponif; // PON IF 
            pPMCnt->statusChangeTimes = 0;
        }

        ret = getPMDataHistory(port, pPMCnt);
        if (ret == FAIL)
            return VAR_NO_RESOURCE;

        htonpm(pPMCnt);
        *pLength = sizeof(PMCounters_t);
        return SUCCESS;
    }

    return VAR_BAD_PARAMETERS;
}
#endif
/* -------------------------Perforemance Attributes and actions end--------------------- */

//@TODO
#ifdef TCSUPPORT_EPON_CTC_POWERSAVING
u_char oamSetSleepControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    OamCtcSleepControl_Ptr cscp = (OamCtcSleepControl_Ptr)pValue;
    eponPsIoctl_t epict;

    cscp->startTime = ntohl(cscp->startTime);
    cscp->sleepDuration = ntohl(cscp->sleepDuration);
    cscp->wakeDuration = ntohl(cscp->wakeDuration);

    eponOamExtDbg(DBG_OAM_L2, ">> SleepControl: SleepDuration=%d, "
        "WakeDuration=%d, SleepFlag=%x, SleepMode=%x\n",\
        cscp->sleepDuration, cscp->wakeDuration, cscp->sleepFlag, cscp->sleepMode);

    epict.cmdType = EPON_PS_CMD_SLP_CTL;
    epict.startTime = cscp->startTime;
    epict.sleepDuration = cscp->sleepDuration;
    epict.wakeDuration = cscp->wakeDuration;
    epict.sleepFlag = cscp->sleepFlag;
    epict.sleepMode = cscp->sleepMode;
    eponApiCtlPs(&epict);
    return RET_NOT_REPLY;
}
#else
u_char oamSetSleepControl(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
#endif


/* ------------------------------VOIP actions and attributions--------------------------- */
#ifdef TCSUPPORT_VOIP
u_char oamGetVoIPPort(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char port = sysObjIdx(pObj, PORT_VOIP);
    u_char enable;
    char  buf[5] = {0}, node[20] = {0};

    if (PORT_ERROR == port)
        return VAR_BAD_PARAMETERS;

    //sprintf(node, "VOIP_Entry%d", port);
    snprintf(node, sizeof(node), "VoIPBasic_Entry%d", port);
    if (TCAPI_SUCCESS != tcapi_get(node, "Enable", buf)){
        return VAR_NO_RESOURCE;
    }

    if (0 == strcmp(buf, "Yes")){
        enable = ACTIVE;
    }else{
        enable = DISACTIVE;
    }
    setU8(pValue, enable);
    *pLength = 1;

    return SUCCESS;
}

u_int  gVoipPort  = 0;
u_char gVoipState = 0;
int doOamSetVoIPPort(){
    char  buf[5] = {0}, node[20] = {0};
    if (PORT_ERROR == gVoipPort)
        return VAR_BAD_PARAMETERS;

    snprintf(node, sizeof(node), "VoIPBasic_Entry%d", gVoipPort);
    switch(gVoipState){
        case DISACTIVE:
            strcpy(buf, "No");
            break;
        case ACTIVE:
            strcpy(buf, "Yes");
            break;
        default:
            return VAR_BAD_PARAMETERS;
    }
    
    if (TCAPI_SUCCESS != tcapi_set(node, "Enable", buf))
        return VAR_NO_RESOURCE;

	tcapi_commit(node);	
	return 0;
}

u_char oamSetVoIPPort(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_int  port  = sysObjIdx(pObj, PORT_VOIP);
    u_char state = GetU8(pValue);
    char  buf[5] = {0}, node[20] = {0};

    if (PORT_ERROR == port)
        return VAR_BAD_PARAMETERS;

#if 1 
	gVoipPort  = port;
	gVoipState = state;
	
	postDBSetSyncMsg(DB_SYNC_VIOP_PORT_CONFIG);
#else
    snprintf(node, sizeof(node), "VoIPBasic_Entry%d", port);
    switch(state){
        case DISACTIVE:
            strcpy(buf, "No");
            break;
        case ACTIVE:
            strcpy(buf, "Yes");
            break;
        default:
            return VAR_BAD_PARAMETERS;
    }
    
    if (TCAPI_SUCCESS != tcapi_set(node, "Enable", buf))
        return VAR_NO_RESOURCE;
        
	//	tcapi_commit(VOIPBASIC);
	postDBSetSyncMsg(DB_SYNC_VOIPBASIC_COMMIT_CONFIG);
#endif
	
    return SET_OK;
}

u_char oamGetIADInfo(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    char buf[48] = {0};
    OamIADInfo_Ptr iip = (OamIADInfo_Ptr)pValue;

    memcpy(iip->mac, eponOam.eponOamLlid[0].macAddr, 6);
    //tcdbg_printf("\nIAD Info mac address = %2X:%2X:%2X:%2X:%2X:%2X", iip->mac[0], iip->mac[1],
    //  iip->mac[2],iip->mac[3],iip->mac[4],iip->mac[5]);

    memset(buf, 0, sizeof(buf));
    iip->protocolSupported = IAD_SUPPORT_SIP;
    if (TCAPI_SUCCESS != tcapi_get("VoIPAdvanced_Common", "SignalingProtocols", buf)){
        return VAR_NO_RESOURCE;
    }
    if (0 == strcmp(buf, "H.248")){
        iip->protocolSupported = IAD_SUPPORT_H248;
    }else if(0 == strcmp(buf, "MGCP")){
        iip->protocolSupported = IAD_SUPPORT_MGCP;
    }else if(0 == strcmp(buf, "IMSSIP")){
        iip->protocolSupported = IAD_SUPPORT_IMSSIP;
    }

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS != tcapi_get("Info_devDefInf", "CustomerSoftwareVersion", buf)){
        return VAR_NO_RESOURCE;
    }
    lowAlignStrcpy(iip->iadSoftwareVersion, buf, 32);

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS != tcapi_get("Info_devDefInf", "CustomerSwVersionTime", buf)){
        return VAR_NO_RESOURCE;
    }
    lowAlignStrcpy(iip->iadSoftwareTime, buf, 32);

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS != tcapi_get("EPON_ONU", "POTSPortNum", buf)){
        return VAR_NO_RESOURCE;
    }
    iip->voipUserCount = atoi(buf);

    *pLength = sizeof(OamIADInfo_t);
    return SUCCESS;
}

u_char
oamGetGlobalParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, 
                            OUT int *pLength, OUT u_char *pValue)
{  
    if ( VOIP_NODE_VALID != gCtcDB.cfg_info.voip.glbParamFlag )
        return VAR_NO_RESOURCE;

    memcpy(pValue, &gCtcDB.cfg_info.voip.glbParam, 
                    sizeof(OamGlobalParamConfig_t));
    *pLength = sizeof(OamGlobalParamConfig_t);
    return SUCCESS;
}

static inline int
checkGlbParamCfgData( OamGlobalParamConfig_Ptr pGlbParamCfg )
{
    if ( pGlbParamCfg->voiceIpMode > IP_MODE_PPPOE){
        return -1;
    }
    return 0;
}

u_char oamSetGlobalParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, 
                                   IN int length, IN u_char *pValue)
{
    if ( VOIP_NODE_INVALID == gCtcDB.cfg_info.voip.glbParamFlag )
        return VAR_NO_RESOURCE;

    if ( 0 != checkGlbParamCfgData((OamGlobalParamConfig_Ptr)pValue) )
        return VAR_BAD_PARAMETERS;

    memcpy(&gCtcDB.cfg_info.voip.glbParamCfgSet, pValue, 
                    sizeof(OamGlobalParamConfig_t));

    gCtcDB.db_sync_sflag |= DB_SYNC_FLAG_VOIP_GLB_PARAM_CFG;

    return SET_OK;
}

u_char oamGetSIPParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    char buf[20];
    OamSIPParamConfig_Ptr pSIPPrmCfg = (OamSIPParamConfig_Ptr)pValue;

    *pLength = sizeof(OamSIPParamConfig_t);

#define OAM_GET_SIP_PARAM_CFG_TCAPI_GET(item, subitem)          \
        if ( TCAPI_SUCCESS != tcapi_get(item, subitem, buf) )   \
            return VAR_NO_RESOURCE
    
#define OAM_GET_SIP_PARAM_CFG_GET_INTEGER(suffix, member, item, subitem)        \
        OAM_GET_SIP_PARAM_CFG_TCAPI_GET(item, subitem);                         \
        pSIPPrmCfg->member = hton##suffix(atoi(buf))
        
#define OAM_GET_SIP_PARAM_CFG_GET_SHORT(member, item, subitem)      \
        OAM_GET_SIP_PARAM_CFG_GET_INTEGER(s, member, item, subitem)
    
#define OAM_GET_SIP_PARAM_CFG_GET_LONG(member, item, subitem)       \
        OAM_GET_SIP_PARAM_CFG_GET_INTEGER(l, member, item, subitem)
    
#define OAM_GET_SIP_PARAM_CFG_GET_IP(member, item, subitem)    \
        OAM_GET_SIP_PARAM_CFG_TCAPI_GET(item, subitem);        \
        pSIPPrmCfg->member = inet_addr(buf)


    char entryNode[20];
    u_char portid = sysObjIdx(pObj, PORT_VOIP);

    if (PORT_ERROR == portid)
        return VAR_BAD_PARAMETERS;

    sprintf(entryNode, VOIPBASIC_ENTRY, portid);

    OAM_GET_SIP_PARAM_CFG_GET_SHORT ( mgPort                    ,   VOIPBASIC_COMMON   ,    "LocalSIPPort"          );
    OAM_GET_SIP_PARAM_CFG_GET_IP    ( sipProxyServIp            ,   VOIPBASIC_COMMON   ,    "SIPProxyAddr"          );
    OAM_GET_SIP_PARAM_CFG_GET_SHORT ( sipProxyServPort          ,   VOIPBASIC_COMMON   ,    "SIPProxyPort"          );
    OAM_GET_SIP_PARAM_CFG_GET_IP    ( backupSipProxyServIp      ,   VOIPBASIC_COMMON   ,    "SBSIPProxyAddr"        );
    OAM_GET_SIP_PARAM_CFG_GET_SHORT ( backupSipProxyServPort    ,   VOIPBASIC_COMMON   ,    "SIPProxyBackupPort"    );
    OAM_GET_SIP_PARAM_CFG_GET_IP    ( sipRegServIp              ,   VOIPBASIC_COMMON   ,    "RegistrarServer"       );
    OAM_GET_SIP_PARAM_CFG_GET_SHORT ( sipRegServPort            ,   VOIPBASIC_COMMON   ,    "RegistrarServerPort"   );
    OAM_GET_SIP_PARAM_CFG_GET_IP    ( backupSipRegServIp        ,   VOIPBASIC_COMMON   ,    "SBRegistrarServer"     );
    OAM_GET_SIP_PARAM_CFG_GET_SHORT ( backupSipRegServPort      ,   VOIPBASIC_COMMON   ,    "SBRegistrarServerPort" );
    OAM_GET_SIP_PARAM_CFG_GET_IP    ( outBoundServIp            ,   VOIPBASIC_COMMON   ,    "SIPOutboundProxyAddr"  );
    OAM_GET_SIP_PARAM_CFG_GET_SHORT ( outBoundServPort          ,   VOIPBASIC_COMMON   ,    "SIPOutboundProxyPort"  );
    OAM_GET_SIP_PARAM_CFG_GET_LONG  ( sipRegInterval            ,   entryNode          ,    "RegisterExpires"       );
    OAM_GET_SIP_PARAM_CFG_GET_SHORT ( heartbeatCycle            ,   VOIPBASIC_COMMON   ,    "HeartbeatCycle"        );
    OAM_GET_SIP_PARAM_CFG_GET_SHORT ( heartbeatCount            ,   VOIPBASIC_COMMON   ,    "HeartbeatCount"        );
    
    pSIPPrmCfg->activeSipProxyServIp = pSIPPrmCfg->sipProxyServIp;

    OAM_GET_SIP_PARAM_CFG_TCAPI_GET(VOIPBASIC_COMMON, "HeartbeatSwitch");
    if (0 == strcmp(buf, "Yes"))
        pSIPPrmCfg->heartbeatSwitch = HEARTBEAT_ENABLE;
    else
        pSIPPrmCfg->heartbeatSwitch = HEARTBEAT_DISABLE;

#undef OAM_GET_SIP_PARAM_CFG_TCAPI_GET
#undef OAM_GET_SIP_PARAM_CFG_GET_INTEGER
#undef OAM_GET_SIP_PARAM_CFG_GET_SHORT
#undef OAM_GET_SIP_PARAM_CFG_GET_LONG
#undef OAM_GET_SIP_PARAM_CFG_GET_IP

    return SUCCESS;
}

#if 1
OamSIPParamConfig_t gSIPPrmCfg = {0};
u_char gSIPPortid = 0;
int doOamSetSIPParameterConfig(){	
    char buf[20];
    struct in_addr addr;
	char entryNode[20];
#define OAM_SET_SIP_PARAM_CONFIG_GET_IP_STRING(member)      \
		addr.s_addr = ntohl(gSIPPrmCfg.member);			\
		sprintf(buf, "%s", inet_ntoa(addr))
	
#define OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET(item, subitem)   \
		if (TCAPI_SUCCESS != tcapi_set(item, subitem, buf)) \
			return VAR_NO_RESOURCE
	
#define OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP(member, item, subitem) \
		OAM_SET_SIP_PARAM_CONFIG_GET_IP_STRING(member); 			 \
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET(item, subitem)
	
#define OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT(member, item, subitem)  \
		sprintf(buf, "%d", ntohs(gSIPPrmCfg.member));				   \
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET(item, subitem)
	
		sprintf(entryNode, VOIPBASIC_ENTRY, gSIPPortid);
	
	
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( mgPort 			   , VOIPBASIC_COMMON	, "LocalSIPPort"		  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( sipProxyServIp 	   , VOIPBASIC_COMMON	, "SIPProxyAddr"		  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( sipProxyServPort	   , VOIPBASIC_COMMON	, "SIPProxyPort"		  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( backupSipProxyServIp  , VOIPBASIC_COMMON	, "SBSIPProxyAddr"		  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( backupSipProxyServPort, VOIPBASIC_COMMON	, "SBSIPProxyPort"		  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( sipRegServIp		   , VOIPBASIC_COMMON	, "RegistrarServer" 	  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( sipRegServPort 	   , VOIPBASIC_COMMON	, "RegistrarServerPort"   );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( backupSipRegServIp    , VOIPBASIC_COMMON	, "SBRegistrarServer"	  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( backupSipRegServPort  , VOIPBASIC_COMMON	, "SBRegistrarServerPort" );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( outBoundServIp 	   , VOIPBASIC_COMMON	, "SIPOutboundProxyAddr"  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( outBoundServPort	   , VOIPBASIC_COMMON	, "SIPOutboundProxyPort"  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( sipRegInterval 	   , entryNode			, "RegisterExpires" 	  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( heartbeatSwitch	   , VOIPBASIC_COMMON	, "HeartbeatSwitch" 	  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( heartbeatCycle 	   , VOIPBASIC_COMMON	, "HeartbeatCycle"		  );
		OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( heartbeatCount 	   , VOIPBASIC_COMMON	, "HeartbeatCount"		  );
	
		if (TCAPI_SUCCESS != tcapi_commit(VOIPBASIC)){
			return VAR_NO_RESOURCE;
		}
		
		if (TCAPI_SUCCESS != tcapi_commit(VOIPADVANCED)){
			return VAR_NO_RESOURCE;
		}
		
		if (TCAPI_SUCCESS != tcapi_save()){
			return VAR_NO_RESOURCE;
		} 
		
#undef OAM_SET_SIP_PARAM_CONFIG_GET_IP_STRING
#undef OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET
#undef OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP
#undef OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT

	return 0;

}
#endif
u_char oamSetSIPParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    char buf[20];
    struct in_addr addr;
    OamSIPParamConfig_Ptr pSIPPrmCfg = (OamSIPParamConfig_Ptr)pValue;
    char entryNode[20];
    u_char portid = sysObjIdx(pObj, PORT_VOIP);

    if (PORT_ERROR == portid)
        return VAR_BAD_PARAMETERS;

#if 1
		gSIPPortid = portid;
		gSIPPrmCfg.mgPort = pSIPPrmCfg->mgPort;
		gSIPPrmCfg.sipProxyServIp = pSIPPrmCfg->sipProxyServIp;
		gSIPPrmCfg.sipProxyServPort = pSIPPrmCfg->sipProxyServPort;
		gSIPPrmCfg.backupSipProxyServIp = pSIPPrmCfg->backupSipProxyServIp;
		gSIPPrmCfg.backupSipProxyServPort = pSIPPrmCfg->backupSipProxyServPort;
		gSIPPrmCfg.sipRegServIp = pSIPPrmCfg->sipRegServIp;
		gSIPPrmCfg.sipRegServPort = pSIPPrmCfg->sipRegServPort;
		gSIPPrmCfg.backupSipRegServIp = pSIPPrmCfg->backupSipRegServIp;
		gSIPPrmCfg.backupSipRegServPort = pSIPPrmCfg->backupSipRegServPort;
		gSIPPrmCfg.outBoundServIp = pSIPPrmCfg->outBoundServIp;
		gSIPPrmCfg.outBoundServPort = pSIPPrmCfg->outBoundServPort;
		gSIPPrmCfg.sipRegInterval = pSIPPrmCfg->sipRegInterval;		
		gSIPPrmCfg.heartbeatSwitch = pSIPPrmCfg->heartbeatSwitch;
		gSIPPrmCfg.heartbeatCycle = pSIPPrmCfg->heartbeatCycle;
		gSIPPrmCfg.heartbeatCount = pSIPPrmCfg->heartbeatCount;
		postDBSetSyncMsg(DB_SYNC_SIP_PARAMETER_CONFIG);
#else
#define OAM_SET_SIP_PARAM_CONFIG_GET_IP_STRING(member)      \
        addr.s_addr = ntohl(pSIPPrmCfg->member);            \
        sprintf(buf, "%s", inet_ntoa(addr))
    
#define OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET(item, subitem)   \
        if (TCAPI_SUCCESS != tcapi_set(item, subitem, buf)) \
            return VAR_NO_RESOURCE
    
#define OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP(member, item, subitem) \
        OAM_SET_SIP_PARAM_CONFIG_GET_IP_STRING(member);              \
        OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET(item, subitem)
    
#define OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT(member, item, subitem)  \
        sprintf(buf, "%d", ntohs(pSIPPrmCfg->member));                 \
        OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET(item, subitem)


    sprintf(entryNode, VOIPBASIC_ENTRY, portid);


    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( mgPort                , VOIPBASIC_COMMON   , "LocalSIPPort"          );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( sipProxyServIp        , VOIPBASIC_COMMON   , "SIPProxyAddr"          );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( sipProxyServPort      , VOIPBASIC_COMMON   , "SIPProxyPort"          );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( backupSipProxyServIp  , VOIPBASIC_COMMON   , "SBSIPProxyAddr"        );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( backupSipProxyServPort, VOIPBASIC_COMMON   , "SBSIPProxyPort"        );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( sipRegServIp          , VOIPBASIC_COMMON   , "RegistrarServer"       );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( sipRegServPort        , VOIPBASIC_COMMON   , "RegistrarServerPort"   );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( backupSipRegServIp    , VOIPBASIC_COMMON   , "SBRegistrarServer"     );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( backupSipRegServPort  , VOIPBASIC_COMMON   , "SBRegistrarServerPort" );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP  ( outBoundServIp        , VOIPBASIC_COMMON   , "SIPOutboundProxyAddr"  );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( outBoundServPort      , VOIPBASIC_COMMON   , "SIPOutboundProxyPort"  );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( sipRegInterval        , entryNode          , "RegisterExpires"       );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( heartbeatSwitch       , VOIPBASIC_COMMON   , "HeartbeatSwitch"       );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( heartbeatCycle        , VOIPBASIC_COMMON   , "HeartbeatCycle"        );
    OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT ( heartbeatCount        , VOIPBASIC_COMMON   , "HeartbeatCount"        );

#if 0
    if (TCAPI_SUCCESS != tcapi_commit(VOIPBASIC)){
        return VAR_NO_RESOURCE;
    }
    
    if (TCAPI_SUCCESS != tcapi_commit(VOIPADVANCED)){
        return VAR_NO_RESOURCE;
    }
    
    if (TCAPI_SUCCESS != tcapi_save()){
        return VAR_NO_RESOURCE;
    } 
#else	
	postDBSetSyncMsg(DB_SYNC_VOIPBASIC_COMMIT_CONFIG|DB_SYNC_VOIPADV_COMMIT_CONFIG|DB_SYNC_SAVE_CONFIG);
#endif
    
#undef OAM_SET_SIP_PARAM_CONFIG_GET_IP_STRING
#undef OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET
#undef OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_IP
#undef OAM_SET_SIP_PARAM_CONFIG_TCAPI_SET_INT
#endif

    return SET_OK;
}

u_char oamGetSIPUserParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    char  buf[33] = {0}, node[40] = {0};
    u_char portid = sysObjIdx(pObj, PORT_VOIP);
    OamSIPUserParamConfig_Ptr pSIPUsrPrmCfg = (OamSIPUserParamConfig_Ptr)pValue;

    if (PORT_ERROR == portid)
        return VAR_BAD_PARAMETERS;

    *pLength = sizeof(OamSIPUserParamConfig_t);

    sprintf(node, VOIPBASIC_ENTRY, portid);
    if (TCAPI_SUCCESS == tcapi_get(node, "SIPDisplayName", buf)){
        strcpy(pSIPUsrPrmCfg->userAccount, buf);
    }else{
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, 33);
    if (TCAPI_SUCCESS == tcapi_get(node, "SIPAuthenticationName", buf)){
        strcpy(pSIPUsrPrmCfg->userName, buf);
    }else{
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, 33);
    if (TCAPI_SUCCESS == tcapi_get(node, "SIPPassword", buf)){
        strcpy(pSIPUsrPrmCfg->userPassword, buf);
    }else{
        return VAR_NO_RESOURCE;
    }

    return SUCCESS;
}


u_int  gSIPUserVoipPort  = 0;
OamSIPUserParamConfig_t gSIPUsrPrmCfg = {0};
int doOamSetSIPUserParameterConfig(){
	char node[40] = {0};

	if (PORT_ERROR == gSIPUserVoipPort)
		return VAR_BAD_PARAMETERS;

	sprintf(node, VOIPBASIC_ENTRY, gSIPUserVoipPort);

	if (0 != tcapi_set(node, "SIPDisplayName", gSIPUsrPrmCfg.userAccount)){
		return VAR_NO_RESOURCE;
	}

	if (0 != tcapi_set(node, "SIPAuthenticationName", gSIPUsrPrmCfg.userName)){
		return VAR_NO_RESOURCE;
	}

	if (0 != tcapi_set(node, "SIPPassword", gSIPUsrPrmCfg.userPassword)){
		return VAR_NO_RESOURCE;
	}

	tcapi_commit(node);	
	return 0;
}

u_char oamSetSIPUserParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    char node[40] = {0};
    u_char portid = sysObjIdx(pObj, PORT_VOIP);
    OamSIPUserParamConfig_Ptr pSIPUsrPrmCfg = (OamSIPUserParamConfig_Ptr)pValue;

    if (PORT_ERROR == portid)
        return VAR_BAD_PARAMETERS;


#if 1
	gSIPUserVoipPort = portid;

	strcpy(gSIPUsrPrmCfg.userAccount, pSIPUsrPrmCfg->userAccount);
	strcpy(gSIPUsrPrmCfg.userName, pSIPUsrPrmCfg->userName);
	strcpy(gSIPUsrPrmCfg.userPassword, pSIPUsrPrmCfg->userPassword);

	postDBSetSyncMsg(DB_SYNC_SIP_USER_PARAMETER_CONFIG);
#else
    sprintf(node, VOIPBASIC_ENTRY, portid);

    if (0 != tcapi_set(node, "SIPDisplayName", pSIPUsrPrmCfg->userAccount)){
        return VAR_NO_RESOURCE;
    }

    if (0 != tcapi_set(node, "SIPAuthenticationName", pSIPUsrPrmCfg->userName)){
        return VAR_NO_RESOURCE;
    }

    if (0 != tcapi_set(node, "SIPPassword", pSIPUsrPrmCfg->userPassword)){
        return VAR_NO_RESOURCE;
    }

#if 0	
    if(tcapi_commit(node) != 0){
        eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
        return VAR_NO_RESOURCE;
    }
#else
	postDBSetSyncMsg(DB_SYNC_VOIPBASIC_COMMIT_CONFIG);
#endif
#endif

    return SET_OK;
}

u_char oamGetFaxModemConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    char buf[10] = {0};
    OamFaxModemConfig_Ptr pFaxMdmCfg = (OamFaxModemConfig_Ptr)pValue;

    *pLength = sizeof(OamFaxModemConfig_t);

    if (TCAPI_SUCCESS == tcapi_get(VOIPMEDIA_COMMON, "T38Enable", buf)){
        if (strcmp(buf, "Yes")){
            pFaxMdmCfg->voiceT38Enable = VOICE_T38;
        }else{
            pFaxMdmCfg->voiceT38Enable = VOICE_T30;
        }
    }else{
        return VAR_NO_RESOURCE;
    }

    pFaxMdmCfg->voiceControl = CONSULT;

    return SUCCESS;
}

OamFaxModemConfig_t gFaxMdmCfg = {0}; 
int doOamSetFaxModemConfig(){	
    char buf[10] = {0};
	
    if (gFaxMdmCfg.voiceT38Enable == VOICE_T38){
        sprintf(buf, "Yes");
    }else if (gFaxMdmCfg.voiceT38Enable == VOICE_T30){
        sprintf(buf, "No");
    }else{
        return RET_ONLY_SKIP;// ? default T30
    }

    if (0 != tcapi_set(VOIPMEDIA_COMMON, "T38Enable", buf)){
        return VAR_NO_RESOURCE;
    }
	
	return 0;
}

u_char oamSetFaxModemConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    char buf[10] = {0};
    OamFaxModemConfig_Ptr pFaxMdmCfg = (OamFaxModemConfig_Ptr)pValue;

#if 1
	gFaxMdmCfg.voiceT38Enable = pFaxMdmCfg->voiceT38Enable;
	postDBSetSyncMsg(DB_SYNC_FAX_MODEM_CONFIG);
#else
    if (pFaxMdmCfg->voiceT38Enable == VOICE_T38){
        sprintf(buf, "Yes");
    }else if (pFaxMdmCfg->voiceT38Enable == VOICE_T30){
        sprintf(buf, "No");
    }else{
        return RET_ONLY_SKIP;// ? default T30
    }

    if (0 != tcapi_set(VOIPMEDIA_COMMON, "T38Enable", buf)){
        return VAR_NO_RESOURCE;
    }
#endif

    return SET_OK;
}

u_char oamGetH248ParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    char  buf[65] = {0}, node[40] = {0};
    OamH248ParamConfig_Ptr pH248PrmCfg = (OamH248ParamConfig_Ptr)pValue;

    if(pH248PrmCfg == NULL)
            return VAR_NO_RESOURCE;

    *pLength = sizeof(OamH248ParamConfig_t);

    strncpy(node, VOIPH248_COMMON, strlen(VOIPH248_COMMON));
    if (TCAPI_SUCCESS == tcapi_get(node, "MediaGatewayPort", buf)){
        pH248PrmCfg->MGPortNo = htons(atoi(buf));
    }else{
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "MediaGatewayControler", buf)){
        if(0 == strlen(buf)){
            pH248PrmCfg->MGCIP =  0;
        }else{
            pH248PrmCfg->MGCIP =  inet_addr(buf);       
        }
    }else{
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "MediaGatewayControlerPort", buf)){
        pH248PrmCfg->MgcComPortNo= htons(atoi(buf));
    }else{
        return VAR_NO_RESOURCE;
    }
    
    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "SBMediaGatewayControler", buf)){
        if(0 == strlen(buf)){
            pH248PrmCfg->BackupMgcIp =  0;
        }else{
            pH248PrmCfg->BackupMgcIp = inet_addr(buf);  
        }
    }else{
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "SBMediaGatewayControlerPort", buf)){
        pH248PrmCfg->BackupMgcComPortNo = htons(atoi(buf));
    }else{
        return VAR_NO_RESOURCE;
    }   

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "ActiveMGC", buf)){
        pH248PrmCfg->ActiveMGC = (u_char)atoi(buf);
    }else{
        pH248PrmCfg->ActiveMGC = 0x01;
    }

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "DeviceIDType", buf)){
        pH248PrmCfg->RegMode = htons(atoi(buf));
    }else{
        return VAR_NO_RESOURCE;
    }   

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "DeviceID", pH248PrmCfg->MID)){
        //strcpy(pH248PrmCfg->MID, buf);
    }else{
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "HeartbeatMode", buf)){
        pH248PrmCfg->HeartbeatMode = htons(atoi(buf));
    }else{
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "HeartBeatTimer", buf)){
        pH248PrmCfg->HeartbeatCycle = htons(atoi(buf));
    }else{
        return VAR_NO_RESOURCE;
    }
    
    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, "HeartBeatRetransTimes", buf)){
        pH248PrmCfg->HeartbeatCount = htons(atoi(buf));
    }else{
        return VAR_NO_RESOURCE;
    }

    return SUCCESS;
}

OamH248ParamConfig_t gH248PrmCfg = {0};
int doOamSetH248ParameterConfig(){
	char node[40] = {0}, buf[65] = {0};
	struct in_addr addr;

	strncpy(node, VOIPH248_COMMON, strlen(VOIPH248_COMMON));
	sprintf(buf, "%d", ntohs(gH248PrmCfg.MGPortNo));
	if (0 != tcapi_set(node, "MediaGatewayPort", buf)){
		return VAR_NO_RESOURCE;
	}


	memset(buf, 0, sizeof(buf));
	addr.s_addr = ntohl(gH248PrmCfg.MGCIP);
	sprintf(buf, "%s", inet_ntoa(addr));
	if (0 != tcapi_set(node, "MediaGatewayControler", buf)){
		return VAR_NO_RESOURCE;
	}

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248PrmCfg.MgcComPortNo));
	if (0 != tcapi_set(node, "MediaGatewayControlerPort", buf)){
		return VAR_NO_RESOURCE;
	}	

	memset(buf, 0, sizeof(buf));
	addr.s_addr = ntohl(gH248PrmCfg.BackupMgcIp);
	sprintf(buf, "%s", inet_ntoa(addr));
	if (0 != tcapi_set(node, "SBMediaGatewayControler", buf)){
		return VAR_NO_RESOURCE;
	}

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248PrmCfg.BackupMgcComPortNo));
	if (0 != tcapi_set(node, "SBMediaGatewayControlerPort", buf)){
		return VAR_NO_RESOURCE;
	}	

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248PrmCfg.RegMode));
	if (0 != tcapi_set(node, "DeviceIDType", buf)){
		return VAR_NO_RESOURCE;
	}
	if(gH248PrmCfg.RegMode == 0x00){
		if (0 != tcapi_set(node, "DeviceID", "")){
			return VAR_NO_RESOURCE;
		}
	}else{
		if (0 != tcapi_set(node, "DeviceID", gH248PrmCfg.MID)){
			return VAR_NO_RESOURCE;
		}
	}

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248PrmCfg.HeartbeatMode));
	if (0 != tcapi_set(node, "HeartbeatMode", buf)){
		return VAR_NO_RESOURCE;
	}

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248PrmCfg.HeartbeatCycle));
	if (0 != tcapi_set(node, "HeartBeatTimer", buf)){
		return VAR_NO_RESOURCE;
	}	

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248PrmCfg.HeartbeatCount));
	if (0 != tcapi_set(node, "HeartBeatRetransTimes", buf)){
		return VAR_NO_RESOURCE;
	}		
	if(tcapi_commit(node) != 0){
		eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		return VAR_NO_RESOURCE;
	}

	return 0;
}

u_char oamSetH248ParameterConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    char node[40] = {0}, buf[65] = {0};
    struct in_addr addr;
    OamH248ParamConfig_Ptr pH248PrmCfg = (OamH248ParamConfig_Ptr)pValue;

    if(pH248PrmCfg == NULL)
        return VAR_NO_RESOURCE;

#if 1
	gH248PrmCfg.MGPortNo = pH248PrmCfg->MGPortNo;
	gH248PrmCfg.MGCIP = pH248PrmCfg->MGCIP;
	gH248PrmCfg.MgcComPortNo = pH248PrmCfg->MgcComPortNo;
	gH248PrmCfg.BackupMgcIp = pH248PrmCfg->BackupMgcIp;	
	gH248PrmCfg.BackupMgcComPortNo = pH248PrmCfg->BackupMgcComPortNo;	
	gH248PrmCfg.ActiveMGC = pH248PrmCfg->ActiveMGC;
	gH248PrmCfg.RegMode = pH248PrmCfg->RegMode;
	gH248PrmCfg.HeartbeatMode = pH248PrmCfg->HeartbeatMode;	
	gH248PrmCfg.HeartbeatCycle = pH248PrmCfg->HeartbeatCycle;	
	gH248PrmCfg.HeartbeatCount = pH248PrmCfg->HeartbeatCount;	
	strcpy(gH248PrmCfg.MID, pH248PrmCfg->MID);

	
	postDBSetSyncMsg(DB_SYNC_H248_PARAMETER_CONFIG);
#else

    strncpy(node, VOIPH248_COMMON, strlen(VOIPH248_COMMON));
    sprintf(buf, "%d", ntohs(pH248PrmCfg->MGPortNo));
    if (0 != tcapi_set(node, "MediaGatewayPort", buf)){
        return VAR_NO_RESOURCE;
    }


    memset(buf, 0, sizeof(buf));
    addr.s_addr = ntohl(pH248PrmCfg->MGCIP);
    sprintf(buf, "%s", inet_ntoa(addr));
    if (0 != tcapi_set(node, "MediaGatewayControler", buf)){
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248PrmCfg->MgcComPortNo));
    if (0 != tcapi_set(node, "MediaGatewayControlerPort", buf)){
        return VAR_NO_RESOURCE;
    }   

    memset(buf, 0, sizeof(buf));
    addr.s_addr = ntohl(pH248PrmCfg->BackupMgcIp);
    sprintf(buf, "%s", inet_ntoa(addr));
    if (0 != tcapi_set(node, "SBMediaGatewayControler", buf)){
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248PrmCfg->BackupMgcComPortNo));
    if (0 != tcapi_set(node, "SBMediaGatewayControlerPort", buf)){
        return VAR_NO_RESOURCE;
    }   
/*
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248PrmCfg->ActiveMGC));
    if (0 != tcapi_set(node, "ActiveMGC", buf)){
        return VAR_NO_RESOURCE;
    }*/
    
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248PrmCfg->RegMode));
    if (0 != tcapi_set(node, "DeviceIDType", buf)){
        return VAR_NO_RESOURCE;
    }
    if(pH248PrmCfg->RegMode == 0x00){
        if (0 != tcapi_set(node, "DeviceID", "")){
            return VAR_NO_RESOURCE;
        }
    }else{
        if (0 != tcapi_set(node, "DeviceID", pH248PrmCfg->MID)){
            return VAR_NO_RESOURCE;
        }
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248PrmCfg->HeartbeatMode));
    if (0 != tcapi_set(node, "HeartbeatMode", buf)){
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248PrmCfg->HeartbeatCycle));
    if (0 != tcapi_set(node, "HeartBeatTimer", buf)){
        return VAR_NO_RESOURCE;
    }   

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248PrmCfg->HeartbeatCount));
    if (0 != tcapi_set(node, "HeartBeatRetransTimes", buf)){
        return VAR_NO_RESOURCE;
    }       
#if 0
    if(tcapi_commit(node) != 0){
        eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
        return VAR_NO_RESOURCE;
    }
#else	
	postDBSetSyncMsg(DB_SYNC_VOIPH248_COMMON_COMMIT_CONFIG);
#endif
#endif

    return SET_OK;
}

u_char oamGetH248UserTIDInfo(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    char  node[40] = {0};
    OamH248UserTIDInfo_Ptr pH248UsrTIDInfo = (OamH248UserTIDInfo_Ptr)pValue;

    if(pH248UsrTIDInfo == NULL)
        return VAR_NO_RESOURCE;
    
    *pLength = sizeof(OamH248UserTIDInfo_t);

    strncpy(node, VOIPH248_COMMON, strlen(VOIPH248_COMMON));

    if (TCAPI_SUCCESS == tcapi_get(node, "UserTIDName", pH248UsrTIDInfo->UserTIDName)){
    }else{
        return VAR_NO_RESOURCE;
    }

    return SUCCESS;
}

OamH248UserTIDInfo_t gH248UsrTIDInfo = {0};
int doOamSetH248UserTIDInfo(){
    char node[40] = {0};
	strncpy(node, VOIPH248_COMMON, strlen(VOIPH248_COMMON));

	if (0 != tcapi_set(node, "UserTIDName", gH248UsrTIDInfo.UserTIDName)){
		return VAR_NO_RESOURCE;
	}

	if(tcapi_commit(node) != 0){
		eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		return VAR_NO_RESOURCE;
	}

	return 0;
}

u_char oamSetH248UserTIDInfo(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    char node[40] = {0};
    OamH248UserTIDInfo_Ptr pH248UsrTIDInfo = (OamH248UserTIDInfo_Ptr)pValue;    

    if(pH248UsrTIDInfo == NULL)
        return VAR_NO_RESOURCE;
    
#if 1
	strcpy(gH248UsrTIDInfo.UserTIDName, pH248UsrTIDInfo->UserTIDName);
	postDBSetSyncMsg(DB_SYNC_H248_USER_TID_CONFIG);
#else
    strncpy(node, VOIPH248_COMMON, strlen(VOIPH248_COMMON));

    if (0 != tcapi_set(node, "UserTIDName", pH248UsrTIDInfo->UserTIDName)){
        return VAR_NO_RESOURCE;
    }

#if 0    
    if(tcapi_commit(node) != 0){
        eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
        return VAR_NO_RESOURCE;
    }
#else
	postDBSetSyncMsg(DB_SYNC_VOIPH248_COMMON_COMMIT_CONFIG);
#endif
#endif

    return SET_OK;
}


OamH248RTPTIDConfig_t gH248RTPTIDCfg = {0};

int doOamSetH248RTPTIDConfig(){
	char node[40] = {0}, buf[20] = {0};
	
	strncpy(node, VOIPH248_COMMON, strlen(VOIPH248_COMMON));
   
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248RTPTIDCfg.RTPTIDPrefix));
	if (0 != tcapi_set(node, "RTPPrefix", buf)){
		return VAR_NO_RESOURCE;
	}
	
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248RTPTIDCfg.RTPTIDDigitBegin));
	if (0 != tcapi_set(node, "EphemeralTermIDStart", buf)){
		return VAR_NO_RESOURCE;
	}	
	
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248RTPTIDCfg.RTPTIDMode));
	if (0 != tcapi_set(node, "EphemeralTermIDUniform", buf)){
		return VAR_NO_RESOURCE;
	}
	
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", ntohs(gH248RTPTIDCfg.RTPTIDDigitLen));
	if (0 != tcapi_set(node, "PhysicalTermIDAddLen", buf)){
		return VAR_NO_RESOURCE;
	}		

	if(tcapi_commit(node) != 0){
		eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		return VAR_NO_RESOURCE;
	}

	return 0;
}

u_char oamSetH248RTPTIDConfig(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    char node[40] = {0}, buf[20] = {0};
    OamH248RTPTIDConfig_Ptr pH248RTPTIDCfg = (OamH248RTPTIDConfig_Ptr)pValue;   

    if(pH248RTPTIDCfg == NULL)
        return VAR_NO_RESOURCE;
    

#if 1
	strcpy(gH248RTPTIDCfg.RTPTIDPrefix, pH248RTPTIDCfg->RTPTIDPrefix);
	strcpy(gH248RTPTIDCfg.RTPTIDDigitBegin, pH248RTPTIDCfg->RTPTIDDigitBegin);
	gH248RTPTIDCfg.RTPTIDMode = pH248RTPTIDCfg->RTPTIDMode;
	gH248RTPTIDCfg.RTPTIDDigitLen = pH248RTPTIDCfg->RTPTIDDigitLen;
	postDBSetSyncMsg(DB_SYNC_H248_RTP_TID_CONFIG);	
#else
    strncpy(node, VOIPH248_COMMON, strlen(VOIPH248_COMMON));
   
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248RTPTIDCfg->RTPTIDPrefix));
    if (0 != tcapi_set(node, "RTPPrefix", buf)){
        return VAR_NO_RESOURCE;
    }
    
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248RTPTIDCfg->RTPTIDDigitBegin));
    if (0 != tcapi_set(node, "EphemeralTermIDStart", buf)){
        return VAR_NO_RESOURCE;
    }   
    
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248RTPTIDCfg->RTPTIDMode));
    if (0 != tcapi_set(node, "EphemeralTermIDUniform", buf)){
        return VAR_NO_RESOURCE;
    }
    
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", ntohs(pH248RTPTIDCfg->RTPTIDDigitLen));
    if (0 != tcapi_set(node, "PhysicalTermIDAddLen", buf)){
        return VAR_NO_RESOURCE;
    }       

#if 0	
    if(tcapi_commit(node) != 0){
        eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
        return VAR_NO_RESOURCE;
    }
#else
	postDBSetSyncMsg(DB_SYNC_VOIPH248_COMMON_COMMIT_CONFIG);
#endif
#endif

    return SET_OK;


}

u_char
oamGetPOTSStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, 
                 OUT  int *pLength, OUT u_char *pValue)
{
    u_char idx = sysObjIdx(pObj, PORT_VOIP);
    char buf[32] = {0};

    if (PORT_ERROR == idx ||  NULL == pLength || NULL == pValue)
        return VAR_BAD_PARAMETERS;	

    OamCtcDBVoipPortConf_t * pPort = &gCtcDB.cfg_info.voip.port[idx];

    /* if VoIPLine2Enable is not set , only idx 0 is used */
    if (0 != idx)
    {
        tcapi_get(VOIPBASIC_COMMON,"VoIPLine2Enable",buf);
        if (0 != strcmp(buf,"Yes"))
            return VAR_NO_RESOURCE;
    }

    if ( VOIP_NODE_VALID != pPort->potsStatusFlag){
        return VAR_NO_RESOURCE;
    }

    memcpy( pValue, &pPort->potsStatus, sizeof(OamPOTSStatus_t) );
    
    *pLength = sizeof(OamPOTSStatus_t);
    
    return SUCCESS;
}

//@TODO
u_char oamSetIADOperation(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_int iadOperation = GetU32(pValue);
    switch(iadOperation){
        case IAD_OPER_REREGISTER:
        case IAD_OPER_LOGOUT:
        case IAD_OPER_RESET:
        if (SUCCESS != ONUIadOperate(iadOperation))
            return VAR_NO_RESOURCE;
        break;
        default:
        return VAR_BAD_PARAMETERS;
    }
    return SET_OK;
}

#ifdef TCSUPPORT_VOIP
u_char oamGetTimerCfg(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    char  buf[33] = {0}, node[40] = {0};
    oamTimeCfg_Ptr timerCfg_ptr = (oamTimeCfg_Ptr)pValue;

    *pLength = sizeof(oamTimeCfg_t);
    strcpy(node, VOIP_DIGIT_MAP_ENTRY);
    if (TCAPI_SUCCESS == tcapi_get(node, TIME_CFG_DML_NAME, buf)){
        timerCfg_ptr->dml = atoi(buf);
    }else{
        return VAR_NO_RESOURCE;
    }

    memset(buf, 0, sizeof(buf));
    if (TCAPI_SUCCESS == tcapi_get(node, TIME_CFG_DMS_NAME, buf)){
        timerCfg_ptr->dms = atoi(buf);
    }else{
        return VAR_NO_RESOURCE;
    }

    /*return  default value for dml or dms*/
    if((timerCfg_ptr->dml <TIME_CFG_DMLS_MIN_VAL) || (timerCfg_ptr->dml >TIME_CFG_DMLS_MAX_VAL)){
        timerCfg_ptr->dml = TIME_CFG_DML_DEF_VAL;
    }
    if((timerCfg_ptr->dms <TIME_CFG_DMLS_MIN_VAL) || (timerCfg_ptr->dms >TIME_CFG_DMLS_MAX_VAL)){
        timerCfg_ptr->dms = TIME_CFG_DMS_DEF_VAL;
    }
    
    return SUCCESS;
}

oamTimeCfg_t gTimerCfg = {0};
int doOamSetTimerCfg(){
	char  buf[33] = {0}, node[40] = {0};

	strcpy(node, VOIP_DIGIT_MAP_ENTRY);

	sprintf(buf, "%d", gTimerCfg.dml);
	if (0 != tcapi_set(node, TIME_CFG_DML_NAME, buf)){
		return VAR_NO_RESOURCE;
	}
	
	sprintf(buf, "%d", gTimerCfg.dms);
	if (0 != tcapi_set(node, TIME_CFG_DMS_NAME, buf)){
		return VAR_NO_RESOURCE;
	}

	if(tcapi_commit(node) != 0){
		eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		return VAR_NO_RESOURCE;
	}

	return 0;

}


u_char oamSetTimerCfg(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    char  buf[33] = {0}, node[40] = {0};
    oamTimeCfg_Ptr timerCfg_ptr = (oamTimeCfg_Ptr)pValue;
    
    if((timerCfg_ptr->dml <TIME_CFG_DMLS_MIN_VAL) || (timerCfg_ptr->dml >TIME_CFG_DMLS_MAX_VAL)){
        return VAR_BAD_PARAMETERS;
    }
    if((timerCfg_ptr->dms <TIME_CFG_DMLS_MIN_VAL) || (timerCfg_ptr->dms >TIME_CFG_DMLS_MAX_VAL)){
        return VAR_BAD_PARAMETERS;
    }

#if 1
	gTimerCfg.dml = timerCfg_ptr->dml;
	gTimerCfg.dms = timerCfg_ptr->dms;

	postDBSetSyncMsg(DB_SYNC_SET_TIMER_CONFIG);
#else
    strcpy(node, VOIP_DIGIT_MAP_ENTRY);

    sprintf(buf, "%d", timerCfg_ptr->dml);
    if (0 != tcapi_set(node, TIME_CFG_DML_NAME, buf)){
        return VAR_NO_RESOURCE;
    }
    
    sprintf(buf, "%d", timerCfg_ptr->dms);
    if (0 != tcapi_set(node, TIME_CFG_DMS_NAME, buf)){
        return VAR_NO_RESOURCE;
    }

#if 0	
    if(tcapi_commit(node) != 0){
        eponOamExtDbg(DBG_OAM_L1, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
        return VAR_NO_RESOURCE;
    }
#else
	postDBSetSyncMsg(DB_SYNC_VOIPDIGIT_MAP_ENTRY_COMMIT_CONFIG);
#endif
#endif
    return SET_OK;
}
#endif //TCSUPPORT_VOIP


/* -------------SIP Digit Map Data Manager ------------------ */
typedef struct sipDigitMapData_s
{
    struct sipDigitMapData_s *next;
    u_char  blockNo;
    u_char  length;
    u_int   data[125];
}SIPDigitMapData_t, *SIPDigitMapData_Ptr;

typedef struct sipDigitMapDataMgr_s
{
    u_char  state;// NULL=0/BEGIN=1
    u_char  num;     // all block num
    u_char  curNum; // current recivie num
    u_int   length; // the recivie blocks length sum
    SIPDigitMapData_Ptr end;
    SIPDigitMapData_Ptr list;
}SIPDigitMapDataMgr_t, *SIPDigitMapDataMgr_Ptr;

SIPDigitMapDataMgr_t gSIPMapDataMgr = {0};

void freeDigitMapDataMgr(SIPDigitMapDataMgr_Ptr pMapMgr)
{
    if (pMapMgr){
        SIPDigitMapData_Ptr pMap = pMapMgr->list;
        while(pMap){
            pMapMgr->list = pMap->next;
            free(pMap);
            pMap = pMapMgr->list;
        }
        memset(pMapMgr, 0, sizeof(SIPDigitMapDataMgr_t));
    }
}


char gStrSIPDigitMap[1024] = {0};
u_char oamSetSIPDigitMap(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    OamSIPDigitMap_Ptr sipdmp = (OamSIPDigitMap_Ptr)pValue;
    SIPDigitMapData_Ptr pMapData = NULL;

    if (length <= 2){
        return VAR_BAD_PARAMETERS;
    }

    if (sipdmp->blockNo != gSIPMapDataMgr.curNum){
        freeDigitMapDataMgr(&gSIPMapDataMgr);
        return VAR_BAD_PARAMETERS;
    }

    pMapData = (SIPDigitMapData_Ptr)malloc(sizeof(SIPDigitMapData_t));
    pMapData->next = NULL;
    pMapData->blockNo = sipdmp->blockNo;
    pMapData->length = length - 2;
    memcpy(pMapData->data, pValue+2, pMapData->length); 

    if (gSIPMapDataMgr.state == 0){
        gSIPMapDataMgr.state  = 1;
        gSIPMapDataMgr.curNum = 1;
        gSIPMapDataMgr.num    = sipdmp->blockNum;
        gSIPMapDataMgr.length = pMapData->length;
        
        gSIPMapDataMgr.end  = pMapData;
        gSIPMapDataMgr.list = gSIPMapDataMgr.end;
    }else if (gSIPMapDataMgr.state == 1){
        gSIPMapDataMgr.length += pMapData->length;
        gSIPMapDataMgr.curNum ++;

        gSIPMapDataMgr.end->next = pMapData;
        gSIPMapDataMgr.end       = pMapData;
    }

    if (gSIPMapDataMgr.curNum == gSIPMapDataMgr.num){ // revice all blocks
        u_char *block = (u_char*)malloc(gSIPMapDataMgr.length);
        u_char *dp = block;
        SIPDigitMapData_Ptr pMapData = gSIPMapDataMgr.list;

        while(pMapData){
            memcpy(dp, pMapData->data, pMapData->length);
            PTR_MOVE(dp, pMapData->length);
            pMapData = pMapData->next;
        }

#if 0
        if (0 != tcapi_set("VoIPDigitMap_Entry", "DigitMap", (char*)block)){
            freeDigitMapDataMgr(&gSIPMapDataMgr);
            return VAR_NO_RESOURCE;
        }
#else
		strncpy(gStrSIPDigitMap,(char*)block,sizeof(gStrSIPDigitMap)-1);
		postDBSetSyncMsg(DB_SYNC_SIP_DIGITMAP_CONFIG);
#endif
        freeDigitMapDataMgr(&gSIPMapDataMgr);
    }

    return SET_OK;
}
#endif //TCSUPPORT_VOIP
/* ------------------------------VOIP actions and attributions end------------------------ */


/**************************** Nodes we current don't support ********************************/

u_char oamGetE1Port(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
u_char oamSetE1Port(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}

/* -----------------------------MxUMng actions and attributions--------------------------- */
// for MDU/MTU SNMP remote manager
u_char oamGetMxUMngGlobalParameter(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
u_char oamSetMxUMngGlobalParameter(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}


u_char oamGetMxUMngSNMPParameter(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
u_char oamSetMxUMngSNMPParameter(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    return VAR_BAD_PARAMETERS;
}
/* -----------------------------MxUMng actions and attributions end----------------------- */


