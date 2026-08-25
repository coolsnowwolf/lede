/*
********************************************************************************
Copyright Statement:

This software/firmware and related documentation ("EcoNet Software") are 
protected under relevant copyright laws. The information contained herein is 
confidential and proprietary to EcoNet (HK) Limited ("EcoNet") and/or its 
licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND 
AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("ECONET SOFTWARE") 
RECEIVED FROM ECONET AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN 
"AS IS" BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, WHETHER 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. NOR DOES
ECONET PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD
PARTIES WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET
SOFTWARE. RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS
RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER
LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES MADE TO
RECEIVER'S SPECIFICATION OR CONFORMING TO A PARTICULAR STANDARD OR OPEN FORUM.
RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND ECONET'S ENTIRE AND CUMULATIVE
LIABILITY WITH RESPECT TO THE ECONET SOFTWARE RELEASED HEREUNDER SHALL BE, AT
ECONET'S SOLE OPTION, TO REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR
REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET
FOR SUCH ECONET SOFTWARE.
********************************************************************************

Module Name:
    oam_ctc_voip_conf.c

Abstract:
    This file contains functions used to fullfill voip configuration database
    related tasks.

Revision History:
Who                  When             What
---------            -----------      -----------------------------------------
Name                 Date             Modification logs

Archer.Shi mtk06084   2013/12/26      Create

*/
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "ctc/oam_ctc_voip_conf.h"
#include "libtcapi.h"
#include "ctc/oam_sys_api.h"

#define VOIP_SIP_ENABLED        "Yes"
#define VOIP_SIP_DISABLED       "No"

enum{
    PORT_STATUS_REGISTING  ,
    PORT_STATUS_IDLE       ,
    PORT_STATUS_OFFHOOK    ,
    PORT_STATUS_DIALUP     ,
    PORT_STATUS_RINGING    ,
    PORT_STATUS_RINGBACK   ,
    PORT_STATUS_CONNECTING ,
    PORT_STATUS_CONNECTED  ,
    PORT_STATUS_RELEASECONN,
    PORT_STATUS_REG_FAILED ,
    PORT_STATUS_NOTACTIVED ,
};

enum{
    PORT_SERVICE_STATE_ENDLOCAL  ,
    PORT_SERVICE_STATE_ENDREMOTE ,
    PORT_SERVICE_STATE_ENDAUTO   ,
    PORT_SERVICE_STATE_NORMAL
};

enum{
    TR69_CALL_STATUS_IDLE          ,
    TR69_CALL_STATUS_CALLING       ,
    TR69_CALL_STATUS_RINGING       ,
    TR69_CALL_STATUS_CONNECTING    ,
    TR69_CALL_STATUS_INCALL        ,
    TR69_CALL_STATUS_HOLD          ,
    TR69_CALL_STATUS_DISCONNECTING
};

static u_int gCallStatusMap [] = {
    PORT_STATUS_IDLE      ,
    PORT_STATUS_OFFHOOK   ,
    PORT_STATUS_RINGING   ,
    PORT_STATUS_CONNECTING,
    PORT_STATUS_CONNECTED ,
    PORT_STATUS_CONNECTED ,
    PORT_STATUS_IDLE
};

static const char * const gCallStatusStrs [] =
{
    "Idle",
    "Calling",
    "Ringing",
    "Connecting",
    "InCall",
    "Hold",
    "Disconnecting"
};

enum{
    TR69_SIP_STATUS_UP           ,
    TR69_SIP_STATUS_INITIALIZING ,
    TR69_SIP_STATUS_REGISTERING  ,
    TR69_SIP_STATUS_UNREGISTERING,
    TR69_SIP_STATUS_ERROR        ,
    TR69_SIP_STATUS_TESTING      ,
    TR69_SIP_STATUS_QUIESCENT    ,
    TR69_SIP_STATUS_DISABLED
};

static const char * const gSipStatusStrs [] =
{
    "Up"           ,
    "Initializing" ,
    "Registering"  ,
    "Unregistering",
    "Error"        ,
    "Testing"      ,
    "Quiescent"    ,
    "Disabled"

};

enum {
    CHAN_DISABLED,
    CHAN_ENALBED
};


/*
    helper function
*/

static int
oam_tcapi_getl(char * nodeName, char * attrName, char * buff, long * pResult)
{
    if (TCAPI_SUCCESS != tcapi_get(nodeName, attrName, buff)){
        return -1;
    }

    char * endPtr;
    errno = 0; // first reset errno
    *pResult = strtol(buff, &endPtr, 0);
    if (0 != errno || ( 0 == *pResult && buff == endPtr)){
        return -1;
    }

    return 0;
}

static int
oam_tcapi_getul(const char * const nodeName, const char * const attrName, 
                char * buff, unsigned long * pResult)
{
    if (TCAPI_SUCCESS != tcapi_get(nodeName, attrName, buff)){
        return -1;
    }

    char * endPtr;
    errno = 0; // first reset errno
    *pResult = strtoul(buff, &endPtr, 0);
    if (0 != errno || ( 0 == *pResult && buff == endPtr)){
        return -1;
    }

    return 0;
}

static inline int
oam_tcapi_get_addr(char * nodeName, char * attrName, char * buff, 
                   u_int * pAddr)
{
    if (TCAPI_SUCCESS != tcapi_get(nodeName, attrName, buff)){
        return -1;
    }
    *pAddr = inet_addr(buff);
    return 0;
}

static inline int
oam_tcapi_set_net_addr(char * nodeName, char * attrName, u_int addr)
{
    return tcapi_set(nodeName, attrName, inet_ntoa(*((struct in_addr*)&addr)));
}


/*
   descritption: find the same string in an array of strings, by comparing one
                 by one, ignoring case
   params: 
        matchStr: the string to be matched
        matchLen: maximum length of the string to be matched
        strArray: the array of strings to be searched
        arrSize : the size of the array of strings to be searched
   return value: on success, return the index of the matching string, otherwise
                 return -1
*/

static inline int
find_match_string( const char *  const matchStr, u_int matchLen,
                   const char ** const strArray, u_int arrSize )
{
    OAM_FUNC_TRACE_ENTER;
    int idx = 0;
    for ( ; idx < arrSize; ++ idx){
        if ( 0 == strncasecmp(matchStr, strArray[idx], matchLen) ){
            OAM_FUNC_TRACE_LEAVE;
            return idx;
        }
    }
    OAM_FUNC_TRACE_LEAVE;
    return -1;
}

/*******************************************************************************
*
*                            GlobalParameterConfig
*
*******************************************************************************/

enum { IF_NUM_MAX = 15, BUFF_MAX = 30 };

#define GET_WAN_PVC_IDX(Interface)    ((Interface) >> 3 )
#define GET_WAN_ENTRY_IDX(Interface)  ((Interface) & 0x7)


static int
oam_str2ul(const char * const str, unsigned long * pResult)
{
    errno = 0;
    char * endptr;
    *pResult = strtoul(str, &endptr, 0);
    if (0 != errno || ( 0 == *pResult && str == endptr)){
        return -1;
    }
    return 0;
}


/*
    description: parse the valid interface number
    parameters: buff[in] contains all the valid  interface IDs,
                formats like "1,12,4,6"
                IFIdx[out] output the integral form of all the IF IDs
                pIFNum[out] the number of IF IDs
    return value: 0/success -1/fail
*/

static int
parseValidIF(char * buff, u_int * IFIdx, u_char * pIFNum)
{
    u_char IFNum = 0, i = 0;
    char * temp[IF_NUM_MAX];
    temp[IFNum] = buff;

    // seaprate the valid interface IDs
    for(; 0 != buff[i] && i < BUFF_MAX && IFNum < IF_NUM_MAX; ++ i){
        if(',' == buff[i]){
            buff[i] = '\0';
            if ( (i + 1) < BUFF_MAX &&  (IFNum + 1) < IF_NUM_MAX){
                temp[++IFNum] = & buff[i + 1];
            }else{
                return -1;
            } 
        }
    }

    // convert strings into integers
    for( i = 0; i < IFNum; ++ i ){
        if ( 0 != oam_str2ul(temp[i], &IFIdx[i] )){
            return -1;
        }
    }

    *pIFNum = IFNum;

    return 0;
}

static char gVoipWanNode[30] = {0};

// find the line used by VOIP
static int 
get_voip_wan_node()
{
    char buff[BUFF_MAX] = {0};
    u_char IFNum = 0;
    u_int IFIdx[IF_NUM_MAX] = {0};
    int i = 0;
    char type[BUFF_MAX] = {0};
    u_int pvcIdx = 0;
    u_int entryIdx = 0;

    if(TCAPI_SUCCESS != tcapi_get(WANINFO_COMMON, "ValidIFIndex", buff)){
        return -1;
    }
    
    if ( 0 != parseValidIF(buff, IFIdx, &IFNum) ){
        return -1;
    }

    /*
        terate all the valid interface IDs, and find the one used by VoIP
    */
    for(i = 0; i < IFNum; ++ i){
        pvcIdx = GET_WAN_PVC_IDX(IFIdx[i]);
        entryIdx = GET_WAN_ENTRY_IDX(IFIdx[i]);
        
        sprintf( gVoipWanNode, "Wan_PVC%d_Entry%d", pvcIdx, entryIdx);
        
        if ( TCAPI_SUCCESS != tcapi_get(gVoipWanNode, "ServiceList", type)){
            return -1;
        }
        
        if (NULL != strstr(type, "VOICE")){
            return 0;
        }
    }

    return -1;
}

static unsigned char
ip_mode_mapping(unsigned char IPMode){
    static const u_char IPModeTbl[] = {
        IP_MODE_DHCP, IP_MODE_STATIC, IP_MODE_PPPOE
    };

    if (IPMode >= ARRAY_SIZE(IPModeTbl)){
        return IP_MODE_UNDEFINED;
    }else{
        return IPModeTbl[IPMode];
    }
}

static int
get_ip_mask_gw_via_tcapi(OamGlobalParamConfig_Ptr pGlbParamCfg, char *buff)
{
    typedef struct
    {
        char  * attr;
        u_int * addr;
    } pair_t;
    
    const pair_t arr[] = {
        {"IPADDR",  &pGlbParamCfg->iadIpAddr   },
        {"NETMASK", &pGlbParamCfg->iadNetMask  },
        {"GATEWAY", &pGlbParamCfg->iadDefaultGW}
    };

    int i = 0;
    for (; i < ARRAY_SIZE(arr); ++i)
    {
        if (0 != oam_tcapi_get_addr(gVoipWanNode, arr[i].attr, buff, arr[i].addr)){
            return -1;
        }
    }
    return 0;
}

static inline int
get_default_ip_mask_gw(OamGlobalParamConfig_Ptr pGlbParamCfg, char *buff)
{
    pGlbParamCfg->iadIpAddr    = 0;
    pGlbParamCfg->iadNetMask   = 0;
    pGlbParamCfg->iadDefaultGW = 0;
    return 0;
}

static inline int
get_ip_mask_gw(OamGlobalParamConfig_Ptr pGlbParamCfg, char *buff)
{
    if ( IP_MODE_STATIC == pGlbParamCfg->voiceIpMode ){
        return get_ip_mask_gw_via_tcapi(pGlbParamCfg, buff);
    } else {
        return get_default_ip_mask_gw(pGlbParamCfg, buff);
    }
}

static inline int
get_voice_ip_mode(OamGlobalParamConfig_Ptr pGlbParamCfg, char *buff)
{
    long ISP = 0;

    if ( 0 != oam_tcapi_getl(gVoipWanNode, "ISP", buff, &ISP) ){
        return -1;
    }

    pGlbParamCfg->voiceIpMode = ip_mode_mapping((u_char)ISP);

    if ( IP_MODE_UNDEFINED == pGlbParamCfg->voiceIpMode ){
        return -1;
    }

    return 0;
}

static inline int
get_pppoe_username_passwd(OamGlobalParamConfig_Ptr pGlbParamCfg, char *buff)
{
    if ( IP_MODE_PPPOE == pGlbParamCfg->voiceIpMode ){
        if (TCAPI_SUCCESS != tcapi_get(gVoipWanNode, "USERNAME", buff)){
            return -1;
        }
        strncpy(pGlbParamCfg->PPPoEUsername, buff, 32);

        if (TCAPI_SUCCESS != tcapi_get(gVoipWanNode, "PASSWORD", buff)){
            return -1;
        }
        strncpy(pGlbParamCfg->PPPoEPassword, buff, 32);
    }else{
        pGlbParamCfg->PPPoEUsername[0] = '\0';
        pGlbParamCfg->PPPoEPassword[0] = '\0';
    }

    return 0;
}

// fill the unimplemented fields with default values
static inline void
fill_unimpl_fields(OamGlobalParamConfig_Ptr pGlbParamCfg)
{
    pGlbParamCfg->PPPoEMode = PPPOE_AUTH_AUTO;

    // Because the info about VLAN tag is not present in current romfile, and
    // tag-stacking is not supported current, so hard-coded value is used here
    pGlbParamCfg->taggedFlag = 0;
    
    // These attribute, CVLan, SVLan and Priority, don't have corresponding
    // romfile-nodes/romfile-attributes, so hard-coded value is used here
    pGlbParamCfg->voiceCVlan = 0;
    pGlbParamCfg->voiceSVlan = 0;
    pGlbParamCfg->voicePriority = 5;
}

void
db_voip_conf_update_glb_param_cfg(OamCtcDB_t *pDB)
{
    OAM_FUNC_TRACE_ENTER;

    VoIP_Node_Flag_e * pFlag = &pDB->cfg_info.voip.glbParamFlag;
    OamGlobalParamConfig_Ptr pGlbParamCfg = &pDB->cfg_info.voip.glbParam;
    enum {BUFF_SIZE = 35};
    char buff[BUFF_SIZE] = {0};
    do{
    
        if ( 0 != get_voip_wan_node() ){
            OamTrace("Can't find VoIP wan line!\n");
            break;
        }
        
        if ( 0 != get_voice_ip_mode(pGlbParamCfg, buff) ){
            OamTrace("Can't update ip mode!\n");
            break;
        }

        if ( 0 != get_ip_mask_gw(pGlbParamCfg, buff) ){
            OamTrace("Can't update ip mask gw!\n");
            break;
        }

        if ( 0 != get_pppoe_username_passwd(pGlbParamCfg, buff) ){
            OamTrace("Can't update PPPoE username passwd!\n");
            break;
        }

        fill_unimpl_fields(pGlbParamCfg);

        *pFlag = VOIP_NODE_VALID;

        OamTrace("Global Parameter cofig node update success!\n");
        OAM_FUNC_TRACE_LEAVE;
        return;
        
    } while(0);

    *pFlag = VOIP_NODE_INVALID;
    OAM_FUNC_TRACE_LEAVE;
    return;
}



/*******************************************************************************
*
*                               POTSStatus
*
*******************************************************************************/

/*
   descritption: query whether some pot is enabled or not
   params: 
        pEnable[out]: get result, enum, CHAN_ENALBED, CHAN_DISABLED
        idx[in]: pot index
   return value: SUCCESS/0, FAIL/-1 
*/
static int
get_if_chan_enabled( u_char *pEnable, u_char idx){
    OAM_FUNC_TRACE_ENTER;
    enum {NODE_NAME_LEN = 30, BUFF_SIZE = 10};

    char node[NODE_NAME_LEN];
    char buff[BUFF_SIZE];
    
    snprintf(node, NODE_NAME_LEN, VOIPBASIC_ENTRY, idx);
    if (TCAPI_SUCCESS != tcapi_get(node, "SIPEnable", buff)){
        return -1;
    }

    OamTrace("Get %s_SIPEnable:%s\n", node, buff);
    
    if (0 == strncasecmp(buff, VOIP_SIP_ENABLED, BUFF_SIZE)){
        *pEnable = CHAN_ENALBED;
    }else if (0 == strncasecmp(buff, VOIP_SIP_DISABLED, BUFF_SIZE)){
        *pEnable = CHAN_DISABLED;
    }else{ // invalid value
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }
    OAM_FUNC_TRACE_LEAVE;
    return 0;
}


/*
   descritption: get sip client's status on some pot
   params: 
        pSipStatus[out]: get result, enum, CHAN_SIP_DOWN, CHAN_SIP_UP
        idx[in]: pot index
   return value: SUCCESS/0, FAIL/-1 
*/
static int
get_sip_status( u_char * pSipStatus, u_char idx )
{
    OAM_FUNC_TRACE_ENTER;

    enum {NODE_NAME_LEN = 30, BUFF_SIZE = 20};

    char node[NODE_NAME_LEN];
    char buff[BUFF_SIZE];
    
    snprintf(node, NODE_NAME_LEN, VOIP_INFO_ENTRY, idx);
    OamTrace("Node name: %s",node);
    if (TCAPI_SUCCESS != tcapi_get(node, "Status", buff)){
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    *pSipStatus =
    find_match_string(buff, BUFF_SIZE, gSipStatusStrs, ARRAY_SIZE(gSipStatusStrs));

    if ( -1 == *pSipStatus){
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    OAM_FUNC_TRACE_LEAVE;
    return 0;
}

/*
   descritption: get call status on some pot
   params: 
        pCallStatus[out]: get result, enum value CALL_STATUS_****
        idx[in]: pot index
   return value: SUCCESS/0, FAIL/-1 
*/
static int
get_call_status( u_char * pCallStatus, u_char idx )
{
    OAM_FUNC_TRACE_ENTER;

    enum {NODE_NAME_LEN = 30, BUFF_SIZE = 20};

    char node[NODE_NAME_LEN];
    char buff[BUFF_SIZE];
    
    snprintf(node, NODE_NAME_LEN, VOIP_INFO_ENTRY, idx);
    if (TCAPI_SUCCESS != tcapi_get(node, "CallState", buff)){
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    *pCallStatus = 
    find_match_string(buff, BUFF_SIZE, gCallStatusStrs, ARRAY_SIZE(gCallStatusStrs));

    if ( -1 == *pCallStatus ){
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    OAM_FUNC_TRACE_LEAVE;
    return 0;
}


static int
map_tr69_to_epon_pots_status(u_char sipStatus, u_char callStatus,
                             u_int * pPortStatus, u_int * pServiceStatus)
{
    OAM_FUNC_TRACE_ENTER;

    *pServiceStatus = htonl(PORT_SERVICE_STATE_NORMAL);

    switch(sipStatus)
    {
    case TR69_SIP_STATUS_INITIALIZING:
    case TR69_SIP_STATUS_REGISTERING:
        *pPortStatus  = htonl(PORT_STATUS_REGISTING);
        break;

    case TR69_SIP_STATUS_ERROR:
        *pPortStatus = htonl(PORT_STATUS_REG_FAILED);
        break;

    case TR69_SIP_STATUS_TESTING:
    case TR69_SIP_STATUS_DISABLED:
        *pPortStatus = htonl(PORT_STATUS_NOTACTIVED);
        break;
        
    case TR69_SIP_STATUS_UNREGISTERING:
        *pPortStatus = htonl(PORT_STATUS_RELEASECONN);
        break;

    case TR69_SIP_STATUS_UP:
    case TR69_SIP_STATUS_QUIESCENT:
        if ( callStatus >= ARRAY_SIZE(gCallStatusMap)){
            OAM_FUNC_TRACE_LEAVE;
            return -1;
        }
        *pPortStatus = htonl(gCallStatusMap[callStatus]);
        break;
        
    default:
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    OAM_FUNC_TRACE_LEAVE;
    return 0;
}

static int
get_pots_status_if_chan_enabled(OamPOTSStatus_t * pStatus, u_char idx)
{
    OAM_FUNC_TRACE_ENTER;
    u_char sipStatus = 0;
    u_char callStatus = 0;

    if (0 != get_sip_status(&sipStatus,idx)){
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }
    
    if (0 != get_call_status(&callStatus,idx)){
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    if (0 != map_tr69_to_epon_pots_status(sipStatus, callStatus,  
             &pStatus->iadPortStatus, &pStatus->iadPortServiceStatus)){
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }
    OAM_FUNC_TRACE_LEAVE;    
    return 0;
}

static int
get_codec_mode(u_int * pCodec, u_char idx)
{
    OAM_FUNC_TRACE_ENTER;
    const char  node_name[] = "VoIPCodecs_PVC%d_Entry%d";

    /* for CTC SPEC 0:G.711A 1:G.729 2:G711U 3:G.723 4:G.726 5:T.38 6:Other(G.722)*/
    const char * const codecs[]=
    {
        "G.711 a-law","G.729",
        "G.711 u-law","G.723",
        "G.726" ,"T.38","Other"
    };

    enum {NAME_LEN = 35, BUFF_LEN = 10 ,MAX_ENTRY_NUM = 16};
    char node[NAME_LEN];
    char buff[BUFF_LEN];
    
    u_int min = 0xffffffff;
    u_int codec_code = 0xffffffff;
    unsigned long tmp;
    int i = 0;
    int j = 0;

    for (j = 0; j <= MAX_ENTRY_NUM; j++) 
    {
        memset(node,0,sizeof(node));
        memset(buff,0,sizeof(buff));
        sprintf(node, node_name, idx,j);

        if (TCAPI_SUCCESS != tcapi_get(node,"codec",buff))
            continue;
		
        for (i = 0; i < (ARRAY_SIZE(codecs) - 1); i++)
        {
            if (0 == strncmp(codecs[i],buff,strlen(codecs[i])))
                break;
        }

        if ( 0 != oam_tcapi_getul(node, "priority", buff, &tmp) )
        {
            OAM_FUNC_TRACE_LEAVE;
            return -1;
        }
        if ( tmp < min && tmp != 0)
        {
            min = tmp;
            codec_code = i;
        }
    }

    if ( 0xffffffff == codec_code )
    {
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }
    *pCodec = htonl(codec_code);
    OAM_FUNC_TRACE_LEAVE;
    return 0;
}

/* 
   descritption: get a single pots status configuration data
   return value: SUCCESS/0, FAIL/-1 
*/
static int
db_voip_conf_update_single_pots_status(OamCtcDBVoipPortConf_t * pPort, 
                                       u_char idx)
{
    OAM_FUNC_TRACE_ENTER;
    u_char *pEnable = &pPort->enable;
    OamPOTSStatus_t * pStatus = &pPort->potsStatus;
    u_int * pCodec = &pStatus->iadPortCodecMode;

    if (0 != get_if_chan_enabled(pEnable,idx)){
        OamTrace("Can't get status of chan %d", idx);
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    OamTrace("SIPEnabled value: %d\n", *pEnable);
    
    if (0 != get_codec_mode(pCodec,idx)){
        OamTrace("Can't get codec of chan %d", idx);
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    switch(*pEnable)
    {
    case CHAN_DISABLED:
        pStatus->iadPortStatus = htonl(PORT_STATUS_NOTACTIVED);
        pStatus->iadPortServiceStatus = htonl(PORT_SERVICE_STATE_NORMAL);
        break;
        
    case CHAN_ENALBED:
        if( 0 != get_pots_status_if_chan_enabled(pStatus, idx) ){
            OamTrace("Can't get pots status & pots srv status of chan %d", idx);
            OAM_FUNC_TRACE_LEAVE;
            return -1;
        }
        break;
        
    default:
        OAM_FUNC_TRACE_LEAVE;
        return -1;
    }

    OamTrace("Update pots status for chan %d success\n", idx);
    OAM_FUNC_TRACE_LEAVE;
    return 0;
}

/* 
    get the pots status configuration data 
*/

void
db_voip_conf_update_pots_status(OamCtcDB_t *pDB)
{
    OAM_FUNC_TRACE_ENTER;
    OamCtcDBVoipPortConf_t * pPort = pDB->cfg_info.voip.port;
    VoIP_Node_Flag_e * pFlag = NULL;
    
    int portIdx = 0;
    for (; portIdx < OAM_VOIP_PORT_NUM_MAX; ++ portIdx ){
        pFlag = &pPort[portIdx].potsStatusFlag;
        if( 0 !=  db_voip_conf_update_single_pots_status(&(pPort[portIdx]), portIdx)){
            *pFlag = VOIP_NODE_INVALID;
            OAM_FUNC_TRACE_LEAVE;
            return;
        }
        *pFlag = VOIP_NODE_VALID;
    }
    OAM_FUNC_TRACE_LEAVE;
    return;
}


/*
    Set Operation Related function
*/

static inline int
sync_set_voice_ip_mode(u_char voiceIpMode)
{
    char * pszISP, * pszlinkMode;

    switch (voiceIpMode)
    {
    case IP_MODE_PPPOE:
        pszISP = "2";
        pszlinkMode = "linkPPP";
        break;

    case IP_MODE_STATIC:
        pszISP = "1";   // IP_MODE_STATIC and IP_MODE_STATIC have the
                        // same linkMode, so here will fall through

    case IP_MODE_DHCP:
        pszISP = "0";
        pszlinkMode = "linkIP";
        break;

    default:
        return -1;
    }

    if (TCAPI_SUCCESS != tcapi_set(gVoipWanNode, "ISP", pszISP)){
        return -1;
    }

    if (TCAPI_SUCCESS != tcapi_set(gVoipWanNode, "LinkMode", pszlinkMode)){
        return -1;
    }

    return 0;
}

static int
sync_set_ip_mask_gw(const OamGlobalParamConfig_t * const pCfgParam)
{
    int i = 0;
    
    if ( IP_MODE_STATIC != pCfgParam->voiceIpMode ){
        return 0;
    }

    typedef struct {
        char * attr;
        u_int  addr;
    } pair_t;
    
    const pair_t arr[] = {
        {"IPADDR" , pCfgParam->iadIpAddr   },
        {"NETMASK", pCfgParam->iadNetMask  },
        {"GATEWAY", pCfgParam->iadDefaultGW}
    };
    
    for ( ; i < ARRAY_SIZE(arr); ++i ){
        if ( 0 != oam_tcapi_set_net_addr(gVoipWanNode, arr[i].attr, arr[i].addr)){
            return -1;
        }
    }

    return 0;
}


void
sync_set_glb_param_cfg(OamCtcDB_t *pDB)
{
    const OamGlobalParamConfig_t * const 
    pOldParam =  & pDB->cfg_info.voip.glbParam;

    const OamGlobalParamConfig_t * const 
    pNewParam =  & pDB->cfg_info.voip.glbParamCfgSet;

    if ( 0 != get_voip_wan_node() ){
        return;
    }

    if ( pOldParam->voiceIpMode !=  pNewParam->voiceIpMode ){
        if ( 0 != sync_set_voice_ip_mode(pNewParam->voiceIpMode) ){
            return;
        }

        if ( 0 != sync_set_ip_mask_gw(pNewParam) ){
            return;
        }
    }
}

