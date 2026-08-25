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
    omci_me_vlan_func.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    wayne.lee   2012/7/28   Create
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_vlan.h"
#include "omci_me_gemport_mapping.h"
#include "omci_me_mac_bridge.h"
#include "omci_me_layer3_data_services.h"

extern int commitNodeTimeId;

#ifndef TCSUPPORT_PON_VLAN
/*******************************************************************************************************************************
vlan API

********************************************************************************************************************************/

/*******************************************************************************************
**function name
    findVlanRuleIndex
**description:
    find vlan rule index, according the rule information
 **retrun :
    -1: failure
    0:  success
**parameter:
    vlanRulePortId :    vlan rule port Id
    findRule_ptr:       del this rule 
    index:          return rule index(0~)
********************************************************************************************/
int findVlanRuleIndex(IN uint8 vlanRulePortId, IN pon_vlan_rule_p findRule_ptr, OUT uint8 index){
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findVlanRuleIndex API->");
    return 0;
}
void displayVlanRule(pon_vlan_rule rule){
#if 0
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_outer_tpid=%d", rule.filter_outer_tpid);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_outer_pri=%d", rule.filter_outer_pri);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_outer_dei=%d", rule.filter_outer_dei);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_outer_vid=%d", rule.filter_outer_vid);

    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_inner_tpid=%d", rule.filter_inner_tpid);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_inner_pri=%d", rule.filter_inner_pri);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_inner_dei=%d", rule.filter_inner_dei);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_inner_vid=%d", rule.filter_inner_vid);
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n down_filter_outer_tpid=%d", rule.down_filter_outer_tpid);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n down_filter_outer_pri=%d", rule.down_filter_outer_pri);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n down_filter_outer_dei=%d", rule.down_filter_outer_dei);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n down_filter_outer_vid=%d", rule.down_filter_outer_vid);
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n down_filter_inner_tpid=%d", rule.down_filter_inner_tpid);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n down_filter_inner_pri=%d", rule.down_filter_inner_pri);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n down_filter_inner_dei=%d", rule.down_filter_inner_dei);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n down_filter_inner_vid=%d", rule.down_filter_inner_vid);
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n tag_num=%d", rule.tag_num);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n filter_ethertype=%d", rule.filter_ethertype);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n treatment_method=%d", rule.treatment_method);

    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n add_fst_tpid=%d", rule.add_fst_tpid);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n add_fst_pri=%d", rule.add_fst_pri);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n add_fst_dei=%d", rule.add_fst_dei);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n add_fst_vid=%d", rule.add_fst_vid);
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n add_sec_tpid=%d", rule.add_sec_tpid);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n add_sec_pri=%d", rule.add_sec_pri);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n add_sec_dei=%d", rule.add_sec_dei);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n add_sec_vid=%d", rule.add_sec_vid);

    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n rule_priority=%d", rule.rule_priority);

#endif
    return;
}
int pon_enable_vlan_rule(void){
//      omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_enable_vlan_rule API");     
    return 0;
}
 int pon_disable_vlan_rule(void){
 //     omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_disable_vlan_rule API");    
    return 0;
}
 int pon_get_vlan_status(void){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_vlan_status API");  
    return 0;
}

 int pon_add_vlan_rule(int port, pon_vlan_rule rule){
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_add_vlan_rule API port=%d",port);   
    displayVlanRule(rule);
    return 0;
}
 int pon_del_vlan_rule(int port, int index){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_del_vlan_rule API port=%d, index=%d",port, index);
    return 0;
}
 int pon_clean_vlan_rule(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_clean_vlan_rule API port=%d",port);     
    return 0;
}

//this func will write data directly,it will cause error when rule pointer to the area which size less than 32*sizeof(pon_vlan_rule)
 int pon_get_vlan_rule(int port,pon_vlan_rule * rule_ptr){

    rule_ptr->filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
    rule_ptr->filter_outer_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    rule_ptr->filter_outer_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
    rule_ptr->filter_outer_vid = OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096;
    
    rule_ptr->filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
    rule_ptr->filter_inner_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    rule_ptr->filter_inner_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
    rule_ptr->filter_inner_vid = OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096;
    
    rule_ptr->down_filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
    rule_ptr->down_filter_outer_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    rule_ptr->down_filter_outer_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
    rule_ptr->down_filter_outer_vid = OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096;
    
    rule_ptr->down_filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
    rule_ptr->down_filter_inner_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    rule_ptr->down_filter_inner_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
    rule_ptr->down_filter_inner_vid = OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096;
    
    rule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_1;
    rule_ptr->filter_ethertype = OMCI_VLAN_TAG_OP_RULE_FILTER_ETHERTYPE_0;
    rule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_0;
    
    rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_0;
    rule_ptr->add_fst_pri = OMCI_VLAN_TAG_PBIT_0;
    rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_0;
    rule_ptr->add_fst_vid = OMCI_VLAN_TAG_VID_4094;
    
    rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_0;
    rule_ptr->add_sec_pri = OMCI_VLAN_TAG_PBIT_0;
    rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_0;
    rule_ptr->add_sec_vid = OMCI_VLAN_TAG_VID_4094;

    rule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_LOW;

//  omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_vlan_rule API port=%d",port);       
    return 0;
}

 int pon_set_vlan_downstream_mode(int port,int mode){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_set_vlan_downstream_mode API, port = %d, mode = %d", port, mode);
    return 0;
}
 int pon_get_vlan_downstream_mode(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_vlan_downstream_mode API, port = %d, return mode = 0", port);
    return 0;
}

 int pon_set_vlan_tpid(int port,uint16 inTPID, uint16 outTPID){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_set_vlan_tpid API, port = %d, inTPID = 0x%02x, outTPID=0x%02x", port, inTPID, outTPID);
    return 0;
}
 int pon_get_vlan_input_tpid(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_vlan_input_tpid API, port = %d", port);
    return 0;
}
 int pon_get_vlan_output_tpid(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_vlan_output_tpid API, port = %d", port);
    return 0;
}

 int pon_set_vlan_dscp_map(int port, uint32 * map){
    int i = 0;
#if 0
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_set_vlan_dscp_map API, port = %d, map=", port);
    for(i=0; i<6; i++){
        if(i%2 == 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n");
        }
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "0x%02x ", *(map+i));
    }

#endif
    return 0;
}
 int pon_get_vlan_dscp_map(int port, uint32 * map){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_vlan_dscp_map API, port = %d,", port);
    return 0;
}

 int pon_enable_default_vlan_rule(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_enable_default_vlan_rule API, port = %d,", port);
    return 0;
}
 int pon_disable_default_vlan_rule(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_disable_default_vlan_rule API, port = %d,", port);
    return 0;
}
 int pon_get_default_vlan_status(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_default_vlan_status API, port = %d,", port);
    return 0;
}

 int pon_enable_igmp_vlan_rule(void){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_enable_igmp_vlan_rule API,");
    return 0;
}
 int pon_disable_igmp_vlan_rule(void){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_disable_igmp_vlan_rule API,");
    return 0;
}
 int pon_get_igmp_vlan_status(void){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_igmp_vlan_status API,");
    return 0;
}

 int pon_set_igmp_vlan_mode(int port, int mode){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_set_igmp_vlan_mode API, port = %d, mode=%d", port, mode);
    return 0;
}
 int pon_get_igmp_vlan_mode(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_igmp_vlan_mode API, port = %d,", port);
    return 0;
}

 int pon_set_igmp_vlan_tci(int port, uint16 tci){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_set_igmp_vlan_tci API, port = %d, tci=0x%02x", port, tci);
    return 0;
}
 int pon_get_igmp_vlan_tci(int port){
 // omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n pon_get_igmp_vlan_tci API, port = %d,", port);
    return 0;
}
#endif
#ifndef TCSUPPORT_PON_VLAN_FILTER
/*******************************************************************************************************************************
vlan filter API

********************************************************************************************************************************/
int addGponVlanFilterRule(gponVlanFilterIoctl_ptr vlanFilterRule_ptr){
#if 0
    int i= 0;
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API");

    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API-->vlanFilterRule_ptr->port=0x%02x",vlanFilterRule_ptr->port);

    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API-->vlanFilterRule_ptr->portType=0x%02x",vlanFilterRule_ptr->portType);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API-->vlanFilterRule_ptr->type=0x%02x",vlanFilterRule_ptr->type);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API-->vlanFilterRule_ptr->untaggedAction=0x%02x",vlanFilterRule_ptr->untaggedAction);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API-->vlanFilterRule_ptr->taggedAction=%d",vlanFilterRule_ptr->taggedAction);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API-->vlanFilterRule_ptr->maxValidVlanListNum=0x%02x",vlanFilterRule_ptr->maxValidVlanListNum);
    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API-->vlanFilterRule_ptr->reserved=0x%02x",vlanFilterRule_ptr->reserved);
    for(i=0; i<MAX_GPON_VLAN_FILTER_LIST; i++){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGponVlanFilterRule API-->vlanFilterRule_ptr->vlanList[%d]=0x%02x",i,vlanFilterRule_ptr->vlanList[i]);
    }

#endif
    return 0;
}
int getGponVlanFilterRule(gponVlanFilterIoctl_ptr vlanFilterRule_ptr){//according to the port and port type
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGponVlanFilterRule API");
    return 0;
}
int delGponVlanFilterRule(uint16 port, uint8 portType){
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delGponVlanFilterRule API");
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delGponVlanFilterRule API-->port=0x%02x",port);
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delGponVlanFilterRule API-->portType=0x%02x",portType); 
    return 0;
}
int displayAllGponVlanFilterRule(void){
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n displayAllGponVlanFilterRule API");
    return 0;
}
#endif
/*******************************************************************************************************************************
globle variable

********************************************************************************************************************************/
/*******************************************************************************************************************************
general function

********************************************************************************************************************************/

/*******************************************************************************************
**function name
    convertDSCPMap2Pbit
**description:
    convert 24bytes dscp pbits mapping to pbits array
 **retrun :
    -1: failure
    0:  success
**parameter:
    dscp :  dscp pbits mapping(24 bytes, 64 dscp item)
    dscp2Pbit: pbits array
********************************************************************************************/

int convertDSCPMap2Pbit(IN uint8* dscp, OUT uint8 * dscp2Pbit){
//  uint8 times = 0;
    uint8 *tmp =NULL;
//  uint16 tmpContent = 0;
//  uint8 byte = 0;
    int ret = -1;
    uint8 i = 0;
    uint8 j = 0;
    uint8 index = 0;    
    uint32 tempU32 = 0;

    if((dscp == NULL) || (dscp2Pbit == NULL)){
        goto end;
    }
    tmp = dscp; 
    
    for(i=0; i<8; i++){
        tempU32 = *tmp<<16;
        tempU32 |= *(tmp+1)<<8;
        tempU32 |= *(tmp+2);
//      omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n tempU32=0x%02x",tempU32);
        for(j=0; j<8; j++){
            index = i*8;
            index += j;
            dscp2Pbit[index] = (uint8)(tempU32>>(21-j*3)) &0x07;
//          omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n dscp2Pbit[%d]=0x%02x",index,dscp2Pbit[index]);
        }
        tmp +=3;
    }

#if 0
    for(times = 0; times<MAX_OMCI_DSCP_KINDS; times++){
        if((times+1)*3 > 8*(byte+1)){
            tmpContent = ((*(tmp+1)<<8) | *tmp);
            dscp2Pbit[times] = ((tmpContent & (0x07 << (times*3-byte*8)))>>(times*3-byte*8));
//          omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n convertDSCPMap2Pbit->dscp2Pbit[%d] = %d",times, dscp2Pbit[times]);
            tmp++;
            byte++;
        }else{
            dscp2Pbit[times] = ((*tmp & (0x07 << (times*3-byte*8)))>>(times*3-byte*8));
//          omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n convertDSCPMap2Pbit->dscp2Pbit[%d] = %d",times, dscp2Pbit[times]);
        }
    }
#endif
    ret = 0;
end:
    if(ret != 0){
        ret = -1;
    }
    return ret; 

}

/*******************************************************************************************
**function name
    getVlanRulePortIdByInst
**description:
    get vlan rule port id (only support port id : 11~14( lan port 1~4))
 **retrun :
    -1: failure
    0:  success
**parameter:
    classId: support 9.3.12, 9.3.13
    instId :    9.3.12 or 9.3.13  instance id
    vlanRulePortId: return vlan rule port id
********************************************************************************************/
int getVlanRulePortIdByInst(uint16 classId, uint16 instId, uint8 * vlanRulePortId){
    uint8 * attrValue = NULL;
    int ret = -1;
    uint8 associationType = OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_0;
    uint16 associationInstId = 0;
    uint8 portId = 0;
    int ifaceType = 0;
    uint8 tempIndex = 0;
    uint8 ifcType = 0 ;
#ifdef TCSUPPORT_PON_IP_HOST
    uint8 isIpHostPort = 0;
    omciMeInst_t *meInst_p = NULL;
    omciManageEntity_ptr me_p = NULL;
#endif

    if(vlanRulePortId == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> association lanPortId == NULL, fail ");
        ret = -1;
        goto end;
    }
    if((classId != OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE)
        && (classId != OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE)){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> class id doesn't support, classid=0x%02x, fail ",classId);
        ret = -1;
        goto end;
    }
    /*malloc for saving attribute value*/
    attrValue =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
    if(attrValue == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> attrValue calloc error ");
        ret = -1;
        goto end;
    }
    
    /*get 9.3.12, 9.3.13 association type attribute value*/
    if(classId == OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE){
        tempIndex = 4;
    }else if(classId == OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE){
        tempIndex = 1;
    }else{
        //nothing
    }
    ret = getAttributeValueByInstId(classId, instId, tempIndex, attrValue, 1);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> getAttributeValueByInstId fail ");
        ret = -1;
        goto end;
    }
    associationType = *attrValue;

    /*get 9.3.12, 9.3.13 association pointer attribute value*/
    if(classId == OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE){
        tempIndex = 5;
    }else if(classId == OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE){
        tempIndex = 7;
    }else{
        //nothing
    }   
    ret = getAttributeValueByInstId(classId, instId, tempIndex, attrValue, 2);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> getAttributeValueByInstId 2 fail ");
        ret = -1;
        goto end;
    }
    associationInstId = get16(attrValue);

    /*get lan port id by association type*/
    if((classId == OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE)
        && (associationType == OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_0)){ //get lan port id by 9.3.12  physical path termination point ethernet UNI(default)
#if defined(TCSUPPORT_PON_IP_HOST)
        // according to G.984: OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_0 may point to ip host config data
        meInst_p = NULL;
        me_p = omciGetMeByClassId(OMCI_CLASS_ID_PPTP_ETHERNET_UNI);

        //if instance id of ip host config data and pptp uni is equal,it takes some mistakes.
        if(me_p != NULL)
            meInst_p = omciGetInstanceByMeId(me_p , instId);
        if(meInst_p != NULL){
            portId = (uint8)(instId);
            if(isFiberhome_551601){
                if(portId == 0){
                    portId++;
                }
            }
        }else{
            //ip host config data port or not?
            me_p = omciGetMeByClassId(OMCI_CLASS_ID_IP_HOST_CONFIG_DATA);
            meInst_p = omciGetInstanceByMeId(me_p , instId);
            if(meInst_p != NULL){
                isIpHostPort = 1;
                portId = (uint8)(instId) + IP_HOST_CONFIG_PORT_BASIC;               
            }else{
                ret = -1;
                goto end;
            }
        }   
#else
        portId = (uint8)(instId);
        if(isFiberhome_551601){
            if(portId == 0){
                portId++;
            }
        }
#endif
    }else if(((classId == OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE) && (associationType == OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_10))
            || ((classId == OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE) && (associationType == OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_2))){//get lan port id by  physical path termination point ethernet UNI
        portId = (uint8)(associationInstId);
    }else if(((classId == OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE) && (associationType == OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_3))
            || ((classId == OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE) && (associationType == OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_0))){//get lan port id by mac bridge port cofig data
        /*get mac bridge port interface type*/
        ifaceType = getMacBridgePortInstType(associationInstId, &ifcType);
        /*get  mac bridge port  ANI/UNI port id*/
        if(ifaceType == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//lan, UNI port
            ret = getMacBridgeLanPortId(associationInstId, &portId);
            if(ret != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> getMacBridgeLanPortId  fail ");
                ret = -1;
                goto end;
            }

#ifdef TCSUPPORT_PON_IP_HOST
            if(ifcType == OMCI_MAC_BRIDGE_PORT_IP_CONFIG_HOST){             
                isIpHostPort = 1;
            }else
#endif
            portId = portId + OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC; //port id :1~4
        }else{
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> ifaceType = 0x%02x,  fail ",ifaceType);
            ret = -1;
            goto end;
        }
    }else if(((classId == OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE) && (associationType == OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_10))){
        portId = 0;
        *vlanRulePortId = 0;
        ret = 0;
        goto end;
    }
#ifdef TCSUPPORT_PON_IP_HOST
    else if(((classId == OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE) && (associationType == OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_3))
                || ((classId == OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE) && (associationType == OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_1))){
        isIpHostPort = 1;
        portId = associationInstId + IP_HOST_CONFIG_PORT_BASIC;
    }
#endif
    else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> association type is  fail ");
        ret = -1;
        goto end;
    }

    /*get vlan port id */
#ifdef TCSUPPORT_PON_IP_HOST
    if(isIpHostPort){
        //just support one ip config data host now. port ID = 60
        if(portId != IP_HOST_CONFIG_PORT_BASIC && portId != (IP_HOST_CONFIG_PORT_BASIC +1)){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n get ip host config port index -> portId = %d, fail", portId);
            ret = -1;
            goto end;
        }
        portId = IP_HOST_CONFIG_PORT_BASIC; // only support 1 iphost in driver layer
        *vlanRulePortId = OMCI_VLAN_BASIC_IP_HOST_PORT_VAL + portId;
    }else
#endif
    {
    if((portId<= 0) || (portId >OMCI_LAN_PORT_NUM) ){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getVlanRulePortIdByInst-> portId(1~4) = %d, fail", portId);
        ret = -1;
        goto end;
    }
    *vlanRulePortId = OMCI_VLAN_BASIC_LAN_PORT_VAL + portId;
    }
    
    ret = 0;
end:
    if(attrValue != NULL){
        free(attrValue);
    }
    
    if(ret != 0){
        ret = -1;
    }
    return ret;
    
}

/*******************************************************************************************
**function name
    initVlanTagOpRule
**description:
    set default value to rule
 **retrun :
    -1: failure
    0:  success
**parameter:
    rule_ptr :  vlan rule pointer
********************************************************************************************/
int initVlanTagOpRule(pon_vlan_rule_p rule_ptr){

    if(rule_ptr == NULL){
        return -1;
    }
    /*set default value to current rule*/
    rule_ptr->filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
    rule_ptr->filter_outer_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    rule_ptr->filter_outer_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
    rule_ptr->filter_outer_vid = OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096;
    
    rule_ptr->filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
    rule_ptr->filter_inner_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    rule_ptr->filter_inner_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
    rule_ptr->filter_inner_vid = OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096;
    
    rule_ptr->down_filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
    rule_ptr->down_filter_outer_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    rule_ptr->down_filter_outer_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
    rule_ptr->down_filter_outer_vid = OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096;
    
    rule_ptr->down_filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
    rule_ptr->down_filter_inner_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    rule_ptr->down_filter_inner_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
    rule_ptr->down_filter_inner_vid = OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096;
    
    rule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_5;
    rule_ptr->filter_ethertype = OMCI_VLAN_TAG_OP_RULE_FILTER_ETHERTYPE_0;
    rule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_0;
    
    rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_0;
    rule_ptr->add_fst_pri = OMCI_VLAN_TAG_PBIT_0;
    rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_0;
    rule_ptr->add_fst_vid = OMCI_VLAN_TAG_VID_4094;
    
    rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_0;
    rule_ptr->add_sec_pri = OMCI_VLAN_TAG_PBIT_0;
    rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_0;
    rule_ptr->add_sec_vid = OMCI_VLAN_TAG_VID_4094;

    rule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_LOW;

    return 0;   
}

/*******************************************************************************************
**function name
    setVlanTagOpRuleValByTci
**description:
    set vlan tag operation rule value by tci or upstream vlan tag operation mode
 **retrun :
    -1: failure
    0:  success
**parameter:
    upMode :    9.3.12 upstream vlan tag operation mode
    tci:        9.3.12 tci
    untaggedRule_ptr: untagged vlan rule pointer
    taggedRule_ptr: tagged vlan rule pointer
********************************************************************************************/
int setVlanTagOpRuleValByTci(uint8 upMode, uint16 tci, pon_vlan_rule_p untaggedRule_ptr, pon_vlan_rule_p taggedRule_ptr){
    int ret = -1;
    
    if(untaggedRule_ptr == NULL || taggedRule_ptr == NULL){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpRuleByTci->parameter is NULL, fail");
        goto end;
    }

    switch(upMode){
        case OMCI_VLAN_TAG_OP_MODE_0:
            untaggedRule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_0; //untagged frame
            untaggedRule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_0; //transparent
            untaggedRule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_LOW; //low priority
            
            taggedRule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_1; // 1 tag
            taggedRule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_0; //transparent
            taggedRule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_LOW; //low priority       
            break;
        case OMCI_VLAN_TAG_OP_MODE_1:
            untaggedRule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_0; //untagged frame
            untaggedRule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_21; //add 1 tag
            untaggedRule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1; //output TPID
            untaggedRule_ptr->add_fst_pri =  ((tci>>13) &0x07);
            untaggedRule_ptr->add_fst_dei =  ((tci>>12)&0x01);
            untaggedRule_ptr->add_fst_vid = (tci & 0x0fff) ;
            untaggedRule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_LOW; //low priority
            
            taggedRule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_1;  // 1 tag
            taggedRule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_40; //modify inner tag
            taggedRule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1; //output TPID
            taggedRule_ptr->add_fst_pri =  ((tci>>13) &0x07);
            taggedRule_ptr->add_fst_dei =  ((tci>>12)&0x01);
            taggedRule_ptr->add_fst_vid = (tci & 0x0fff);
            taggedRule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_LOW; //low priority           
            break;
        case OMCI_VLAN_TAG_OP_MODE_2:
            untaggedRule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_0; //untagged frame
            untaggedRule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_21; //add 1 tag
            untaggedRule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1; //output TPID
            untaggedRule_ptr->add_fst_pri =  ((tci>>13) &0x07);
            untaggedRule_ptr->add_fst_dei =  ((tci>>12)&0x01);
            untaggedRule_ptr->add_fst_vid = (tci & 0x0fff) ;
            untaggedRule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_LOW; //low priority
            
            taggedRule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_1;  // 1 tag
            taggedRule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_21; //add 1 tag
            taggedRule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1; //output TPID
            taggedRule_ptr->add_fst_pri =  ((tci>>13) &0x07);
            taggedRule_ptr->add_fst_dei =  ((tci>>12)&0x01);
            taggedRule_ptr->add_fst_vid = (tci & 0x0fff);
            taggedRule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_LOW; //low priority   
            break;
        default:
            ret = -1;
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpRuleByTci -> upMode = %d,  fail ",upMode);
            goto end;
    }
    ret = 0;
end:
    return ret;
}

/*******************************************************************************************
**function name
    findVlanTagOpRuleByRuleInfo
**description:
    according the rule information , find this rule form rule list
 **retrun :
    -1: failure
    0:  success
**parameter:
    vlanRulePortId :    vlan rule port Id
    delRule_ptr:        del this rule 
********************************************************************************************/
int findVlanTagOpRuleByRuleInfo(uint8 vlanRulePortId, pon_vlan_rule_p findRule_ptr){
    pon_vlan_rule_p vlanRule_ptr = NULL;
    pon_vlan_rule_p tempVlanRule_ptr = NULL;
    int i = 0;
    uint16 entryRuleLen = 0;
    int findFlag = -1;
    int ret = -1;

    if(findRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findVlanTagOpRuleByRuleInfo-> delRule_ptr is NULL,  error");
        goto end;
    }

    /*malloc vlan rule space for saving getting all rule of port*/
    entryRuleLen = sizeof(pon_vlan_rule);
    vlanRule_ptr = (pon_vlan_rule_p)calloc(OMCI_MAX_PORT_VLAN_RULE_NUM, entryRuleLen);
    if(vlanRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findVlanTagOpRuleByRuleInfo-> vlanRule_ptr calloc error ");
        goto end;
    }

    /*get current all vlan rule*/
    ret = pon_get_vlan_rule(vlanRulePortId, vlanRule_ptr);
    if(ret < 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findVlanTagOpRuleByRuleInfo-> pon_get_vlan_rule, error");
        goto end;
    }

    /*del current vlan operation rule in  this port all vlan operation rule*/
    for(i = 0; i<OMCI_MAX_PORT_VLAN_RULE_NUM; i++){
        tempVlanRule_ptr = vlanRule_ptr+i;
        if(memcmp(tempVlanRule_ptr, findRule_ptr, entryRuleLen) == 0){
            findFlag = 0; //find
            break;
        }
    }

end:
    if(vlanRule_ptr != NULL){
        free(vlanRule_ptr);
    }

    return findFlag;

}





/*******************************************************************************************
**function name
    delVlanTagOpRuleByRuleInfo
**description:
    according the rule information , delete this rule form rule list
 **retrun :
    -1: failure
    0:  success
**parameter:
    vlanRulePortId :    vlan rule port Id
    delRule_ptr:        del this rule 
********************************************************************************************/
int delVlanTagOpRuleByRuleInfo(uint8 vlanRulePortId, pon_vlan_rule_p delRule_ptr){
    int ret = -1;
    pon_vlan_rule_p vlanRule_ptr = NULL;
    pon_vlan_rule_p tempVlanRule_ptr = NULL;
    int i = 0;
    uint16 entryRuleLen = 0;

    if(delRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delAllVlanTagOpRuleByInstId-> delRule_ptr is NULL,  error");
        ret  = -1;
        goto end;
    }

    /*malloc vlan rule space for saving getting all rule of port*/
    entryRuleLen = sizeof(pon_vlan_rule);
    vlanRule_ptr = (pon_vlan_rule_p)calloc(OMCI_MAX_PORT_VLAN_RULE_NUM, entryRuleLen);
    if(vlanRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delAllVlanTagOpRuleByInstId-> vlanRule_ptr calloc error ");
        ret = -1;
        goto end;
    }

    /*get current all vlan rule*/
    ret = pon_get_vlan_rule(vlanRulePortId, vlanRule_ptr);
    if(ret < 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delAllVlanTagOpRuleByInstId-> pon_get_vlan_rule, error");
        ret = -1;
        goto end;
    }

    /*del current vlan operation rule in  this port all vlan operation rule*/
    for(i = 0; i<OMCI_MAX_PORT_VLAN_RULE_NUM; i++){
        tempVlanRule_ptr = vlanRule_ptr+i;
        if((tempVlanRule_ptr->filter_outer_tpid ==  delRule_ptr->filter_outer_tpid)
            && (tempVlanRule_ptr->filter_outer_pri ==  delRule_ptr->filter_outer_pri)
            && (tempVlanRule_ptr->filter_outer_dei ==  delRule_ptr->filter_outer_dei)
            && (tempVlanRule_ptr->filter_outer_vid ==  delRule_ptr->filter_outer_vid)
            && (tempVlanRule_ptr->filter_inner_tpid ==  delRule_ptr->filter_inner_tpid)
            && (tempVlanRule_ptr->filter_inner_pri ==  delRule_ptr->filter_inner_pri)
            && (tempVlanRule_ptr->filter_inner_dei ==  delRule_ptr->filter_inner_dei)
            && (tempVlanRule_ptr->filter_inner_vid ==  delRule_ptr->filter_inner_vid)
            && (tempVlanRule_ptr->tag_num ==  delRule_ptr->tag_num)
            && (tempVlanRule_ptr->filter_ethertype ==  delRule_ptr->filter_ethertype)
            && (tempVlanRule_ptr->rule_priority ==  delRule_ptr->rule_priority)
            ){
            ret = pon_del_vlan_rule(vlanRulePortId, i);
            if(ret != 0){
                ret = -1;
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delAllVlanTagOpRuleByInstId-> pon_del_vlan_rule, i = %d, error", i);
                goto end;
            }else{
//              omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delAllVlanTagOpRuleByInstId-> pon_del_vlan_rule, i = %d, ok", i);
            }
        }
        
    }

    ret = 0;
end:
    if(vlanRule_ptr != NULL){
        free(vlanRule_ptr);
    }
    if(ret != 0){
        ret = -1;
    }
    return ret;

}


/*******************************************************************************************
**function name
    setExtVlanOpTBLDefaultRule
**description:
    set default ruel for untagged, single tagged, double tagged frame
 **retrun :
    -1: failure
    0:  success
**parameter:
    
********************************************************************************************/
int setExtVlanOpTBLDefaultRule(void){
    int ret = -1;
    pon_vlan_rule currVlanRule1;
    pon_vlan_rule_p currVlanRule1_ptr = NULL;

    /*init vlan tag opration rule*/
    currVlanRule1_ptr = &currVlanRule1; 
    ret = initVlanTagOpRule(currVlanRule1_ptr);
    if(ret != 0){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setExtVlanOpTBLDefaultRule->initVlanTagOpRule, fail");
        goto end;
    }

    /*1. set default rule for untagged frame*/
    currVlanRule1_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_0;
    currVlanRule1_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_HIGH;

    ret = findVlanTagOpRuleByRuleInfo(OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT, currVlanRule1_ptr);
    if(ret != 0){//no find
        ret = pon_add_vlan_rule(OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT, currVlanRule1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setExtVlanOpTBLDefaultRule->9.3.13  add untagged default vlan rule, fail");
        }
    }

    /*2. set default rule for 1 tagged frame*/
    currVlanRule1_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_1;
    currVlanRule1_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_HIGH;

    ret = findVlanTagOpRuleByRuleInfo(OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT, currVlanRule1_ptr);
    if(ret != 0){//no find
        ret = pon_add_vlan_rule(OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT, currVlanRule1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setExtVlanOpTBLDefaultRule->9.3.13  add 1 tagged default vlan rule, fail");
        }
    }

    /*3. set default rule for 1 tagged frame*/
    currVlanRule1_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_2;
    currVlanRule1_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_HIGH;

    ret = findVlanTagOpRuleByRuleInfo(OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT, currVlanRule1_ptr);
    if(ret != 0){//no find
        ret = pon_add_vlan_rule(OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT, currVlanRule1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setExtVlanOpTBLDefaultRule->9.3.13  add 2 tagged default vlan rule, fail");
        }
    }

    ret = 0;    
end:
    if(ret != 0){
        ret = -1;
    }
    return ret;
}

/*******************************************************************************************
**function name
    swapOmciVlanTag
**description:
    swap add_Fst tag with add_sec tag in rule
 **retrun :
    -1: failure
    0:  success
**parameter:
    rule_ptr :  original vlan rule information
********************************************************************************************/
int swapOmciVlanTag(IN pon_vlan_rule_p rule_ptr){
    uint8 tempU8 = 0;
    uint16 tempU16 = 0;
    uint32 tempU32 = 0;

    if(rule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n swapOmciVlanTag,rule_ptr == NULL,fail");
        return -1;
    }
    
    tempU32 = rule_ptr->add_fst_tpid;
    rule_ptr->add_fst_tpid = rule_ptr->add_sec_tpid;
    rule_ptr->add_sec_tpid = tempU32;

    tempU8 = rule_ptr->add_fst_pri;
    rule_ptr->add_fst_pri = rule_ptr->add_sec_pri;
    rule_ptr->add_sec_pri = tempU8;

    tempU8 = rule_ptr->add_fst_dei;
    rule_ptr->add_fst_dei = rule_ptr->add_sec_dei;
    rule_ptr->add_sec_dei = tempU8;

    tempU16 = rule_ptr->add_fst_vid;
    rule_ptr->add_fst_vid = rule_ptr->add_sec_vid;
    rule_ptr->add_sec_vid = tempU16;

    return 0;
}   


/*******************************************************************************************
**function name
    setExtVlanTagRuleByPkt
**description:
    according 9.3.13 entry , set vlan rule information to rule_ptr.
 **retrun :
    -1: failure
    0:  success
**parameter:
    rule_ptr :  return vlan rule information
    defaultRuleFlag: whether it  is  default vlan port.
    val_ptr:    16 bytes value
    size:       const 16 (16bytes = a entry)
********************************************************************************************/
int setExtVlanTagRuleByPkt(OUT pon_vlan_rule_p rule_ptr, OUT uint8 * defaultRuleFlag,  IN uint8 *val_ptr, IN uint8 size){
    int ret = -1;
    uint8 tempU8 = 0;
    uint16 tempU16 = 0;
    uint32 tempU32 = 0;
    uint8 * temp_ptr = NULL;
    uint8 * delRule_ptr = NULL;
    uint8 filterOuterTag = OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1;
    uint8 filterInnerTag = OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1;
    uint8 addOuterTag = OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1;
    uint8 addInnerTag = OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1;
    uint8 treatmentMode = 0;
    uint8 delEntryFlag = OMCI_EXT_VLAN_TAG_NO_DEL_ENTRY;
    int i = 0;

    if(rule_ptr == NULL || val_ptr == NULL || size != OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setExtVlanTagRuleByPkt-> parameter is error, fail");
        goto end;
    }
    
    rule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_HIGH;
    
    temp_ptr = val_ptr;
    
    /*The OLT deletes a tables entry by setting all of its last eight bytes to 0xFF*/
    delRule_ptr = temp_ptr + OMCI_EXT_VLAN_TAG_ATTR_JUST_8BYTES;
    for(i = 0; i < OMCI_EXT_VLAN_TAG_ATTR_LAST_8BYTES; i++){
        if(*(delRule_ptr+i) != 0xFF){
            break;
        }
    }
    if(i >= OMCI_EXT_VLAN_TAG_ATTR_LAST_8BYTES){
        delEntryFlag = OMCI_EXT_VLAN_TAG_DEL_ENTRY;
    }
    
    /*1. first 4 bytes*/
    tempU32 = get32(temp_ptr);
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU32_1=0x%02x",tempU32);

    /*set filter outer pbit value in rule*/
    tempU8  =  ((tempU32 >> 28) & 0x0f); //filter outer pbit
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU8 outer pri=0x%02x",tempU8);
    if((tempU8>=OMCI_VLAN_TAG_PBIT_0) && (tempU8<=OMCI_VLAN_TAG_PBIT_7)){
        rule_ptr->filter_outer_pri = tempU8;                
    }else if(tempU8 == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_8){
        rule_ptr->filter_outer_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    }else if(tempU8 == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_14){
        rule_ptr->filter_outer_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
        *defaultRuleFlag = OMCI_EXT_VLAN_TAG_OP_TBL_DEFAULT_RULE_FLAG_1;
    }else if(tempU8 == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_15){
        rule_ptr->filter_outer_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
        filterOuterTag = OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_0;
    }else{//don't happen
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 filter outer priority value,1 fail");
        ret = -1;
        goto end;
    }

    /*set filter outer vid value in rule*/
    tempU16 = ((tempU32>>15) &0x1fff); //filter outer vid
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU16 outer vid=0x%02x",tempU16);
    
    if((tempU16>OMCI_VLAN_TAG_VID_4094)
        && (tempU16 != OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096)){//don't happen
//      omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 filter outer vid  value,2 fail");
        ret = -1;
        goto end;
    }
    rule_ptr->filter_outer_vid = tempU16;// valid 0~4094, 4096

    /*set filter outer TPID/DEI value in rule*/
    tempU16 = ((tempU32>>12) &0x07);//filter outer TPID/DEI
    //omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU16 filter outer TPID/DEI=0x%02x",tempU16);
    
    switch(tempU16){
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE0:
            rule_ptr->filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
            rule_ptr->filter_outer_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE4:
            rule_ptr->filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_1;
            rule_ptr->filter_outer_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE5:
            rule_ptr->filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_2;
            rule_ptr->filter_outer_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE6:
            rule_ptr->filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_2;
            rule_ptr->filter_outer_dei = OMCI_VLAN_TAG_DEI_0;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE7:
            rule_ptr->filter_outer_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_2;
            rule_ptr->filter_outer_dei = OMCI_VLAN_TAG_DEI_1;
            break;                  
        default://don't happen
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 filter outer TPID/DEl value,3 fail");
            ret = -1;
            goto end;
            break;
    }

    /*2. second 4 bytes*/
    tempU32 = get32(temp_ptr+4); //filter inner information
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU32_2=0x%02x",tempU32);   
    
    /*set filter inner pbit value in rule*/
    tempU8  =  ((tempU32 >> 28) & 0x0f); 
    //omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU8 filter inner pbit=0x%02x",tempU8);
    
    if((tempU8>=OMCI_VLAN_TAG_PBIT_0) && (tempU8<=OMCI_VLAN_TAG_PBIT_7)){
        rule_ptr->filter_inner_pri = tempU8;                
    }else if(tempU8 == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_8){
        rule_ptr->filter_inner_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
    }else if(tempU8 == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_14){
        rule_ptr->filter_inner_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
        *defaultRuleFlag = OMCI_EXT_VLAN_TAG_OP_TBL_DEFAULT_RULE_FLAG_1;
    }else if(tempU8 == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_15){
        rule_ptr->filter_inner_pri = OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8;
        filterInnerTag = OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_0;
    }else{//don't happen
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 filter inner priority value,4 fail");
        ret = -1;
        goto end;
    }

    tempU16 = ((tempU32>>15) &0x1fff); //filter outer vid
    //omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU16 filter outer vid=0x%02x",tempU16);
    if((tempU16>OMCI_VLAN_TAG_VID_4094)
        && (tempU16 != OMCI_VLAN_TAG_OP_RULE_FILTER_VID_4096)){//don't happen
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 filter inner vid  value,5 fail");
        ret = -1;
        goto end;
    }
    rule_ptr->filter_inner_vid = tempU16;// valid 0~4094, 4096

    /*set filter inner TPID/DEI value in rule*/
    tempU16 = ((tempU32>>12) &0x07);//filter inner TPID/DEI
    //omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU16 filter inner TPID/DEI=0x%02x",tempU16);
    switch(tempU16){
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE0:
            rule_ptr->filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_0;
            rule_ptr->filter_inner_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE4:
            rule_ptr->filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_1;
            rule_ptr->filter_inner_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE5:
            rule_ptr->filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_2;
            rule_ptr->filter_inner_dei = OMCI_VLAN_TAG_OP_RULE_FILTER_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE6:
            rule_ptr->filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_2;
            rule_ptr->filter_inner_dei = OMCI_VLAN_TAG_DEI_0;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_TPID_MODE7:
            rule_ptr->filter_inner_tpid = OMCI_VLAN_TAG_OP_RULE_FILTER_TPID_2;
            rule_ptr->filter_inner_dei = OMCI_VLAN_TAG_DEI_1;
            break;                  
        default://don't happen
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 filter inner TPID/DEl value,6 fail");
            ret = -1;
            goto end;
            break;
    }
    
    /*set tagNum value in rule*/
    if(filterOuterTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1){
        rule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_2;
    }else if(filterInnerTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1){
        rule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_1;
    }else{
        rule_ptr->tag_num = OMCI_VLAN_TAG_OP_RULE_TAG_NUM_0;
    }
    
    /*set filter etherType value in rule*/
    tempU8 = (tempU32 & 0x0f);
    //omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU8 filter etherType=0x%02x",tempU8);
    
    switch(tempU8){
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE0:
            rule_ptr->filter_ethertype = OMCI_VLAN_TAG_OP_RULE_FILTER_ETHERTYPE_0;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE1:
            rule_ptr->filter_ethertype = 0x0800;//IPoE frame
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE2:
            rule_ptr->filter_ethertype = OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PPPoE_8863; //PPPoE frame
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE3:
            rule_ptr->filter_ethertype = 0x0806; //arp frame
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_ETHERTYPE_MODE4:
            rule_ptr->filter_ethertype = 0x86dd; //IPv6 IPoE frame
            break;
        default://don't happen
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 filter ethertype value,7 fail");
            ret = -1;
            goto end;
            break;
    }

    /*no need set treatment field in delete entry*/
    if(delEntryFlag == OMCI_EXT_VLAN_TAG_DEL_ENTRY){
        ret = 0;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 delete entry ");
        goto end;
    }
    /*3. third 4 bytes*/
    tempU32 = get32(temp_ptr+8); //treatment tags to remove, treatment outer attribute
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU32_3=0x%02x",tempU32);   

    /*set treamtent tags to remove value*/
    tempU8 = ((tempU32>>30)&0x03);
    //omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU8 treamtent tags to remove=0x%02x",tempU8);
    
    treatmentMode = tempU8;

    /*set treamet outer priority value in vlan rule*/
    tempU8 = ((tempU32>>16)&0x0f);
    //omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU8 treamet outer priority=0x%02x",tempU8);
    
    if(((tempU8>=OMCI_VLAN_TAG_PBIT_0) && (tempU8<=OMCI_VLAN_TAG_PBIT_7))
        || (tempU8 == OMCI_VLAN_TAG_OP_RULE_ADD_PRI_8)
        || (tempU8 == OMCI_VLAN_TAG_OP_RULE_ADD_PRI_9)
        || (tempU8 == OMCI_VLAN_TAG_OP_RULE_ADD_PRI_10)){
        rule_ptr->add_sec_pri = tempU8;
    }else if(tempU8 == OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_PBIT_15){
        addOuterTag = OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_0;
        rule_ptr->add_sec_pri = OMCI_VLAN_TAG_PBIT_0;
    }else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 treamet outer priority value,8 fail");
        ret = -1;
        goto end;
    }

    /*set treamet outer vid value in vlan rule*/
    tempU16 = ((tempU32>>3)&0x1fff);
    if((tempU16>OMCI_VLAN_TAG_VID_4094)
        && (tempU16 != OMCI_VLAN_TAG_OP_RULE_ADD_VID_4096)
        && (tempU16 != OMCI_VLAN_TAG_OP_RULE_ADD_VID_4097)){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 treamet outer vid value,9 fail");
        ret = -1;
        goto end;
    }
    rule_ptr->add_sec_vid = tempU16;// valid 0~4094, 4096:copy form the inner VID of the received frame, 4097:copy form the outer VID of the received frame

    /*set treamet outer TPID/DEI value in vlan rule*/
    tempU16 = (tempU32&0x07);//treatment outer TPID/DEI
    switch(tempU16){
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE0:
            rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_2;
            rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE1:
            rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_3;
            rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_3;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE2:
            rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE3:
            rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_3;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE4:
            rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_0;
            rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_0;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE6:
            rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_0;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE7:
            rule_ptr->add_sec_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            rule_ptr->add_sec_dei = OMCI_VLAN_TAG_DEI_1;
            break;                      
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE5:
        default://don't happen
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 treamet outer TPID/DEl value,10 fail");
            ret = -1;
            goto end;
            break;
    }

    /*4. fourth 4 bytes*/
    tempU32 = get32(temp_ptr+12); //treatment inner attribute
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanTagRuleByInst-> 9.3.13 tempU32_4=0x%02x",tempU32);   
    
    /*set treamet inner priority value in vlan rule*/
    tempU8 = ((tempU32>>16)&0x0f);
    if(((tempU8>=OMCI_VLAN_TAG_PBIT_0) && (tempU8<=OMCI_VLAN_TAG_PBIT_7))
        || (tempU8 == OMCI_VLAN_TAG_OP_RULE_ADD_PRI_8)
        || (tempU8 == OMCI_VLAN_TAG_OP_RULE_ADD_PRI_9)
        || (tempU8 == OMCI_VLAN_TAG_OP_RULE_ADD_PRI_10)){
        rule_ptr->add_fst_pri = tempU8;
    }else if(tempU8 == OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_PBIT_15){
        addInnerTag = OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_0;
    }else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 treamet inner priority value,11 fail");
        ret = -1;
        goto end;
    }
    
    /*set treamet inner  vid value in vlan rule*/
    tempU16 = ((tempU32>>3)&0x1fff);
    if((tempU16>OMCI_VLAN_TAG_VID_4094)
        && (tempU16 != OMCI_VLAN_TAG_OP_RULE_ADD_VID_4096)
        && (tempU16 != OMCI_VLAN_TAG_OP_RULE_ADD_VID_4097)){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 treamet inner vid value,12 fail");
        ret = -1;
        goto end;
    }
    rule_ptr->add_fst_vid = tempU16;// valid 0~4094, 4096:copy form the inner VID of the received frame, 4097:copy form the outer VID of the received frame

    /*set treamet inner TPID/DEI value in vlan rule*/
    tempU16 = (tempU32&0x07);//treatment inner TPID/DEI
    switch(tempU16){
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE0:
            rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_2;
            rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE1:
            rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_3;
            rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_3;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE2:
            rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_2;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE3:
            rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_3;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE4:
            rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_0;
            rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_0;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE6:
            rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_0;
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE7:
            rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_1;
            break;                      
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TPID_MODE5:
        default://don't happen
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 treamet inner TPID/DEl value,13 fail");
            ret = -1;
            goto end;
            break;
    }
        
    /*set treatment_method in vlan rule*/
    switch(treatmentMode){
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TAG_MODE0:
            if(addOuterTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1){
                rule_ptr->treatment_method =  OMCI_VLAN_TAG_OP_RULE_OP_22;//add 2 tag
            }else if(addInnerTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1){
                rule_ptr->treatment_method =  OMCI_VLAN_TAG_OP_RULE_OP_21;//add 1 tag
            }else{
                rule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_0;//transparent 
            }
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TAG_MODE1:
            if(addOuterTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1){//del 1 tag and  add 2 tags
                rule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_23;
                //use add_sec field to store change info
                if(swapOmciVlanTag(rule_ptr) == -1){
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst->swapOmciVlanTag fail[%d]",__LINE__);
                ret = -1;
                goto end;
            }
            }else if(addInnerTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1){
                if(rule_ptr->tag_num == OMCI_VLAN_TAG_OP_RULE_TAG_NUM_1){
                    rule_ptr->treatment_method =  OMCI_VLAN_TAG_OP_RULE_OP_40;//modify 1 inner tag
                }else if(rule_ptr->tag_num == OMCI_VLAN_TAG_OP_RULE_TAG_NUM_2){
                    rule_ptr->treatment_method =  OMCI_VLAN_TAG_OP_RULE_OP_41;//modify 1 outer tag and only set second treatment

                    /*update second treament value from first treamet value*/
                    rule_ptr->add_sec_tpid = rule_ptr->add_fst_tpid;
                    rule_ptr->add_sec_pri = rule_ptr->add_fst_pri;
                    rule_ptr->add_sec_dei = rule_ptr->add_fst_dei;
                    rule_ptr->add_sec_vid = rule_ptr->add_fst_vid;

                    /*set first treament to default value*/
                    rule_ptr->add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_0;
                    rule_ptr->add_fst_pri = OMCI_VLAN_TAG_PBIT_0;
                    rule_ptr->add_fst_dei = OMCI_VLAN_TAG_DEI_0;
                    rule_ptr->add_fst_vid = OMCI_VLAN_TAG_VID_4094; 
                    
                }else{//don't happen
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 tag_num value = %d, 15 fail",rule_ptr->tag_num);
                    ret = -1;
                    goto end;
                }
            }else{
                rule_ptr->treatment_method =  OMCI_VLAN_TAG_OP_RULE_OP_31;//del 1  tag
            }
            break;
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TAG_MODE2:
            if((addOuterTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_0)
                &&  (addInnerTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_0)){
                rule_ptr->treatment_method =  OMCI_VLAN_TAG_OP_RULE_OP_32;//del 2  tag
            }else if((addOuterTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1)
                &&  (addInnerTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1)){//del 2 tags and add 2 tags
                rule_ptr->treatment_method =  OMCI_VLAN_TAG_OP_RULE_OP_42;
            }else if((addOuterTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_0)
                &&  (addInnerTag == OMCI_EXT_VLAN_TAG_OP_TBL_HANDLE_TAG_1)){//del 2 tag and  add 1 tag
                rule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_33;
                //use add_sec field to store change info
                if(swapOmciVlanTag(rule_ptr) == -1){
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst->swapOmciVlanTag fail[%d]",__LINE__);
                    ret = -1;
                    goto end;
                }               
            }else{//no happen
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> fail[%d]",__LINE__);
                ret = -1;
                goto end;
            }
            break;                  
        case OMCI_EXT_VLAN_TAG_OP_TBL_TREATMENT_TAG_MODE3:
            rule_ptr->treatment_method = OMCI_VLAN_TAG_OP_RULE_OP_1;//block
            break;                  
        default://don't happen
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 treamet tag removel value,17 fail");
            ret = -1;
            goto end;
            break;
    }

    rule_ptr->rule_priority = OMCI_VLAN_TAG_OP_RULE_PRI_HIGH;

    ret = 0;
end:
    if(ret != 0){
        ret = -1;
    }
    return ret;
    
}

/*******************************************************************************************
**function name
    setVlanTagRuleByInst
**description:
    set vlan operation rule to specified port
 **retrun :
    -1: failure
    0:  success
**parameter:
    classId:    9.3.12 or 9.3.13 class id
    instId: 9.3.12 or 9.3.13 instance id
    vlanRulePortId :    vlan rule port Id
********************************************************************************************/
int setVlanTagRuleByInst(IN uint16 classId, IN uint16 instId, IN  uint8 vlanRulePortId){
    int ret = -1;
    pon_vlan_rule currVlanRule1;
    pon_vlan_rule currVlanRule2;
    pon_vlan_rule_p currVlanRule1_ptr = NULL;
    pon_vlan_rule_p currVlanRule2_ptr = NULL;
    uint8 * attrValue = NULL;
    uint8 upMode = 0;
    uint16 tci = 0;
    uint8 downMode = 0;
    uint16 inputTPID = 0;
    uint16 outputTPID = 0;
    uint32 * dscpMapU32_ptr = NULL;
    uint8 *temp_ptr = NULL;
    int i = 0;
    omciTableAttriValue_t *tmpTbl_ptr = NULL;
    omciMeInst_t *meInst_p = NULL;
    omciManageEntity_ptr me_p = NULL;
    uint32 tableSize = 0;
    uint32 len = 0;
    uint8 defaultRuleFlag = 0;
    uint8 currVlanRulePortId = 0;

    if((classId != OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE )
        && (classId != OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE )){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> classId don't support, classId = 0x%02x, fail", classId);
        goto end;
    }

    /*malloc for saving attribute value*/
    attrValue =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
    if(attrValue == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> attrValue calloc error ");
        ret = -1;
        goto end;
    }   

    /*malloc for DSCP mapping*/
    dscpMapU32_ptr = (uint32 *)calloc(OMCI_DSCP_MAP_U32_NUM, sizeof(uint32));
    if(dscpMapU32_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> dscpMapU32_ptr calloc error ");
        ret = -1;
        goto end;
    }

    /*get instance */
    me_p = omciGetMeByClassId(classId);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst->omciGetMeByClassId fail");
        ret = -1;
        goto end;
    }
    
    meInst_p = omciGetInstanceByMeId(me_p , instId);
    if(meInst_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst->omciGetInstanceByMeId fail");
        ret = -1;
        goto end;
    }   
    
    /*init vlan tag opration rule*/
    currVlanRule1_ptr = &currVlanRule1;
    currVlanRule2_ptr = &currVlanRule2; 
    ret = initVlanTagOpRule(currVlanRule1_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> initVlanTagOpRule currVlanRule1_ptr error ");
        ret = -1;
        goto end;
    }
    ret = initVlanTagOpRule(currVlanRule2_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> initVlanTagOpRule currVlanRule2_ptr error ");
        ret = -1;
        goto end;
    }

    /*set 9.3.12 vlan tag rule*/
    if(classId == OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE){
        /*get 9.3.12 Upsream tag operation mode attribute value*/
        ret = getAttributeValueByInstId(classId, instId, 1, attrValue, 1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> getAttributeValueByInstId  Upsream tag operation mode  fail ");
            ret = -1;
            goto end;
        }   
        upMode = *attrValue;

        /*get 9.3.12 Upstream TCI Value attribute value*/
        ret = getAttributeValueByInstId(classId, instId, 2, attrValue, 2);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> getAttributeValueByInstId  Upsream TCI fail ");
            ret = -1;
            goto end;
        }
        tci = get16(attrValue);

        /*get 9.3.12 DownStream mode attribute value*/
        ret = getAttributeValueByInstId(classId, instId, 3, attrValue, 1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> getAttributeValueByInstId  downstream Mode, fail ");
            ret = -1;
            goto end;
        }
        downMode = *attrValue;
        
#if 1
        /* adjust downstream mode*/
        switch(downMode){
            case OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_0:
                break;
            case OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_ATT_STRIP_VAL:
                downMode = OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_2;
                break;
            default:
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpDownModeValue-> downMode doesn't support, fail");
                ret = -1;
                goto end;
        }
#endif

        /*set value to currVlanRule_ptr pointer*/
        ret = setVlanTagOpRuleValByTci(upMode, tci, currVlanRule1_ptr, currVlanRule2_ptr);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> setVlanTagOpRuleValByTci  fail ");
            ret = -1;
            goto end;
        }   

        /*set 9.3.12 vlan TPID  rule*/
        ret = pon_set_vlan_tpid(vlanRulePortId, OMCI_VLAN_TAG_OP_RULE_TPID_8100, OMCI_VLAN_TAG_OP_RULE_TPID_8100);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.12 pon_set_vlan_tpid, fail");
        }
        
        /*set  9.3.12  vlan operation rule*/        
        ret = pon_add_vlan_rule(vlanRulePortId, currVlanRule1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> pon_add_vlan_rule->currVlanRule1, fail");
        }
        ret = pon_add_vlan_rule(vlanRulePortId, currVlanRule2);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> pon_add_vlan_rule->currVlanRule2, fail");
        }

        /*set downstream mode rule*/
        ret = pon_set_vlan_downstream_mode(vlanRulePortId, downMode);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> pon_set_vlan_downstream_mode, fail");
        }

    }

    /*set 9.3.13 vlan tag rule*/
    if(classId == OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE){
        /*get 9.3.13 input TPID value*/
        ret = getAttributeValueByInstId(classId, instId, 3, attrValue, 2);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> getAttributeValueByInstId  9.3.13 input TPID fail ");
            ret = -1;
            goto end;
        }   
        inputTPID= get16(attrValue);

        /*get 9.3.13 output TPID value*/
        ret = getAttributeValueByInstId(classId, instId, 4, attrValue, 2);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> getAttributeValueByInstId  9.3.13 output TPID fail ");
            ret = -1;
            goto end;
        }   
        outputTPID = get16(attrValue);

        /*get 9.3.13 downstream mode value*/
        ret = getAttributeValueByInstId(classId, instId, 5, attrValue, 1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> getAttributeValueByInstId  9.3.13 downstream Mode fail ");
            ret = -1;
            goto end;
        }
        if(*attrValue == OMCI_EXT_VLAN_TAG_OP_DOWN_MODE_0){//reverse
            downMode = OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_1;
        }else if(*attrValue == OMCI_EXT_VLAN_TAG_OP_DOWN_MODE_1){//transparent
            downMode = OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_0;
        }else{
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> getAttributeValueByInstId  9.3.13 downstream Mode value is wrong,  fail ");
            ret = -1;
            goto end;
        }

        /*get 9.3.13 DSCP Mapping value*/
        ret = getAttributeValueByInstId(classId, instId, 8, attrValue, OMCI_DSCP_MAP_U8_NUM);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> getAttributeValueByInstId  9.3.13 dscp mapping fail ");
            ret = -1;
            goto end;
        }
        temp_ptr = attrValue;
        for(i = 0; i<OMCI_DSCP_MAP_U32_NUM; i++){
            *(dscpMapU32_ptr+i) = get32(temp_ptr);
            temp_ptr += 4;
        }

        /*get exten vlan operation table*/
        tmpTbl_ptr = omciGetTableValueByIndex(meInst_p, me_p->omciAttriDescriptList[6].attriIndex);
        if(tmpTbl_ptr == NULL){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMOPMulticastAclTableValueByType->omciGetTableValueByIndex fail");
            tableSize = 0;
        }else{
            tableSize = tmpTbl_ptr->tableSize;
        }

        /*set 9.3.13 vlan TPID  rule*/
        ret = pon_set_vlan_tpid(vlanRulePortId, inputTPID, outputTPID);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 pon_set_vlan_tpid, fail");
        }
            
        /*set 9.3.13  downstream mode rule*/
        ret = pon_set_vlan_downstream_mode(vlanRulePortId, downMode);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 pon_set_vlan_downstream_mode, fail");
        }

        /*set 9.3.13 dscp mapping rule*/
        ret = pon_set_vlan_dscp_map(vlanRulePortId, dscpMapU32_ptr);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 pon_set_vlan_dscp_map, fail");
        }

        /*set 9.3.13 vlan operation rule*/
        len = OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
        while(len <= tableSize){
            temp_ptr = tmpTbl_ptr->tableValue_p + len - OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
            defaultRuleFlag = 0;
            ret = setExtVlanTagRuleByPkt(currVlanRule1_ptr, &defaultRuleFlag, temp_ptr, OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN);
            if(ret == 0){
                if(defaultRuleFlag == OMCI_EXT_VLAN_TAG_OP_TBL_DEFAULT_RULE_FLAG_1){
                    currVlanRulePortId = OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT;
                    ret = pon_enable_default_vlan_rule(vlanRulePortId);
                    if(ret != 0){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst->9.3.13  pon_enable_default_vlan_rule, fail");
                        ret = -1;
                        goto end;
                    }
                }else{
                    currVlanRulePortId = vlanRulePortId;
                }
                
                /*set first vlan rule*/
                ret = pon_add_vlan_rule(currVlanRulePortId, currVlanRule1);
                if(ret != 0){
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst->9.3.13  pon_add_vlan_rule->currVlanRule1, len=%d, fail", len);
                }
                
                /*set second vlan rule */
                if(currVlanRule1_ptr->filter_ethertype == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PPPoE_8863){
                    currVlanRule1_ptr->filter_ethertype = 0x8864;
                    ret = pon_add_vlan_rule(currVlanRulePortId, currVlanRule1);
                    if(ret != 0){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst->9.3.13  pon_add_vlan_rule->currVlanRule1, len=%d, 2 fail", len);
                    }
                }               
            }else{
                //nothing
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 setExtVlanTagRuleByPkt, len=%d, fail",len);
            }
            len += OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;  
        }
    }


    ret = 0;
end:
    if(attrValue != NULL){
        free(attrValue);
    }
    if(dscpMapU32_ptr != NULL){
        free(dscpMapU32_ptr);
    }
    if(ret != 0){
        ret = -1;
    }
    return ret;
}

/*******************************************************************************************
**function name
    addVlanTagRuleByVlanPortId
**description:
    search 9.3.12 or 9.3.13 all instance, set vlan operation rule to specified port.
 **retrun :
    -1: failure
    0:  success
**parameter:
    classId:    9.3.12 or 9.3.13 class id
    vlanRulePortId :    vlan rule port Id
********************************************************************************************/
int addVlanTagRuleByVlanPortId(IN uint16 classId, IN uint8 vlanRulePortId){
    int ret = -1;
    uint16 instId = 0;
    omciManageEntity_ptr me_p = NULL;
    omciInst_t *omciInst_p = NULL;
    omciMeInst_t *inst_p = NULL;
    uint8 currVlanRulePortId = 0;

    /*get ME information*/
    me_p = omciGetMeByClassId(classId);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addVlanTagRuleByVlanPortId-> omciGetMeByClassId, fail");
        ret = -1;
        goto end;
    }

    /*search all instance*/
    omciInst_p = &(me_p->omciInst);
    if((omciInst_p == NULL) || (omciInst_p->omciMeInstList == NULL)){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addVlanTagRuleByVlanPortId-> omciInst_p == NULL");
        ret = 0;
        goto end;
    }
    inst_p = omciInst_p->omciMeInstList;
    while(inst_p != NULL){
        instId = get16(inst_p->attributeVlaue_ptr);
        /*get vlan rule port id*/
        ret = getVlanRulePortIdByInst(classId,instId, &currVlanRulePortId);
        if(ret == 0){
             /*add vlan rule in this port*/
            if(currVlanRulePortId == vlanRulePortId){
                if(setVlanTagRuleByInst(classId,instId,vlanRulePortId) != 0){
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addVlanTagRuleByVlanPortId-> setVlanTagRuleByInst fail");
                }
            }else{
//              omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addVlanTagRuleByVlanPortId-> no find vlanRulePortId, currVlanRulePortId=0x%02x,  vlanRulePortId = 0x%02x, classId= 0x%02x, instanceId= 0x%02x", 
//                  currVlanRulePortId, vlanRulePortId, classId, instId);
            }
        }else{
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addVlanTagRuleByVlanPortId-> getVlanRulePortIdByInst, fail");
        }
        inst_p = inst_p->next;
    }
    
    ret = 0;
end:
    if(ret != 0){
        ret = -1;
    }
    return ret;
}


/*******************************************************************************************
**function name
    setVlanRuleToDefaultByPort
**description:
    set default rule for this vlan rule port
 **retrun :
    -1: failure
    0:  success
**parameter:
    vlanRulePortId :    vlan rule port Id
********************************************************************************************/
int setVlanRuleToDefaultByPort(uint8 vlanRulePortId){
    uint32 map[6] = {0};
    int ret = -1;
    
    if(pon_clean_vlan_rule(vlanRulePortId) == -1){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanRuleToDefaultByPort->pon_clean_vlan_rule, fail");
        goto end;
    }
#if 1
    if(pon_set_vlan_downstream_mode(vlanRulePortId, OMCI_VLAN_TAG_OP_RULE_DEFAULT_DOWNSTREAM_MODE) == -1){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanRuleToDefaultByPort->pon_set_vlan_downstream_mode, fail");
        goto end;
    }

    if(pon_set_vlan_tpid(vlanRulePortId,0x8100,0x8100) == -1){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanRuleToDefaultByPort->pon_set_vlan_tpid, fail");
        goto end;
    }

    if(pon_set_vlan_dscp_map(vlanRulePortId,&map[0]) == -1){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanRuleToDefaultByPort->pon_set_vlan_dscp_map, fail");
        goto end;
    }
    if(vlanRulePortId != OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT)
    {
        if(pon_enable_default_vlan_rule(vlanRulePortId) == -1){
            ret = -1;
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanRuleToDefaultByPort->pon_enable_default_vlan_rule, fail");
            goto end;
        }
    }
#endif
    

    ret = 0;
end:
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setVlanRuleToDefaultByPort->vlanRulePortId = %d, ret = 0x%02x",vlanRulePortId, ret);
    return ret;
}

/*******************************************************************************************
**function name
    addGemPortMappingRuleByAniPort
**description:
    add gem port mapping ruel by ani port (not include 802.1p mode)
 **retrun :
    -1: failure
    0:  success
**parameter:
    instId: 9.3.4 instance id
********************************************************************************************/
int addGemPortMappingRuleByAniPort(IN uint16 instId){
    omciGemPortMapping_ptr newRule_ptr = NULL;  
    uint32 classIdInstId = 0;
    uint8 portId = 0;
    uint8 ifaceType = 0;
    uint16 gemPortList[MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM] = {0};
    uint8 validGemPortNum = 0;
    int ret = -1;
    int result = 0;
    uint8 ifcType = 0;
    
    /*get ME id(class id +instance id)*/
    classIdInstId = (OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA<<16);
    classIdInstId |= instId;

    /*don't support 802.1p */
    result = getMacBridgePortInstType(instId, &ifcType);
    if(result != OMCI_MAC_BRIDGE_PORT_WAN_IFC){// UNI side don't need conside.
        ret = 0;
        goto end;
    }else{
        if(ifcType == OMCI_MAC_BRIDGE_PORT_DOT1P){// don't include 802.1p
            ret = 0;
            omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addGemPortMappingRuleByAniPort->don't include 802.1p[%d]", __LINE__);
            goto end;
        }
    }
    
    /*get ANI port id*/
    result = getPortFromMacBridgeByInstId(&portId, &ifaceType, instId);
    if((result != 0) || (ifaceType != OMCI_MAC_BRIDGE_PORT_WAN_IFC)){//don't need add rule
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addGemPortMappingRuleByAniPort->not ANI port");
        ret = 0;
        goto end;
    }

    /*get gem port id*/
    if(getGemPortByMACBridgePortInst(instId, gemPortList, &validGemPortNum) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByAniPort->getGemPortByMACBridgePortInst,, instId=%d, fail",instId);
        goto end;
    }
    /*only support ANI mac bridge port point to the gem port*/
    if(validGemPortNum != 1){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addGemPortMappingRuleByAniPort->validGemPortNum=%d, fail",validGemPortNum);
        ret = 0;
        goto end;
    }
    
    /*malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByAniPort->calloc newRule_ptr  fail");
        goto end;
    }

    newRule_ptr->tagctl = (OMCI_GEMPORT_MAPPING_ANI_PORT
                        |OMCI_GEMPORT_MAPPING_GEMPORT
                        |OMCI_GEMPORT_MAPPING_CLASSID_INSTID);

//  newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG;
    newRule_ptr->aniPort = portId;
    newRule_ptr->gemPort = gemPortList[0];  
    newRule_ptr->classIdInstId = classIdInstId;
    if(addOmciGemPortMappingRule(newRule_ptr) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByAniPort-> addOmciGemPortMappingRule  untagged, fail");
        goto end;
    }
    
    ret = 0;
end:
    if(newRule_ptr != NULL){
        free(newRule_ptr);
    }

//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addGemPortMappingRuleByAniPort-> ret = 0x%02x", ret);   
    return ret;

}

/*******************************************************************************************
**function name
    delGemPortMappingRuleByMbp
**description:
    delete gem port mapping rule by mac bridge port
 **retrun :
    -1: failure
    0:  success
**parameter:
    instId: 9.3.4 instance id
********************************************************************************************/
int delGemPortMappingRuleByMbp(IN uint16 instId){
    int ret = -1;
    uint16 delMatchFlag = 0;
    omciGemPortMapping_ptr newRule_ptr = NULL;  
    uint8 portId = 0;
    uint8 ifaceType = 0;
    uint8 ifcType = 0;
    int result = 0;

    /*malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delGemPortMappingRuleByMbp->calloc newRule_ptr  fail");
        goto end;
    }

    /*get ANI/UNI port id*/
    result = getPortFromMacBridgeByInstId(&portId, &ifaceType, instId);
    if(result != 0){//don't need add rule
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delGemPortMappingRuleByMbp->not ANI/UNI port");
        ret = 0;
        goto end;
    }
    if(ifaceType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//WAN side
        delMatchFlag = OMCI_GEMPORT_MAPPING_ANI_PORT;
        newRule_ptr->aniPort = portId;
    }else{//LAN side
        result = getMacBridgePortInstType(instId, &ifcType);
        if((result == OMCI_MAC_BRIDGE_PORT_LAN_IFC)
            && ((ifcType == OMCI_MAC_BRIDGE_PORT_PPTP_UNI)
#ifdef TCSUPPORT_PON_IP_HOST
                || (ifcType == OMCI_MAC_BRIDGE_PORT_IP_CONFIG_HOST)
#endif
        ))
        {
            delMatchFlag = OMCI_GEMPORT_MAPPING_USERPORT;
            newRule_ptr->userPort = portId;
        }       
        else  // when delete other type's LAN iface, directly exit, added by lidong
        {
            ret = 0;
            goto end;
        }
    }   
    if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delGemPortMappingRuleByMbp->delOmciGemPortMappingRuleByMatchFlag, fail");
        goto end;
    }
    
    ret = 0;
end:

    if(newRule_ptr != NULL){
        free(newRule_ptr);
    }
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delGemPortMappingRuleByMbp-> ret = 0x%02x", ret);   
    return ret;
}

/*******************************************************************************************************************************
9.3.10 802.1p mapper service profile

********************************************************************************************************************************/
int omciMeInitFor8021pMapperProfile(omciManageEntity_t *omciManageEntity_p){
    omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
    
    /* init some param for this ME */
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omci8021pMapperProfileDeleteAction;

    return 0;
}
int omci8021pMapperProfileDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
    uint32 classIdInstId = 0;
    int ret = 0;
    uint16 delMatchFlag = 0;
    omciGemPortMapping_ptr newRule_ptr = NULL;

    classIdInstId = omciPayLoad->meId;
    
    /*del all gem port mapping rule about this dot1p instance id*/
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omci8021pMapperProfileDeleteAction----->classIdInstId=0x%02x",classIdInstId);
    delMatchFlag = OMCI_GEMPORT_MAPPING_CLASSID_INSTID;
    
    /*malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n newRule_ptr->calloc newRule_ptr  fail");
        return -1;
    }   
    newRule_ptr->classIdInstId = classIdInstId;

    ret = delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr);
    
    if(newRule_ptr != NULL){
        free(newRule_ptr);
    }   
    return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);
}

int setdot1pTPPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
    omciManageEntity_t *me_p = NULL;
    uint8 *attributeValuePtr = NULL;
    uint16 length;
    uint8 tpType = 0;   
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint16 currInstId = 0;
    uint32 classIdInstId = 0;
    omciGemPortMapping_ptr oldRule_ptr = NULL;
    omciGemPortMapping_ptr newRule_ptr = NULL;
    uint16 newTPInstId = 0;
    uint16 oldTPInstId = 0;
    uint16 modifyFlag = 0;
    
    if(value == NULL){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPointerValue->value == NULL, fail");
        goto end;
    }
    if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPointerValue->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail");
        goto end;
    }

    if(flag == OMCI_CREATE_ACTION_SET_VAL){
        ret = 0;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pTPPointerValue->create action");
        goto end;
    }
    
    /*get 802.1p ME*/
    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_802_1P);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPointerValue->omciGetMeByClassId fail");
        ret = -1;
        goto end;
    }
    /*get TP type attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[13].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPointerValue->omciGetInstAttriByName failure");
        ret = -1;
        goto end;
    }
    tpType = *attributeValuePtr;
    
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pTPPointerValue->tpType=%d",tpType);
    if(tpType != OMCI_DOT1P_TP_TYPE_PPTP_UNI){
        ret = 0;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pTPPointerValue->don't need modify gem port mapping rule");
        goto end;
    }
    
    /*get dot1p instance id*/
    currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_802_1P<<16);
    classIdInstId |= currInstId;
    /*set modify flag for modify gemp port mapping rule*/
    modifyFlag = (OMCI_GEMPORT_MAPPING_USERPORT | OMCI_GEMPORT_MAPPING_CLASSID_INSTID); 
    
    /*get old  tp pointer instance id*/
    attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
    oldTPInstId = get16(attributeValuePtr); 

    /*malloc and set value for old port mapping*/   
    oldRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(oldRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPointerValue->calloc oldRule_ptr  fail");
        ret = -1;
        goto end;
    }
    oldRule_ptr->classIdInstId= classIdInstId;
    oldRule_ptr->userPort = (oldTPInstId & 0xff) -1;

    /*get new tp pointer instance id*/
    newTPInstId = get16((uint8 *)value);
    /*malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPointerValue->calloc newRule_ptr  fail");
        ret = -1;
        goto end;
    }
    oldRule_ptr->classIdInstId= classIdInstId;
    oldRule_ptr->userPort = (newTPInstId & 0xff) -1;

    /*modify gem port maaping rule*/
    ret = modifyOmciGemPortMappingRule(modifyFlag, oldRule_ptr,  newRule_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPointerValue->modifyOmciGemPortMappingRule, fail");
        ret = -1;
        goto end;
    }
    
    ret = 0;
    
end:    
    if(oldRule_ptr != NULL)
        free(oldRule_ptr);
    if(newRule_ptr != NULL)
        free(newRule_ptr);
    if(ret == 0){       
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return ret; 
    }
}



/*******************************************************************************************
**function name
    updateGemMappingRuleByMB
**description:
    according the dot1p instance id, and mac bridge port instance, update gemport mapping in vlan filter ME.
 **retrun :
    -1: failure
    0:  success
**parameter:
    me_p:9.3.4 me pointer.
    inst_p: mac bridge port instance
    dot1pInstId :   instance id in 9.3.10 802.1p ME
********************************************************************************************/
int updateGemMappingRuleByMB(omciManageEntity_t *me_p, omciMeInst_t *inst_p, uint16 dot1pInstId){
    int ret = -1;
    uint8 *attributeValuePtr = NULL;
    uint16 length = 0;
    uint8 tpType = 0;
    uint16 tpPointer = 0;
    uint16 mbInstId = 0;
    uint8 maxValidNum = 0;
    uint8 vlanList[MAX_VLAN_FILTER_LIST_LEN] = {0};
    uint8 forwardOperation = 0;
    uint8 * oldValue = NULL;
    uint16 delMatchFlag = 0;
    omciGemPortMapping_ptr newRule_ptr = NULL;
    uint8 gemPortRuleLen = 0;
    uint32 classIdInstId = 0;

    if(me_p == NULL || inst_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB->me_p == NULL || inst_p == NULL");
        goto end;
    }

    /*malloc some space */
    /*malloc and set value for new port mapping*/
    gemPortRuleLen = sizeof(omciGemPortMapping_t);
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , gemPortRuleLen);
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB->newRule_ptr->calloc newRule_ptr  fail");
        ret = -1;
        goto end;
    }
    /*malloc space for vlan filter ME*/ 
    oldValue =  (uint8 *)calloc(1 , MAX_VLAN_FILTER_LIST_LEN);
    if(oldValue == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB-> oldValue calloc error ");
        ret = -1;
        goto end;
    }
    
    /*1. prepare for the update gemport mapping rule*/
    /*1.1 get TP Type attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[3].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB->omciGetInstAttriByName tp type fail");
        ret = -1;
        goto end;
    }
    tpType = *attributeValuePtr;
    if(tpType != OMCI_MAC_BRIDGE_PORT_TP_TYPE_3){
        ret = 0;
        goto end;
    }

    /*1.2 get TP pointer attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[4].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB->omciGetInstAttriByName tp pointer fail");
        ret = -1;
        goto end;
    }
    tpPointer = get16(attributeValuePtr);
    if(tpPointer != dot1pInstId){//check 802.1p instance id
        ret = 0;
        goto end;
    }

    /*2.update gemport mapping rule in vlan filter ME*/
    mbInstId = get16(inst_p->attributeVlaue_ptr);
    
    /*2.1.get Vlan filter ME vlan list attribute*/
    if(getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA, mbInstId, 1, oldValue, MAX_VLAN_FILTER_LIST_LEN) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB-> getAttributeValueByInstId 1 fail");
        ret = -1;
        goto end;
    }
    memcpy(vlanList, oldValue, MAX_VLAN_FILTER_LIST_LEN);

    /*2.2 get Vlan filter ME forward operation attribute*/
    if(getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA, mbInstId, 2, oldValue, 1) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB-> getAttributeValueByInstId  2 fail");
        ret = -1;
        goto end;
    }
    forwardOperation = *oldValue;

    /*2.3. get Vlan filter ME number of entry attribute*/
    if(getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA, mbInstId, 3, oldValue, 1) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB-> getAttributeValueByInstId 3 fail");
        ret = -1;
        goto end;
    }
    maxValidNum = *oldValue;

    /*2.4 del old gem port mapping rule*/
    memset(newRule_ptr, 0, gemPortRuleLen);
    delMatchFlag = OMCI_GEMPORT_MAPPING_CLASSID_INSTID;     
    /*get ME id(class id +instance id)*/
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA<<16);
    classIdInstId |= mbInstId;
    newRule_ptr->classIdInstId = classIdInstId;
    
    if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB->delOmciGemPortMappingRuleByMatchFlag, fail");
        ret = -1;
        goto end;       
    }

    /*2.5. add gem port rule */
    /*2.5.1 don't need handle when maxValidNum is 0*/
    if(maxValidNum == 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateGemMappingRuleByMB-> maxValidNum = 0 ");
        ret = 0;
        goto end;
    }
    /*2.5.2 add rule for gem port mapping rule*/
    if(addGemPortMappingRuleByVlanFilter(mbInstId, vlanList, maxValidNum, forwardOperation) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMB-> addGemPortMappingRuleByVlanFilter fail");
        ret = -1;
        goto end;
    }

    ret = 0;
end:
    
    if(oldValue != NULL){
        free(oldValue);
    }
    if(newRule_ptr != NULL){
        free(newRule_ptr);
    }
    return ret;
}
/*******************************************************************************************
**function name
    updateGemMappingRuleBy8021p
**description:
    When the pointing gemport for pbit is changed, the gemport mapping rule will been updated in vlan filter ME
 **retrun :
    -1: failure
    0:  success
**parameter:
    dot1pInstId :   instance id in 9.3.10 802.1p ME
********************************************************************************************/

int updateGemMappingRuleBy8021p(uint16 dot1pInstId){
    int ret = -1;
    omciManageEntity_t *me_p = NULL;
    omciInst_t *omciInst_p ;
    omciMeInst_t *inst_p = NULL;

    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleBy8021p->omciGetMeByClassId fail, don't update gemport mapping by 802.1p");
        goto end;
    }

    omciInst_p = &(me_p->omciInst); 
    /* find the instance and exec it */
    if(omciInst_p == NULL || omciInst_p->omciMeInstList == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n updateGemMappingRuleBy8021p omciMeInstList == NULL, don't update gemport mapping by 802.1p");
        goto end;
    }

    inst_p = omciInst_p->omciMeInstList;
    while(inst_p != NULL){      
        if(updateGemMappingRuleByMB(me_p, inst_p, dot1pInstId) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n updateGemMappingRuleBy8021p meID=%d, fail", get16(inst_p->attributeVlaue_ptr));
        }
        inst_p = inst_p->next;
    }
    
    ret = 0;
end:
    return ret;
}
int setdot1pTPPbitValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 pbit, uint8 flag){
    int ret = -1;
    int retcode = -1;
    omciManageEntity_t *me_p = NULL;
    uint8 *attributeValuePtr = NULL;
    uint16 length;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint16 currInstId = 0;
    uint32 classIdInstId = 0;
    omciGemPortMapping_ptr oldRule_ptr = NULL;
    omciGemPortMapping_ptr newRule_ptr = NULL;
    uint16 newTPInstId = 0;
    uint16 oldTPInstId = 0;
    uint16  newGemPortId = 0;
    uint16  oldGemPortId = 0;
    uint16 modifyFlag = 0;
    
    uint8 tpType = 0;
    uint8 unMarkOptFlag = 0;
    uint8 uniPort = 0;
    uint8 newDscpPits[MAX_OMCI_DSCP_KINDS] = {0};
    int i = 0;
    uint16 delMatchFlag = 0;
    uint8 updateGemPortMapRule = OMCI_NO_UPDATE_GEMPORT_MAPP_RULE;
    
    omciManageEntity_ptr me2_p = NULL;  
    omciInst_t *omciInst_p = NULL ;
    omciMeInst_t *inst_p = NULL;
    uint8 tempTpType = 0;
    uint16 tempTpPointer = 0;
    uint16 tempInstId = 0;
    uint8 findANIPortFlag = OMCI_VLAN_NO_FIND;
    uint16 oldTagCtl = 0;

    if(value == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->value == NULL, fail");
        goto end;
    }
    if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail");
        goto end;
    }

    if(pbit>7){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->pbit is error, pbit=%d, fail",pbit);
        goto end;
    }

    /*1. init*/
    /* 1.1 get dot1p instance id*/
    currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);    

    /*1.2 get new pbit  tp pointer instance id*/
    newTPInstId = get16((uint8 *)value);

    /*1.3 when create action , directed return success*/
    if(flag == OMCI_CREATE_ACTION_SET_VAL){
        ret = 0;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pTPPbitValue->pbit%d, create action",pbit);
        goto end;
    }

    /*1.4 malloc for new port mapping*/     
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->calloc newRule_ptr  fail");
        goto end;
    }
    /*1.5 malloc for old port mapping*/ 
    oldRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(oldRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->calloc oldRule_ptr  fail");
        goto end;
    }   

    /*1.6 set classIdInstId value for gem port mapping*/
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_802_1P<<16);
    classIdInstId |= currInstId;

    /*1.7 set updated mapping rule flag in vlan filter ME*/
    updateGemPortMapRule = OMCI_UPDATE_GEMPORT_MAPP_RULE;
    /*2. according require, delete pbit rule.*/
    if(newTPInstId == OMCI_DOT1P_INTERWORK_TP_DISCARD){
        /*delete mapping rule about this pbit*/
        delMatchFlag = (OMCI_GEMPORT_MAPPING_PBIT | OMCI_GEMPORT_MAPPING_CLASSID_INSTID);
        newRule_ptr->pbit = pbit;
        newRule_ptr->classIdInstId = classIdInstId;
        if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr) != 0){
            ret = -1;
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->newTPInstId == OMCI_DOT1P_INTERWORK_TP_DISCARD");
        }else{
            ret = 0;
        }
        goto end;
    }
    
    /*3. get some old and new instance attribute value*/
    /*3.1  get old pbit  tp pointer instance id */
    attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
    oldTPInstId = get16(attributeValuePtr);
        
    /*3.2 get 802.1p ME*/
    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_802_1P);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbitValue->omciGetMeByClassId fail");
        goto end;
    }

    /*3.3 get TP Type attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[13].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbitValue->omciGetInstAttriByName tp type, failure");
        ret = -1;
        goto end;
    }
    tpType = *attributeValuePtr;

    /*3.4 get TP pointer attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[1].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbitValue->omciGetInstAttriByName tp pointer, failure");
        ret = -1;
        goto end;
    }
//  tpPointer = get16(attributeValuePtr);
    if(tpType == OMCI_DOT1P_TP_TYPE_PPTP_UNI){
        uniPort = *(attributeValuePtr+1) -1;
    }


    /*3.5 get unmark frame option attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[10].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbitValue->omciGetInstAttriByName failure");
        ret = -1;
        goto end;
    }
    unMarkOptFlag = *attributeValuePtr;
    

    /*4 modify rule*/   
    if(oldTPInstId != OMCI_DOT1P_INTERWORK_TP_DISCARD){
        if(newTPInstId == oldTPInstId){
            ret = 0;
            updateGemPortMapRule = OMCI_NO_UPDATE_GEMPORT_MAPP_RULE;
            goto end;
        }
        /*4.1 set modify flag for modify gemp port mapping rule*/
        modifyFlag = (OMCI_GEMPORT_MAPPING_GEMPORT | OMCI_GEMPORT_MAPPING_CLASSID_INSTID |OMCI_GEMPORT_MAPPING_PBIT );  
        
        /*4.2 get old gem port id*/
        ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_GEM_INTERWORK_TP, oldTPInstId, &oldGemPortId);
        if(ret != 0){
            ret = -1;
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->getGemPortIdFromGemInterTP, fail");
            goto end;
        }

        /*4.3 set value for old port mapping*/
        oldRule_ptr->classIdInstId= classIdInstId;
        oldRule_ptr->pbit = pbit;
        oldRule_ptr->gemPort= oldGemPortId;
        
        /*4.4 get  new gem port id*/
        ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_GEM_INTERWORK_TP, newTPInstId, &newGemPortId);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->getGemPortIdFromGemInterTP #1697, fail");
            ret = -1;
            goto end;
        }
    
    
        /*4.5 set value for new port mapping*/      
        newRule_ptr->classIdInstId = classIdInstId;
        newRule_ptr->pbit = pbit;
        newRule_ptr->gemPort= newGemPortId;
    
    
        /*4.6 modify gem port maaping rule*/
        ret = modifyOmciGemPortMappingRule(modifyFlag, oldRule_ptr,  newRule_ptr);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->modifyOmciGemPortMappingRule, fail");
            ret = -1;
            goto end;
        }
        
    }else{
        /*5 add rule*/
        newRule_ptr->tagctl = (OMCI_GEMPORT_MAPPING_TAGFLAG
                                |OMCI_GEMPORT_MAPPING_PBIT
                                |OMCI_GEMPORT_MAPPING_GEMPORT
                                |OMCI_GEMPORT_MAPPING_CLASSID_INSTID);      
        if(tpType == OMCI_DOT1P_TP_TYPE_BRIDGE_MAPPING){
            //nothing
        }else if(tpType == OMCI_DOT1P_TP_TYPE_PPTP_UNI){
            newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_USERPORT;
            newRule_ptr->userPort = uniPort;
        }else{
            ret = -1;
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->tpType = %d, fail",tpType);
            goto end;
        }
        newRule_ptr->classIdInstId= classIdInstId;
        newRule_ptr->pbit = pbit;
        
        ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_GEM_INTERWORK_TP, newTPInstId, &newGemPortId);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->getGemPortIdFromGemInterTP #2343, fail");
            ret = -1;
            goto end;
        }
        newRule_ptr->gemPort = newGemPortId;

        /*5.1 add mapping rule for tagged frame*/
        newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_TAGGED_FLAG;

        /*5.1.1. backup tag ctrl value */
        oldTagCtl = newRule_ptr->tagctl;
        
        /*5.1.2. search API port and add API pbit gem port mapping */
        me2_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
        if(me2_p == NULL){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->omciGetMeByClassId fail");
            goto end;
        }
    
        /*traversal all 9.3.4 instance*/
        omciInst_p = &(me2_p->omciInst);
        if(omciInst_p == NULL){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> omciInst_p == NULL fail,[%d]", __LINE__);
            goto end;
        }
        inst_p = omciInst_p->omciMeInstList;
        
        while(inst_p != NULL){
            /*get 9.3.4 instance id*/
            tempInstId = get16(inst_p->attributeVlaue_ptr);     
            /*get 9.3.4 tp type attribute value*/
            attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me2_p->omciAttriDescriptList[3].attriName, &length);//tp type attribute
            if(attributeValuePtr == NULL){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> omciGetInstAttriByName, tp typeinstid=0x%02x,fail",tempInstId);
                goto end;
            }
            tempTpType= *attributeValuePtr;
            
            /*get 9.3.4 tp pointer attribute value*/
            attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me2_p->omciAttriDescriptList[4].attriName, &length);//tp pointer attribute
            if(attributeValuePtr == NULL){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> omciGetInstAttriByName, tp pointer , instid=0x%02x,fail",tempInstId);
                goto end;
            }
            tempTpPointer= get16(attributeValuePtr);
            if((tempTpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_3)
                && (tempTpPointer == currInstId)){// match ANI port and dot1p instance id , add ANI pbit gemport mapping rule.
                /*find ANI interface */
                for(i = 0; i< MAX_OMCI_ANI_PORT_NUM; i++){                      
                    if((validAniPortFlag[i][1] == OMCI_VALID_MAC_BRIDGE_PORT_FLAG)
                        && (validAniPortFlag[i][0] == tempInstId)){
                        
                        findANIPortFlag = OMCI_VLAN_FIND;
                        newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_ANI_PORT;
                        newRule_ptr->aniPort = i;
                                                
                        if(addOmciGemPortMappingRule(newRule_ptr) != 0){
                            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> addOmciGemPortMappingRule ANI tagged,ani = %d, fail[%d]", i, __LINE__);
                        }
                        
                    }
                }
            }
            
            inst_p = inst_p->next;
        }

        /*5.1.3. restore tag ctrl value */
        newRule_ptr->tagctl = oldTagCtl;

        /*5.1.4. only when no find ANI port, the pbit gemport mapping rule will be set for tagged frame*/
        if(findANIPortFlag ==  OMCI_VLAN_NO_FIND){          

            ret = addOmciGemPortMappingRule(newRule_ptr);
            if(ret != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> addOmciGemPortMappingRule tagged, fail");
                ret = -1;
                goto end;
            }
        }

        /*5.2 add mapping rule for untagged frame*/
        newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG;
        newRule_ptr->tagctl &= (~OMCI_GEMPORT_MAPPING_PBIT);
        if(unMarkOptFlag == OMCI_DOT1P_UNMARK_0){
            /*get dscp pbit mapping attribute in this instance*/
            attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[11].attriName, &length);
            if(attributeValuePtr == NULL){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->#[%d] omciGetInstAttriByName failure",__LINE__);
                ret = -1;
                goto end;
            }
            /*get Pbits by 24 bytes dscp pbits mapping*/
            ret = convertDSCPMap2Pbit(attributeValuePtr,newDscpPits);
            if(ret != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->convertDSCPMap2Pbit new DSCP pbit failure");
                ret = -1;
                goto end;
            }
            
            newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_DSCP;
            for(i = 0; i<MAX_OMCI_DSCP_KINDS; i++){
                if(newDscpPits[i] == pbit){
                    newRule_ptr->dscp = i;
                    ret = addOmciGemPortMappingRule(newRule_ptr);
                    if(ret != 0){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> #2383 addOmciGemPortMappingRule fail dsep=%d",i);
                        ret = -1;
                        goto end;
                    }
                }           
            }
        }else if(unMarkOptFlag == OMCI_DOT1P_UNMARK_1){

            /*get default pbit attribute in this instance*/
            attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[12].attriName, &length);
            if(attributeValuePtr == NULL){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value->#[%d] omciGetInstAttriByName failure",__LINE__);
                ret = -1;
                goto end;
            }

            if(pbit == *attributeValuePtr){
                retcode  = checkMappingRuleUntagAction(currInstId);
                if (1 == retcode || 0 == retcode)
                {
                    ret = addOmciGemPortMappingRule(newRule_ptr);
                    if(ret != 0){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->#[%d] addOmciGemPortMappingRule, fail",__LINE__);
                        ret = -1;
                        goto end;
                    }
                }
            }           

        }else{
            //nothing
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> unMarkOptFlag, fail");
            ret = -1;
            goto end;
        }
        
        
        
    }
    
    ret = 0;
        
    end:
    
#ifdef OMCI_GEM_PORT_MAPPING_DEBUG
                omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pTPPbit%dValue----->ret = 0x%02x",pbit, ret);
                /*LHS dbg Start*/
                omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pTPPbit->after");
                displayOmciCurrGemPortMappingRule();
                /*LHS dbg Start*/
#endif

        if(oldRule_ptr != NULL)
            free(oldRule_ptr);
        if(newRule_ptr != NULL)
            free(newRule_ptr);
        if(ret == 0){
            if(setGeneralValue(value, meInstantPtr , omciAttribute, flag)!= 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit->setGeneralValue, fail");
                return -1;
            }
            
        /*update gemport mapping in vlan filter ME*/
        if(updateGemPortMapRule == OMCI_UPDATE_GEMPORT_MAPP_RULE){
            if(updateAniInfoBy8021p(currInstId)== OMCI_MAC_BRIDGE_PORT_FAIL){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit->updateAniInfoBy8021p, fail");
            }           
            if(updateGemMappingRuleBy8021p(currInstId) != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit->updateGemMappingRuleBy8021p, fail");
            }
            if(updateGemMappingRuleByMbPortUNI(OMCI_UPDATE_UNI_MAPPING_BY_DOT1P_ME,currInstId) != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit->updateGemMappingRuleByMbPortUNI, fail");
            }

        }
            return 0;
        }else{
            return ret; 
        }
}

int setdot1pTPPbit0Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 pbit = 0;

    return setdot1pTPPbitValue(value, meInstantPtr, omciAttribute, pbit, flag);
}
int setdot1pTPPbit1Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 pbit = 1;

    return setdot1pTPPbitValue(value, meInstantPtr, omciAttribute, pbit, flag);
}
int setdot1pTPPbit2Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 pbit = 2;

    return setdot1pTPPbitValue(value, meInstantPtr, omciAttribute, pbit, flag);
}
int setdot1pTPPbit3Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 pbit = 3;

    return setdot1pTPPbitValue(value, meInstantPtr, omciAttribute, pbit, flag);
}
int setdot1pTPPbit4Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 pbit = 4;

    return setdot1pTPPbitValue(value, meInstantPtr, omciAttribute, pbit, flag);
}
int setdot1pTPPbit5Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 pbit = 5;

    return setdot1pTPPbitValue(value, meInstantPtr, omciAttribute, pbit, flag);
}
int setdot1pTPPbit6Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 pbit = 6;

    return setdot1pTPPbitValue(value, meInstantPtr, omciAttribute, pbit, flag);
}
int setdot1pTPPbit7Value(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 pbit = 7;

    return setdot1pTPPbitValue(value, meInstantPtr, omciAttribute, pbit, flag);
}


int getGemPortFromDot1p(IN uint16 instId, IN uint8 pbit, OUT uint16 * gemPort){
    omciMeInst_t *meInst_p = NULL;
    omciManageEntity_t *me_p = NULL;
    uint8 *attributeValuePtr = NULL;
    uint16 length = 0;
    uint8 index = 0;
    uint16 interworkTPInst = 0;
    int ret = -1;
    
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getGemPortFromDot1p->start pbit%d",pbit);

    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_802_1P);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortFromDot1p->omciGetMeByClassId fail");
        goto end;
    }
    meInst_p = omciGetInstanceByMeId(me_p , instId);
    if(meInst_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortFromDot1p->omciGetInstanceByMeId fail");
        goto end;
    }
    /*find pbit attribute*/
    index = 2+pbit;
    if(index > 9){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortFromDot1p->index > 9  fail");
        goto end;
    }
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[index].attriName, &length);
    interworkTPInst = get16(attributeValuePtr);
    if(interworkTPInst == OMCI_DOT1P_INTERWORK_TP_DISCARD){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getGemPortFromDot1p->interworkTPInst= 0x%02x, fail",interworkTPInst);
        ret = 0;
        *gemPort = OMCI_DOT1P_GEM_PORT_DISCARD;
        goto end;
    }
    ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_GEM_INTERWORK_TP, interworkTPInst, gemPort);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortFromDot1p->getGemPortIdFromGemInterTP,fail");
        ret = -1;
        goto end;
    }
    ret = 0;

end:
    if(ret != 0){
        ret = -1;
    }
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getGemPortFromDot1p->ret = %d, gemPort=%d",ret, *gemPort);  
    return ret;
}



int setdot1pUnmarkOptionValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint16 currInstId = 0;
    uint32 classIdInstId = 0;
    uint8 unmarkOptFlag = 0;
    uint8 * tempValue = NULL;
    uint8 * tempValue1 = NULL;
    uint16 len = 0;
    uint8 index = 0;
    omciManageEntity_t *me_p = NULL;
    uint8 *attributeValuePtr = NULL;    
    uint16 length = 0;;
    int i = 0;
    uint8 tempPbit = 0;
    uint16 tempGemPort = 0;
    uint8 dscpPits[MAX_OMCI_DSCP_KINDS] = {0};
    
    if(value == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->value == NULL, fail");
        ret = -1;
        goto end;
    }
    if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail");
        ret = -1;
        goto end;
    }

    if(flag == OMCI_CREATE_ACTION_SET_VAL){
        ret = 0;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pUnmarkOptionValue->create action");
        goto end;
    }


    /*get dot1p instance id*/
    currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

    /*prepare for modify mapping rule */
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_802_1P<<16);
    classIdInstId |= currInstId;
        
    /*get current attribute value and init some parameter*/ 
    unmarkOptFlag = *(uint8 *)value;
    tempValue = (uint8 *)calloc(1, MAX_OMCI_DSCP_PBIT_GPORT_LEN);
    if(tempValue == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->calloc fail");
        ret = -1;
        goto end;
    }
    tempValue1 = tempValue;
    if(unmarkOptFlag == 0){
//      len = MAX_OMCI_DSCP_PBIT_GPORT_LEN; // 64 entry [dscp,pbit,gem port]
        index = 11;//DSCP2Pbit attribute
    }else if(unmarkOptFlag == 1){
//      len = MAX_OMCI_PBIT_GPORT_LEN;// 1 entry [pbit, gem port]
        index = 12;//default pbit attribute
    }else{
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->unmarkOptFlag =%d, fail",unmarkOptFlag);
        goto end;
    }

    /*get Dscp2Pbit or default pbit attribute value in this instance*/
    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_802_1P);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->omciGetMeByClassId fail");
        ret = -1;
        goto end;
    }
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[index].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->omciGetInstAttriByName failure");
        ret = -1;
        goto end;
    }

    /*set value for modify gem port mapping rule*/
    if(unmarkOptFlag == 0){
        /*get Pbits by 24 bytes dscp pbits mapping*/
        ret = convertDSCPMap2Pbit(attributeValuePtr,dscpPits);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->convertDSCPMap2Pbit DSCP pbit failure");
            ret = -1;
            goto end;
        }
        /*set value for modify gem port mapping rule*/
        for(i = 0; i<MAX_OMCI_DSCP_KINDS; i++){
            ret = getGemPortFromDot1p(currInstId,  dscpPits[i], &tempGemPort);
            if(ret != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->getGemPortFromDot1p failure");
                ret = -1;
                goto end;
            }
            if(tempGemPort == OMCI_DOT1P_GEM_PORT_DISCARD){//don't need care this case.
                continue;
            }
            
            *(tempValue1++) = i;//set DSCP
            *(tempValue1++) = dscpPits[i];//set pbit
            put16(tempValue1, tempGemPort);//set gem port           
            tempValue1 +=  2;
            len += MAX_OMCI_DSCP_PBIT_GPORT_ITEM_LEN;
        }       
        
    }else if(unmarkOptFlag == 1){
        /*get pbits*/
        tempPbit = *attributeValuePtr;      
        /*get gem port by pbit*/
        ret = getGemPortFromDot1p(currInstId,  tempPbit, &tempGemPort);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->getGemPortFromDot1p failure");
            ret = -1;
            goto end;
        }
        if(tempGemPort != OMCI_DOT1P_GEM_PORT_DISCARD){
        /*set value for modify gem port mapping rule*/
        *(tempValue1++) = tempPbit;//set pbit
            put16(tempValue1, tempGemPort);//set gem port
        tempValue1 +=  2;
            len += MAX_OMCI_PBIT_GPORT_ITEM_LEN;
        }
    }else{
        //nothing
    }
    
    /*modify gem port mapping rule*/
    if(modifyOmciGemPortMappingRuleByUnmarkOpt(unmarkOptFlag, len, tempValue, classIdInstId) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pUnmarkOptionValue->modifyOmciGemPortMappingRuleByUnmarkOpt failure");
        ret = -1;
        goto end;
    }
    ret = 0;
    
    
end:    
    if(tempValue != NULL){
        free(tempValue);
    }
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return ret;
    }
}
int setdot1pDscpPbitMappValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
    int i = 0;

    uint8 newDscpPits[MAX_OMCI_DSCP_KINDS] = {0};
    uint8 *newDscpMap = NULL;
    uint16  newGemPortId = 0;;
    omciGemPortMapping_ptr newRule_ptr = NULL;
    uint16 delMatchFlag = 0;
    
    omciManageEntity_t *me_p = NULL;
    uint8 *attributeValuePtr = NULL;
    uint16 length;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint16 currInstId = 0;
    uint32 classIdInstId = 0;
    uint8 tpType = 0;
    uint8 uniPort = 0;
    uint8 newPbits = 0;
    
    if(value == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->value == NULL, fail");
        ret = -1;
        goto end;
    }
    if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail");
        ret = -1;
        goto end;
    }
    /*get dot1p instance id*/
    currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_802_1P<<16);
    classIdInstId |= currInstId;

    /*get unmark option attribute value in this instance*/
    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_802_1P);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->omciGetMeByClassId fail");
        ret = -1;
        goto end;
    }
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[10].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->omciGetInstAttriByName failure");
        ret = -1;
        goto end;
    }

    if(*attributeValuePtr != 0){//no need modify gem port mapping, because current rule can't use dscp rule.
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->unmark option attribute show that doesn't need set dscp mapping rule");
        ret = 0;
        goto end;
    }
    /*get TP Type attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[13].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->omciGetInstAttriByName tp type, failure");
        ret = -1;
        goto end;
    }
    tpType = *attributeValuePtr;

    /*get TP pointer attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[1].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->omciGetInstAttriByName tp pointer, failure");
        ret = -1;
        goto end;
    }
    if(tpType == OMCI_DOT1P_TP_TYPE_PPTP_UNI){
        uniPort = *(attributeValuePtr+1) -1;
    }
    
    /*get new dscp pbit mapping*/
    newDscpMap = (uint8 *)value;

    /*get new Pbits by 24 bytes dscp pbits mapping*/
    ret = convertDSCPMap2Pbit(newDscpMap,newDscpPits);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->convertDSCPMap2Pbit new DSCP pbit failure");
        ret = -1;
        goto end;
    }

    /*malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->calloc newRule_ptr  fail");
        ret = -1;
        goto end;
    }
    
    /*set delete flag for modify gemp port mapping rule*/
    delMatchFlag = (OMCI_GEMPORT_MAPPING_TAGFLAG
                | OMCI_GEMPORT_MAPPING_CLASSID_INSTID); 
    newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG;
    newRule_ptr->classIdInstId = classIdInstId;

    /*delete old mapping rule*/
    if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->delOmciGemPortMappingRuleByMatchFlag, fail");
                ret = -1;
                goto end;
            }
    /*add new mapping rule*/
    newRule_ptr->tagctl = (OMCI_GEMPORT_MAPPING_TAGFLAG
//                          |OMCI_GEMPORT_MAPPING_PBIT      //untag rule should not set PBit control flag
                            |OMCI_GEMPORT_MAPPING_GEMPORT
                            |OMCI_GEMPORT_MAPPING_DSCP
                            |OMCI_GEMPORT_MAPPING_CLASSID_INSTID);  
    /*set value for new port mapping*/      
    switch(tpType){
        case OMCI_DOT1P_TP_TYPE_BRIDGE_MAPPING://bridge-mapping
            break;
        case OMCI_DOT1P_TP_TYPE_PPTP_UNI://pptp ethernet uni
            newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_USERPORT;
            newRule_ptr->userPort = uniPort;
            break;
        default://not supoort others
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->newTpType=%d, fail",tpType);
                ret = -1;
                goto end;
            }
            newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG;
            newRule_ptr->classIdInstId= classIdInstId;

    for(i = 0; i<MAX_OMCI_DSCP_KINDS; i++){
        newPbits = newDscpPits[i];
        if(getGemPortFromDot1p(currInstId,  newPbits, &newGemPortId) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDscpPbitMappValue->#[%d] getGemPortFromDot1p new failure dscp=%d",__LINE__,i);
            ret = -1;
            goto end;
        }
        if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
            newRule_ptr->dscp = i;
            newRule_ptr->pbit = newPbits;
            newRule_ptr->gemPort= newGemPortId; 
            if(addOmciGemPortMappingRule(newRule_ptr)!= 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue-> ##[%d] addOmciGemPortMappingRule fail dsep=%d",__LINE__,i);
                ret = -1;
                goto end;
            }
    }
    }

            
    ret = 0;
end:
    if(newRule_ptr != NULL)
        free(newRule_ptr);  
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return ret;
    }
}
int setdot1pDefaultPbitValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
//  int i = 0;
    uint8 newPbits= 0;
    uint16  newGemPortId = 0;;
    omciGemPortMapping_ptr newRule_ptr = NULL;
    uint16 delMatchFlag = 0;    
    omciManageEntity_t *me_p = NULL;
    uint8 *attributeValuePtr = NULL;
    uint16 length;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint16 currInstId = 0;
    uint32 classIdInstId = 0;
    uint8 tpType = 0;
    uint8 uniPort = 0;
    
    if(value == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->value == NULL, fail");
        ret = -1;
        goto end;
    }
    if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail");
        ret = -1;
        goto end;
    }
    if(flag == OMCI_CREATE_ACTION_SET_VAL){
        ret = 0;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pDefaultPbitValue->create action");
        goto end;
    }
    /*get dot1p instance id*/
    currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_802_1P<<16);
    classIdInstId |= currInstId;

    /*get unmark option attribute value in this instance*/
    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_802_1P);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->omciGetMeByClassId fail");
        ret = -1;
        goto end;
    }
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[10].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->omciGetInstAttriByName failure");
        ret = -1;
        goto end;
    }

    if(*attributeValuePtr != 1){//no need modify gem port mapping, because current rule can't use default pbit rule.
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pDefaultPbitValue->don't need modify this default mappint rule");
        ret = 0;
        goto end;
    }
    
    /*get TP Type attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[13].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->omciGetInstAttriByName tp type, failure");
        ret = -1;
        goto end;
    }
    tpType = *attributeValuePtr;
    
    /*get TP pointer attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[1].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->omciGetInstAttriByName tp pointer, failure");
        ret = -1;
        goto end;
    }
    if(tpType == OMCI_DOT1P_TP_TYPE_PPTP_UNI){
        uniPort = *(attributeValuePtr+1) -1;
    }
    
    /*malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->calloc newRule_ptr  fail");
        ret = -1;
        goto end;
    }
    
    /*set delete flag for modify gemp port mapping rule*/
    delMatchFlag = (OMCI_GEMPORT_MAPPING_TAGFLAG
                | OMCI_GEMPORT_MAPPING_CLASSID_INSTID); 
    newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG;
    newRule_ptr->classIdInstId = classIdInstId;
    
    /*delete old mapping rule*/
    if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->delOmciGemPortMappingRuleByMatchFlag, fail");
        ret = -1;
        goto end;
    }
    /*add new mapping rule*/
    newRule_ptr->tagctl = (OMCI_GEMPORT_MAPPING_TAGFLAG
//                          |OMCI_GEMPORT_MAPPING_PBIT   //untag rule should not set PBit control flag
                | OMCI_GEMPORT_MAPPING_GEMPORT
                            |OMCI_GEMPORT_MAPPING_CLASSID_INSTID);  
    /*set value for new port mapping*/  
    switch(tpType){
        case OMCI_DOT1P_TP_TYPE_BRIDGE_MAPPING://bridge-mapping
            break;
        case OMCI_DOT1P_TP_TYPE_PPTP_UNI://pptp ethernet uni
            newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_USERPORT;
            newRule_ptr->userPort = uniPort;
            break;
        default://not supoort others
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->newTpType=%d, fail",tpType);
            ret = -1;
            goto end;
    }
    newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG;
    newRule_ptr->classIdInstId= classIdInstId;

    /*get new pbit mapping*/
    newPbits= *(uint8 *)value;

    /*get new gem port id by new pbit */
    if(getGemPortFromDot1p(currInstId,  newPbits, &newGemPortId) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue->getGemPortFromDot1p new failure");
        ret = -1;
        goto end;
    }

    if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
    newRule_ptr->pbit = newPbits;
    newRule_ptr->gemPort= newGemPortId;
        if(addOmciGemPortMappingRule(newRule_ptr)!= 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pDefaultPbitValue-> ##[%d] addOmciGemPortMappingRule fail",__LINE__);
        ret = -1;
        goto end;
    }
    }
    
    ret = 0;

end:
    if(newRule_ptr != NULL)
        free(newRule_ptr);  
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return ret;
    }
}
int setdot1pTPTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
    int i = 0;
    uint8 uniPort = 0;
    uint8 unMarkOptFlag = 0;
    uint8 newPbits= 0;
    uint16  newGemPortId = 0;;
    omciGemPortMapping_ptr newRule_ptr = NULL;  
    omciManageEntity_t *me_p = NULL;
    uint8 *attributeValuePtr = NULL;
    uint16 length;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint16 currInstId = 0;
    uint32 classIdInstId = 0;
    uint8 newTpType = 0;
    uint8 oldTpType = 0;
//  uint8 actionFlag = 0;//0:add rule, 1:del rule
    uint8 newDscpPits[MAX_OMCI_DSCP_KINDS] = {0};
    uint16 delMatchFlag = 0;

    if(value == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->value == NULL, fail");
        ret = -1;
        goto end;
    }

    if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail");
        ret = -1;
        goto end;
    }

    /*malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->calloc newRule_ptr  fail");
        ret = -1;
        goto end;
    }

    /*get dot1p instance id*/
    currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_802_1P<<16);
    classIdInstId |= currInstId;
    
    /*get old  tp type value*/
    attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->omciGetInstAttriByName,[%d], failure",__LINE__);
        ret = -1;
        goto end;
    }
    oldTpType = *attributeValuePtr; 

    /*get new tp type value*/
    newTpType = *(uint8 *)value;
    
    if((flag != OMCI_CREATE_ACTION_SET_VAL) && (oldTpType == newTpType)){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setdot1pTPTypeValue->don't need set mapping rule in tp type!");
        ret = 0;
        goto end;
    }

    if(flag != OMCI_CREATE_ACTION_SET_VAL){
        /*delete mapping rule with classid+instance id*/
        delMatchFlag = OMCI_GEMPORT_MAPPING_CLASSID_INSTID;
        newRule_ptr->classIdInstId = classIdInstId;
        if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->delOmciGemPortMappingRuleByMatchFlag, fail");
            ret = -1;
            goto end;
    }
    }
    

    /*get 802.1p ME*/
    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_802_1P);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->omciGetMeByClassId fail");
        ret = -1;
        goto end;
    }
    /*get TP pointer attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[1].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->omciGetInstAttriByName,[%d], failure",__LINE__);
        ret = -1;
        goto end;
    }
    if(newTpType == OMCI_DOT1P_TP_TYPE_PPTP_UNI){
        uniPort = *(attributeValuePtr+1) -1;
    }

    /*get unmark frame option attribute in this instance*/
    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[10].attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->omciGetInstAttriByName,[%d], failure",__LINE__);
        ret = -1;
        goto end;
    }
    unMarkOptFlag = *attributeValuePtr;

            /*add gem port mapping rule[tagctl, tagFlag, user port, pbit, gem port, classid_instanceid] for tagged frame*/
            newRule_ptr->tagctl = (OMCI_GEMPORT_MAPPING_TAGFLAG
                                |OMCI_GEMPORT_MAPPING_PBIT
                                |OMCI_GEMPORT_MAPPING_GEMPORT
                                |OMCI_GEMPORT_MAPPING_CLASSID_INSTID);
            
    /*set value for new port mapping*/      
            newRule_ptr->classIdInstId = classIdInstId;
    switch(newTpType){
        case OMCI_DOT1P_TP_TYPE_BRIDGE_MAPPING://bridge-mapping
            break;
        case OMCI_DOT1P_TP_TYPE_PPTP_UNI://pptp ethernet uni
            newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_USERPORT;
            newRule_ptr->userPort = uniPort;
            break;
        default://not supoort others
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->newTpType=%d, fail",newTpType);
            ret = -1;
            goto end;
    }
    
    /*1.add gem port mapping rule for tagged frame*/
    newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_TAGGED_FLAG;    
            for(i = 0; i<MAX_OMCI_PBIT_NUM; i++){
                /*get new gem port id by new pbit */
                newPbits = i;
        if(getGemPortFromDot1p(currInstId,  newPbits, &newGemPortId) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->getGemPortFromDot1p new failure pbit=%d",i);
            ret = -1;
            goto end;
                }
        if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
                newRule_ptr->pbit = newPbits;
                newRule_ptr->gemPort = newGemPortId;
            if(addOmciGemPortMappingRule(newRule_ptr) != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue-> addOmciGemPortMappingRule fail pbit=%d",i);
                ret = -1;
                goto end;
            }
                }
            }

    /*2.add gem port mapping rule for untagged frame*/  
            newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG;
            newRule_ptr->tagctl &= (~OMCI_GEMPORT_MAPPING_PBIT);
            switch(unMarkOptFlag){
                case OMCI_DOT1P_UNMARK_0://[tagctl, tagFlag, user port, DSCP, pbit, gem port, classid_instance id]
                    /*get dscp pbit mapping attribute in this instance*/
                    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[11].attriName, &length);
                    if(attributeValuePtr == NULL){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->#[%d] omciGetInstAttriByName failure",__LINE__);
                        ret = -1;
                        goto end;
                    }
                    /*get Pbits by 24 bytes dscp pbits mapping*/
                    ret = convertDSCPMap2Pbit(attributeValuePtr,newDscpPits);
                    if(ret != 0){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->convertDSCPMap2Pbit new DSCP pbit failure");
                        ret = -1;
                        goto end;
                    }
                    
                    newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_DSCP;
                    for(i = 0; i<MAX_OMCI_DSCP_KINDS; i++){
                        newPbits = newDscpPits[i];
                    if(getGemPortFromDot1p(currInstId,  newPbits, &newGemPortId) != 0){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->#[%d] getGemPortFromDot1p new failure dscp=%d",__LINE__,i);
                        ret = -1;
                        goto end;                   
                            }
                    if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
                            newRule_ptr->dscp = i;
                            newRule_ptr->pbit = newPbits;
                            newRule_ptr->gemPort = newGemPortId;
                        if(addOmciGemPortMappingRule(newRule_ptr) != 0){
                            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue-> #[%d] addOmciGemPortMappingRule fail dsep=%d",__LINE__,i);
                            ret = -1;
                            goto end;
                        }
                            }               
                        }
                    break;
                case OMCI_DOT1P_UNMARK_1://[tagctl, tagFlag, user port, pbit, gem port, classid+instance id]
                    /*get default pbit attribute in this instance*/
                    attributeValuePtr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[12].attriName, &length);
                    if(attributeValuePtr == NULL){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->#[%d] omciGetInstAttriByName failure",__LINE__);
                        ret = -1;
                        goto end;
                    }
                    newPbits = *attributeValuePtr;
                    if(getGemPortFromDot1p(currInstId,  newPbits, &newGemPortId) != 0){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->#[%d] getGemPortFromDot1p new failure newPbits=%d",__LINE__,newPbits);
                        ret = -1;
                        goto end;
                    }
                    if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
                        newRule_ptr->pbit = newPbits;
                        newRule_ptr->gemPort = newGemPortId;
                        if(addOmciGemPortMappingRule(newRule_ptr) != 0){
                            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPTypeValue->#[%d] addOmciGemPortMappingRule fail pbit=%d",__LINE__, newPbits);
                            ret = -1;
                            goto end;
                        }
                    }
                    break;
                default:
                    //nothing 
                    break;
            }
    

    ret = 0;
end:
    if(newRule_ptr != NULL)
        free(newRule_ptr);  
    if(ret == 0){   
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return ret;
    }
}
/*******************************************************************************************************************************
9.3.11 vlan tagging filter data

********************************************************************************************************************************/
int omciMeInitForVlanTagFilterData(omciManageEntity_t *omciManageEntity_p){
    omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
    
    /* init some param for this ME */
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciVlanTagFilterDeleteAction;

    return 0;
}

#define TCSUPPORT_VLAN_FILTER_EX

#ifdef TCSUPPORT_VLAN_FILTER_EX
struct filterFwdOpTbl_s {
    uint8 taggedAction;
    uint8 untaggedAction;
};

const struct filterFwdOpTbl_s filterFwdOpTbl[] = //see  table 9.3.11-1 of G.988
{
    { GPON_VLAN_FILTER_ACTION_BRIDGE, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x00 Bridging (a) (no investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_DISCARD, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x01 Discarding (c) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_BRIDGE, GPON_VLAN_FILTER_ACTION_DISCARD},//0x02 Bridging (a) (no investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_VID, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x03 Action (h) (VID investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_VID, GPON_VLAN_FILTER_ACTION_DISCARD},//0x04 Action (h) (VID investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_G_VID, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x05 Action (g) (VID investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_G_VID, GPON_VLAN_FILTER_ACTION_DISCARD},//0x06 Action (g) (VID investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_PBIT, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x07 Action (h) (user priority investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_PBIT, GPON_VLAN_FILTER_ACTION_DISCARD},//0x08 Action (h) (user priority investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_G_PBIT, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x09 Action (g) (user priority investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_G_PBIT, GPON_VLAN_FILTER_ACTION_DISCARD },//0x0A Action (g) (user priority investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_TCI, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x0B Action (h) (TCI investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_TCI, GPON_VLAN_FILTER_ACTION_DISCARD},//0x0C Action (h) (TCI investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_G_TCI, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x0D Action (g) (TCI investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_G_TCI, GPON_VLAN_FILTER_ACTION_DISCARD },//0x0E Action (g) (TCI investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_VID, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x0F Action (h) (VID investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_VID, GPON_VLAN_FILTER_ACTION_DISCARD },//0x10 Action (h) (VID investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_PBIT, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x11 Action (h) (user priority investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_PBIT, GPON_VLAN_FILTER_ACTION_DISCARD},//0x12 Action (h) (user priority investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_TCI, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x13 Action (h) (TCI investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_TCI, GPON_VLAN_FILTER_ACTION_DISCARD},//0x14 Action (h) (TCI investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_BRIDGE, GPON_VLAN_FILTER_ACTION_DISCARD},//0x15 Bridging(a) (no investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_J_VID, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x16 Action (j) (VID investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_J_VID, GPON_VLAN_FILTER_ACTION_DISCARD},//0x17 Action (j) (VID investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_J_PBIT, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x18 Action (j) (user priority investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_J_PBIT, GPON_VLAN_FILTER_ACTION_DISCARD},//0x19 Action (j) (user priority investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_J_TCI, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x1A Action (j) (TCI investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_J_TCI, GPON_VLAN_FILTER_ACTION_DISCARD},//0x1B Action (j) (TCI investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_VID, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x1C Action (h) (VID investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_VID, GPON_VLAN_FILTER_ACTION_DISCARD},//0x1D Action (h) (VID investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_PBIT, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x1E Action (h) (user priority investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_PBIT, GPON_VLAN_FILTER_ACTION_DISCARD},//0x1F Action (h) (user priority investigation) Discarding (c) 
    { GPON_VLAN_FILTER_ACTION_H_TCI, GPON_VLAN_FILTER_ACTION_BRIDGE},//0x20 Action (h) (TCI investigation) Bridging (a) 
    { GPON_VLAN_FILTER_ACTION_H_TCI, GPON_VLAN_FILTER_ACTION_DISCARD},//0x21 Action (h) (TCI investigation) Discarding (c) 
};
#endif


int checkMappingRuleUntagAction(int16 instanceId)
{
    omciManageEntity_t *me_p = NULL;
    omciInst_t *omciInst_p   = NULL ;
    omciMeInst_t *inst_p     = NULL;
    uint8 *attributeValuePtr = NULL;
    uint8 tempTpType         = 0;
    uint16 tempTpPointer     = 0;
    uint8  findflag          = 0;
    uint8  forwardaction     = 0;
    uint16 currInstId        = 0;
    uint8* val               = NULL;
    int ret                  = 0;
    
    uint16 length;
   
    /*mac con dataME*/
    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
    if(me_p == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbitValue->omciGetMeByClassId fail");
        ret = -1;
        goto end;
    }

    /*802.1pinstanceId mac con datainstanecId*/
    omciInst_p = &(me_p->omciInst);
    if (NULL == omciInst_p)
    {
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> omciInst_p == NULL fail,[%d]", __LINE__);
        ret = -1;
        goto end;
    }

    inst_p = omciInst_p->omciMeInstList;
    while (NULL != inst_p)
    {
        attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[3].attriName, &length);//tp type attribute
        if(attributeValuePtr == NULL){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> omciGetInstAttriByName,fail");
            ret = -1;
            goto end;
        }
        tempTpType= *attributeValuePtr;
        
        /*get 9.3.4 tp pointer attribute value*/
        attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[4].attriName, &length);//tp pointer attribute
        if(attributeValuePtr == NULL){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> omciGetInstAttriByName, tp pointer , fail");
            ret = -1;
            goto end;
        }
        
        tempTpPointer= get16(attributeValuePtr);

        if ((OMCI_MAC_BRIDGE_PORT_TP_TYPE_3 == tempTpType) && (tempTpPointer == instanceId))
        {
            currInstId = get16(inst_p->attributeVlaue_ptr); 

            //tcdbg_printf("\n find: current instanceId is :%d",currInstId);
            findflag = 1;
            break;
        }

        
        inst_p = inst_p->next;
    }

    if (1 == findflag)
    {
        /*malloc for saving attribute value*/
        val =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
        if(val == NULL){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortIdFromGemInterTP-> val calloc error ");
            ret = -1;
            goto end;
        }
        ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA,currInstId,2,val,1);
        if (0 != ret)
        {
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setdot1pTPPbit0Value-> getAttribute, tp pointer ,fail");
            //tcdbg_printf("\n checkMappingRuleUntagAction call getAttributeValueByInstId return error\n");
            
            ret = 0;
            goto end;
        }

        forwardaction = (*((uint8*)(val)));
        //tcdbg_printf("\n forwardaction = %d",forwardaction);
        if (GPON_VLAN_FILTER_ACTION_BRIDGE == filterFwdOpTbl[forwardaction].untaggedAction)
        {
            ret = 1;
            goto end;
        }
        else
        {
            ret = 2;
            goto end;
        }
       
    }
    
    end:
    if(val != NULL)
        free(val);
    return ret;
}

int omciVlanTagFilterDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
    uint32 classIdInstId = 0;
    uint16 currInstId = 0;
    int ret = 0;
    uint16 delMatchFlag = 0;
    omciGemPortMapping_ptr newRule_ptr = NULL;
    uint16 portId = 0;
    uint8 port = 0;
    uint8 ifcType = 0;
    int result = 0;


    classIdInstId = omciPayLoad->meId;
    currInstId = get16((uint8*)&(omciPayLoad->meId)+2);

    /*get port and interface type */
    if(getPortFromMacBridgeByInstId(&port, &ifcType, currInstId) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagFilterDeleteAction----->getPortFromMacBridgeByInstId, fail");
        ret = -1;
        goto end;
    }
    portId = port;
    
    /*del all gem port mapping rule about this vlan filter  instance id*/
    delMatchFlag = OMCI_GEMPORT_MAPPING_CLASSID_INSTID;
    
    /*malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagFilterDeleteAction->newRule_ptr->calloc newRule_ptr  fail");
        ret = -1;
        goto end;
    }   
    newRule_ptr->classIdInstId = classIdInstId;
    
    if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciVlanTagFilterDeleteAction->delOmciGemPortMappingRuleByMatchFlag, fail");
        ret = 0;
        goto end;       
    }
    
    /*del all vlan filter rule by calling vlan filter API*/
    result = delGponVlanFilterRule(portId, ifcType);
    if(result == GPON_VLAN_FILTER_FAIL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagFilterDeleteAction----->delGponVlanFilterRule, result=%x, fail", result);        
    }

    /*update hwnat rule, make restart learn rule*/
    if(updateOmci63365HwNatRule() != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagFilterDeleteAction----->updateOmci63365HwNatRule,  fail");
    }
    ret = 0;
end:
    if(newRule_ptr != NULL){
        free(newRule_ptr);
    }

    if(ret == 0){
        return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);    
    }else{
        return ret;
    }
}

/*******************************************************************************************
**function name
    addGemPortMappingRuleByVlanFilter
**description:
    add gem port mapping rule according the vlan filter ME valute.
 **retrun :
    -1: failure
    0:  success
**parameter:
    currInstId :    vlan filter ME instance id
    vlanList:       tci list
    maxValidNum:max valid entry
    forwardOperation: forward operation value in vlan filter ME.
********************************************************************************************/
int addGemPortMappingRuleByVlanFilter(IN uint16 currInstId,  IN uint8 * vlanList, IN uint8 maxValidNum, IN uint8 forwardOperation){
    int ret = -1;
    int i = 0;
    int j = 0;
    uint8 tempStep = 0;
    uint8 * oldValue = NULL;
    uint16  newGemPortId = 0;;
    omciGemPortMapping_ptr newRule_ptr = NULL;
    uint16 delMatchFlag = 0;
    int ifaceType = 0; //0:UNI, 1:ANI
    uint8 portId = 0;
    uint8 tpType = 0;
    uint16 tempMeInstId = 0;
    uint32 classIdInstId = 0;
    uint8 ifcType = 0;

#ifdef TCSUPPORT_VLAN_FILTER_EX 
    uint8 taggedAction = 0xff;
    uint8 untaggedAction = 0xff;
#endif

    if(vlanList == NULL){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter->vlanList == NULL fail");
        goto end;
    }

    /*malloc  for new port mapping*/        
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter->calloc newRule_ptr  fail");
        ret = -1;
        goto end;
    }
    memset(newRule_ptr,0,sizeof(omciGemPortMapping_t));

    /*malloc for saving attribute value*/
    oldValue =  (uint8 *)calloc(1 , MAX_VLAN_FILTER_LIST_LEN);
    if(oldValue == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> oldValue calloc error ");
        ret = -1;
        goto end;
    }

    /*get ME id(class id +instance id)*/
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA<<16);
    classIdInstId |= currInstId;

    /*get mac bridge port interface type*/
    ifaceType = getMacBridgePortInstType(currInstId, &ifcType);
    if(ifaceType == -1){
        ret = -1;
        goto end;
    }
    /*get  mac bridge port  ANI/UNI port id*/
    if(ifaceType == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//lan, UNI port
        ret = getMacBridgeLanPortId(currInstId, &portId);
        if(ret != 0){
            ret = -1;
            goto end;
        }
    }else if(ifaceType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//wan , ANI port
        ret = getMacBridgeWanPortId(currInstId, &portId);
        if(ret != 0){
            ret = -1;
            goto end;
        }
    }else{
        ret = -1;
        goto end;
    }
    
    if(ifaceType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//only ANI port need add gem port rule
        /*del gem port mapping according to vlan filter ME id*/
        delMatchFlag = OMCI_GEMPORT_MAPPING_CLASSID_INSTID;
        newRule_ptr->classIdInstId = classIdInstId;
        ret = delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr);

        /*set default value for ANI gem port mappint*/
        newRule_ptr->tagctl = (OMCI_GEMPORT_MAPPING_TAGFLAG
                            |OMCI_GEMPORT_MAPPING_ANI_PORT
                            |OMCI_GEMPORT_MAPPING_GEMPORT
                            |OMCI_GEMPORT_MAPPING_CLASSID_INSTID);
        
        newRule_ptr->tagFlag = OMCI_GEMPORT_MAPPING_TAGGED_FLAG;
        newRule_ptr->classIdInstId = classIdInstId;
        newRule_ptr->aniPort = (uint16)portId;

        /*get mac bridge port ME TP Type attribute*/
        ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, currInstId, 3, oldValue, 1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> getAttributeValueByInstId 1 fail ");
            ret = -1;
            goto end;           
        }
        tpType = *oldValue;
        switch(tpType){
            case OMCI_MAC_BRIDGE_PORT_TP_TYPE_3: //support pointer to dot1p
                break;
            case OMCI_MAC_BRIDGE_PORT_TP_TYPE_5: //support pointer to TP
                break;
            case OMCI_MAC_BRIDGE_PORT_TP_TYPE_6: //support pointer to multicast TP
                break;
            default:
                ret = 0; //others types don't need add gem port rule.
                goto end;
                break;
        }

        /*get mac bridge port ME TP pointer attribute*/
        ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, currInstId, 4, oldValue, 2);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> getAttributeValueByInstId 2 fail ");
            ret = -1;
            goto end;
        }
        tempMeInstId = get16(oldValue);
#ifdef TCSUPPORT_VLAN_FILTER_EX
        
            if(forwardOperation < (sizeof(filterFwdOpTbl)/sizeof(filterFwdOpTbl[0]))){
                int findTagRule = 0;
                taggedAction = filterFwdOpTbl[forwardOperation].taggedAction;
                untaggedAction = filterFwdOpTbl[forwardOperation].untaggedAction;

                if((GPON_VLAN_FILTER_ACTION_H_VID == taggedAction) || (GPON_VLAN_FILTER_ACTION_H_TCI == taggedAction)
                    || (GPON_VLAN_FILTER_ACTION_J_VID == taggedAction) || (GPON_VLAN_FILTER_ACTION_J_TCI == taggedAction)){
                    newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_VID;
                    findTagRule = 1;
                }
                if((GPON_VLAN_FILTER_ACTION_H_PBIT== taggedAction) || (GPON_VLAN_FILTER_ACTION_H_TCI == taggedAction)
                    || (GPON_VLAN_FILTER_ACTION_J_PBIT== taggedAction) || (GPON_VLAN_FILTER_ACTION_J_TCI == taggedAction) ){
                    newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_PBIT;
                    findTagRule = 1;
                }
#if 0
                if(GPON_VLAN_FILTER_ACTION_BRIDGE == taggedAction)
                    findTagRule = 1;
#endif

                if(findTagRule == 0){
                    ret = 0;  //others don't support in gem port mapping rule
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "not support in gemport map rule  ");
                    goto end; 
                }
                        
            }
            else{
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> forwardOperation= 0x%02x, fail",forwardOperation);
                ret = 0;
                goto end;
        
            }
        
#else

        /*update tagctl according to the forward operation */
        switch(forwardOperation){
            case VLAN_FILTER_OPERATION_TYPE4:
            case VLAN_FILTER_OPERATION_TYPE10:
            case VLAN_FILTER_OPERATION_TYPE16:
            case VLAN_FILTER_OPERATION_TYPE3:
            case VLAN_FILTER_OPERATION_TYPE0F:
                newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_VID;
                break;
            case VLAN_FILTER_OPERATION_TYPE8:
            case VLAN_FILTER_OPERATION_TYPE12:
            case VLAN_FILTER_OPERATION_TYPE7:
            case VLAN_FILTER_OPERATION_TYPE11:
                newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_PBIT;
                break;
            default: //others don't support in gem port mapping rule
                ret = 0;
                goto end;
                break;
        }               
#endif

        /*add gem port mapping rule*/
        switch(tpType){     
            case OMCI_MAC_BRIDGE_PORT_TP_TYPE_3: //pointer to 802.1p ME
                /*update tagctl*/
#ifndef TCSUPPORT_VLAN_FILTER_EX    
                newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_PBIT;
#endif
                for(i = 0; i<maxValidNum; i++){
                    tempStep = (uint8)(i<<1);//i*2                  
                    /*add gem port mapping rule according to the forward operation */                           
#ifdef TCSUPPORT_VLAN_FILTER_EX
                    if(OMCI_GEMPORT_MAPPING_PBIT & newRule_ptr->tagctl){
                        newRule_ptr->pbit= (vlanList[tempStep] >> 5); //get pbit
                        
                        if(getGemPortFromDot1p(tempMeInstId,  newRule_ptr->pbit , &newGemPortId) != 0){
                            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> getGemPortFromDot1p 5 fail");
                            ret = -1;
                            goto end;
                        }
                        if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
                        newRule_ptr->gemPort = newGemPortId;
                        if(OMCI_GEMPORT_MAPPING_VID & newRule_ptr->tagctl){
                            newRule_ptr->vid = ((vlanList[tempStep] & 0x0f) << 8); //get vid                            
                            newRule_ptr->vid |= vlanList[tempStep+1];
                        }

                        ret = addOmciGemPortMappingRule(newRule_ptr);
                        if(ret != 0){
                            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> addOmciGemPortMappingRule 6  fail i = %d,  j = %d", i, j);
                            ret = -1;
                            goto end;
                        }
                        }

                    }
                    else if(OMCI_GEMPORT_MAPPING_VID & newRule_ptr->tagctl){
                        newRule_ptr->vid = ((vlanList[tempStep] & 0x0f) << 8); //get vid                            
                        newRule_ptr->vid |= vlanList[tempStep+1];
#ifdef TCSUPPORT_VLAN_FILTER_EX
                        newRule_ptr->tagctl |= OMCI_GEMPORT_MAPPING_PBIT;
#endif
                        
                        for(j = 0; j<MAX_OMCI_PBIT_NUM; j++){
                            newRule_ptr->pbit = j;
                            ret = getGemPortFromDot1p(tempMeInstId,  newRule_ptr->pbit , &newGemPortId);
                            if(ret != 0){
                                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> getGemPortFromDot1p 3 fail");
                                ret = -1;
                                goto end;
                            }
                            if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
                            newRule_ptr->gemPort = newGemPortId;
                            ret = addOmciGemPortMappingRule(newRule_ptr);
                            if(ret != 0){
                                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> addOmciGemPortMappingRule 4  fail i = %d,  j = %d", i, j);
                                ret = -1;
                                goto end;
                            }
                        }
                        }
#ifdef TCSUPPORT_VLAN_FILTER_EX
                        newRule_ptr->tagctl &= (0xFFFF - OMCI_GEMPORT_MAPPING_PBIT);
#endif
                    }
                        
#else                   
                    switch(forwardOperation){
                        case VLAN_FILTER_OPERATION_TYPE4:
                        case VLAN_FILTER_OPERATION_TYPE10:
                        case VLAN_FILTER_OPERATION_TYPE16:
                        case VLAN_FILTER_OPERATION_TYPE3:
                        case VLAN_FILTER_OPERATION_TYPE0F:
                            newRule_ptr->vid = ((vlanList[tempStep] & 0x0f) << 8); //get vid                            
                            newRule_ptr->vid |= vlanList[tempStep+1];
                            for(j = 0; j<MAX_OMCI_PBIT_NUM; j++){
                                newRule_ptr->pbit = j;
                                ret = getGemPortFromDot1p(tempMeInstId,  newRule_ptr->pbit , &newGemPortId);
                                if(ret != 0){
                                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> getGemPortFromDot1p 3 fail");
                                    ret = -1;
                                    goto end;
                                }
                                if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
                                newRule_ptr->gemPort = newGemPortId;
                                ret = addOmciGemPortMappingRule(newRule_ptr);
                                if(ret != 0){
                                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> addOmciGemPortMappingRule 4  fail i = %d,  j = %d", i, j);
                                    ret = -1;
                                    goto end;
                                }
                            }
                            }
                            break;
                        case VLAN_FILTER_OPERATION_TYPE8:
                        case VLAN_FILTER_OPERATION_TYPE12:
                        case VLAN_FILTER_OPERATION_TYPE7:
                        case VLAN_FILTER_OPERATION_TYPE11:                          
                            newRule_ptr->pbit= (vlanList[tempStep] >> 5); //get pbit
                            if(getGemPortFromDot1p(tempMeInstId,  newRule_ptr->pbit , &newGemPortId) != 0){
                                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> getGemPortFromDot1p 5 fail");
                                ret = -1;
                                goto end;
                            }
                            if(newGemPortId != OMCI_DOT1P_GEM_PORT_DISCARD){
                            newRule_ptr->gemPort = newGemPortId;
                            ret = addOmciGemPortMappingRule(newRule_ptr);
                            if(ret != 0){
                                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> addOmciGemPortMappingRule 6  fail i = %d,  j = %d", i, j);
                                ret = -1;
                                goto end;
                            }
                            }
                            break;
                        default:
                            break;
                    }
#endif                  
                }
                break;
            case OMCI_MAC_BRIDGE_PORT_TP_TYPE_5: //pointer TP ME
            case OMCI_MAC_BRIDGE_PORT_TP_TYPE_6://pointer TP ME
                if(tpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_5){
                    ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_GEM_INTERWORK_TP, tempMeInstId, &newGemPortId);                  
                }else if(tpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_6){
                    ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_MULTICAST_GEM_INTERWORK_TP, tempMeInstId, &newGemPortId);
                }else{
                    //nothing
                }
                if(ret != 0){
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> getGemPortIdFromGemInterTP 7 fail  ");
                    ret = -1;
                    goto end;
                }
                newRule_ptr->gemPort = newGemPortId;
                
                if(GPON_VLAN_FILTER_ACTION_BRIDGE == taggedAction){
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\naddGemPortMappingRuleByVlanFilter-> tag bridge no op \n");
                }
                if(GPON_VLAN_FILTER_ACTION_BRIDGE == untaggedAction){
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\naddGemPortMappingRuleByVlanFilter-> untag bridge no op \n");
                }
                
                for(i = 0; i<maxValidNum; i++){
                    tempStep = (uint8)(i<<1);//i*2
                    /*add gem port mapping rule according to the forward operation */
#ifdef TCSUPPORT_VLAN_FILTER_EX 
                    if(OMCI_GEMPORT_MAPPING_VID | newRule_ptr->tagctl){
                        newRule_ptr->vid = ((vlanList[tempStep] & 0x0f) << 8); //get vid                            
                        newRule_ptr->vid |= vlanList[tempStep+1];
                    }
                    if(OMCI_GEMPORT_MAPPING_PBIT | newRule_ptr->tagctl){
                        newRule_ptr->pbit= (vlanList[tempStep] >> 5); //get pbit
                    }
#else                   
                    switch(forwardOperation){
                        case VLAN_FILTER_OPERATION_TYPE4:
                        case VLAN_FILTER_OPERATION_TYPE10:
                        case VLAN_FILTER_OPERATION_TYPE16:
                        case VLAN_FILTER_OPERATION_TYPE3:
                        case VLAN_FILTER_OPERATION_TYPE0F:                          
                            newRule_ptr->vid = ((vlanList[tempStep] & 0x0f) << 8); //get vid                            
                            newRule_ptr->vid |= vlanList[tempStep+1];
                            break;
                        case VLAN_FILTER_OPERATION_TYPE8:
                        case VLAN_FILTER_OPERATION_TYPE12:
                        case VLAN_FILTER_OPERATION_TYPE7:
                        case VLAN_FILTER_OPERATION_TYPE11:                          
                            newRule_ptr->pbit= (vlanList[tempStep] >> 5); //get pbit
                            break;
                        default:
                            break;
                    }
#endif                  
                    ret = addOmciGemPortMappingRule(newRule_ptr);
                    if(ret != 0){
                        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortMappingRuleByVlanFilter-> addOmciGemPortMappingRule 8 fail i = %d", i);
                        ret = -1;
                        goto end;
                    }
                }                       
                break;
            default:
                break;
        }
        
    }else{
        ret = 0;
        goto end;
    }

    ret = 0;

end:
    if(newRule_ptr != NULL)
        free(newRule_ptr);
    if(oldValue != NULL)
        free(oldValue);
    if(ret != 0){
        ret = -1;
    }
    
    return ret;
                
}

#ifdef TCSUPPORT_CT_PON
void displayOmciCurrGemPortMappingRule1(omciGemPortMapping_ptr mapping_ptr){

    tcdbg_printf("\r\n index    tagctl  tagFlag userPort    ani port    vid");
    tcdbg_printf("\r\n dscp pbit    gemPort classIdInstId");    
    if (mapping_ptr != NULL){
        tcdbg_printf( "\r\n 0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x",
            mapping_ptr->index, mapping_ptr->tagctl,mapping_ptr->tagFlag, mapping_ptr->userPort, mapping_ptr->aniPort, mapping_ptr->vid, mapping_ptr->dscp,
            mapping_ptr->pbit, mapping_ptr->gemPort, mapping_ptr->classIdInstId);

    }
    return;
}
#endif

extern omciGemPortMapping_ptr gemPortMapping_ptr;

int delGemPortMappingUntagRuleByUntagAction(int currInstanceId)
{
    int ret = 0;
    int iLoop = 0;
    uint16 gemPortList[MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM] = {0};
    int validGemPortNum = 0;    
    omciGemPortMapping_ptr MapPtr = NULL;
    omciGemPortMapping_ptr tmpMapPtr = NULL;

    ret = getGemPortByMACBridgePortInst(currInstanceId, gemPortList, &validGemPortNum);
    if ((0 != ret) || (0 == validGemPortNum))
    {
        tcdbg_printf("\n call getGemPortByMACBridgePortInst return error");
        return -1;
    }

    MapPtr = gemPortMapping_ptr;
    if (NULL == MapPtr)
    {
        return -1;
    }

    //tcdbg_printf("\n validGemPortNum = %d",validGemPortNum);
    for (iLoop = 0; iLoop < validGemPortNum; iLoop++)
    {
        while (MapPtr)
        {
            tmpMapPtr = MapPtr->next;
            if ((OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG == MapPtr->tagFlag)  && 
                (gemPortList[iLoop] == MapPtr->gemPort))
            {
                 //displayOmciCurrGemPortMappingRule1(MapPtr);
                //tcdbg_printf("\n enter delOmciGemPortMappingRuleByMatchFlag");
                ret = delOmciGemPortMappingRuleByMatchFlag(MapPtr->tagctl,MapPtr);
                if (0 != ret)
                {
                    tcdbg_printf("\n error delOmciGemPortMappingRuleByMatchFlag gemPort:%d",gemPortList[iLoop]);

                    return -1;
                }
            }

            MapPtr = tmpMapPtr;
        }
    }

    return 0;
}
/*******************************************************************************************
**function name
    setvlanFilterAttributeValue
**description:
    set gem port mapping rule and vlan filter ruel according to type parameter.
 **retrun :
    -1: failure
    0:  success
**parameter:
    value : set value in attribute packets
    meInstantPtr:       Me instance pointer
    omciAttribute:      ME attribute pointer
    type:           operation type
                modify vlan filter list attribute, modify forward operation attribute, modify numer of entry attribute
    flag                create or set
********************************************************************************************/
int setvlanFilterAttributeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 type, uint8 flag){
    int ret = -1;
    uint8 untagaction = 0;
    uint8 * val = NULL;
    int i = 0;
//  uint8 tempStep = 0;
    uint8 * oldValue = NULL;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint16 currInstId = 0;
    uint8 maxValidNum = 0;
    uint8 vlanList[MAX_VLAN_FILTER_LIST_LEN] = {0};
    uint8 * temp_ptr = NULL;
    uint8 forwardOperation = 0;
    uint8 ifaceType = 0; //0:UNI, 1:ANI
    uint8 port = 0;
    uint16 portId = 0;
    int result = 0;
    uint16 delMatchFlag = 0;
    omciGemPortMapping_ptr newRule_ptr = NULL;
    gponVlanFilterIoctl_ptr vlanFilterRule_ptr = NULL;
    uint32 classIdInstId = 0;
#ifdef TCSUPPORT_PON_ROSTELECOM
    char tempStep[32] = {0};
    uint16 max_vlan = 0;
#endif
    uint8 skip_filter_flag = 0;

    if(value == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> value == NULL, fail");
        ret = -1;
        goto end;
    }
    val = (uint8 *)value;

    currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
    if (VLAN_FILTER_ME_FORWARD_OPERATION_ATT == type)
    {
        forwardOperation = *val;
        untagaction      = filterFwdOpTbl[forwardOperation].untaggedAction;
        if (1 == untagaction)
        {
            ret = delGemPortMappingUntagRuleByUntagAction(currInstId);
            if (0 != ret)
            {
                tcdbg_printf("\r\n call delGemPortMappingRule fail");               
            }
        }
    }
	
    switch(type){
        case VLAN_FILTER_ME_VLAN_FILTER_LIST_ATT://modify vlan filter list attribute
        case VLAN_FILTER_ME_FORWARD_OPERATION_ATT: //modify forward operation attribute
            if(flag == OMCI_CREATE_ACTION_SET_VAL){//CREATE action
                ret = 0;
                goto end;
            }
            break;
        case VLAN_FILTER_ME_NUM_OF_ENTRY_ATT: //modify numer of entry attribute
            break;
        default:
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> type=0x%x, fail", type);
            ret = -1;
            goto end;
    }
    
    /*init for vlan filter and gem port mapping ---------------------------------------------------start*/
    /*1. malloc some space */
    /*1.1 malloc and set value for new port mapping*/       
    newRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
    if(newRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagFilterDeleteAction->newRule_ptr->calloc newRule_ptr  fail");
        ret = -1;
        goto end;
    }
    /*1.2 malloc and set value for vlan filter rule*/       
    vlanFilterRule_ptr = (gponVlanFilterIoctl_ptr)calloc(1 , sizeof(gponVlanFilterIoctl_t));
    if(vlanFilterRule_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagFilterDeleteAction->vlanFilterRule_ptr->calloc vlanFilterRule_ptr  fail");
        ret = -1;
        goto end;
    }   
    
    /*1.3 malloc for saving attribute value*/
    oldValue =  (uint8 *)calloc(1 , MAX_VLAN_FILTER_LIST_LEN);
    if(oldValue == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> oldValue calloc error ");
        ret = -1;
        goto end;
    }

    /*2. get vlan filter  instance id*/

    /*get ME id(class id +instance id)*/
    classIdInstId = (OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA<<16);
    classIdInstId |= currInstId;

    /*3. get port and interface type */
    if(getPortFromMacBridgeByInstId(&port, &ifaceType, currInstId) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue----->getPortFromMacBridgeByInstId, fail");
        ret = -1;
        goto end;
    }
    portId = port;
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setvlanFilterForwardOPValue----->uint8 port=0x%02x, uint16 portId=0x%02x", port, portId);
    
    /*4.get Vlan filter ME vlan list attribute*/
    if(type == VLAN_FILTER_ME_VLAN_FILTER_LIST_ATT){
        memcpy(vlanList, val, MAX_VLAN_FILTER_LIST_LEN);
    }else{
        if(getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA, currInstId, 1, oldValue, MAX_VLAN_FILTER_LIST_LEN) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> getAttributeValueByInstId 1 fail ");
            ret = -1;
            goto end;
        }
        memcpy(vlanList, oldValue, MAX_VLAN_FILTER_LIST_LEN);
    }

    /*5. get Vlan filter ME forward operation attribute*/
    if(type == VLAN_FILTER_ME_FORWARD_OPERATION_ATT){
        forwardOperation = *val;
    }else{
        if(getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA, currInstId, 2, oldValue, 1) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> getAttributeValueByInstId  2 fail ");
            ret = -1;
            goto end;
        }
        forwardOperation = *oldValue;
    }

    /*6. get Vlan filter ME number of entry attribute*/
    if(type == VLAN_FILTER_ME_NUM_OF_ENTRY_ATT){
        maxValidNum = *val;
    }else{
        if(getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA, currInstId, 3, oldValue, 1) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> getAttributeValueByInstId 3 fail ");
            ret = -1;
            goto end;
        }
        maxValidNum = *oldValue;
    }

    if(flag != OMCI_CREATE_ACTION_SET_VAL){//CREATE action
        /*7. delete old gem port mapping rule and old vlan filter rule for this vlan filter instance*/
        /*7.1 del gem port mapping rule*/
        delMatchFlag = OMCI_GEMPORT_MAPPING_CLASSID_INSTID;     
        newRule_ptr->classIdInstId = classIdInstId;
        if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, newRule_ptr) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue->delOmciGemPortMappingRuleByMatchFlag, fail");
            ret = -1;
            goto end;
        }

        /*7.2 del all vlan filter rule in this port by calling vlan filter API*/
        result = delGponVlanFilterRule(portId, ifaceType);
        if(result == GPON_VLAN_FILTER_FAIL){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue----->delGponVlanFilterRule, result=%x, fail", result);  
            ret = -1;
            goto end;
        }
    }
    /*8. don't need handle when maxValidNum is 0*/
    if(maxValidNum == 0){
        ret = 0;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setvlanFilterForwardOPValue-> maxValidNum = 0 ");
        goto end;
    }

    /*init for vlan filter and gem port mapping ---------------------------------------------------end*/    

    /*deal with vlan filter and gem port mapping ---------------------------------------------------start*/ 
    /*1. set set vlan filter structure*/
    /*1.1 set common vlan filter value for vlan structure*/
    vlanFilterRule_ptr->port = portId;
    vlanFilterRule_ptr->portType = ifaceType;
    vlanFilterRule_ptr->type = 0x03;//bit0::set for untagged frame, bit1:set for tagged frame
    vlanFilterRule_ptr->maxValidVlanListNum = maxValidNum;
    vlanFilterRule_ptr->cleanFlag = 0;
    temp_ptr = vlanList;
    for(i = 0; i<MAX_GPON_VLAN_FILTER_LIST; i++){
        temp_ptr = vlanList +(i<<1);
        vlanFilterRule_ptr->vlanList[i] = get16(temp_ptr);
    }

    /*1.2 set others vlan filter value for vlan structure*/
#ifdef TCSUPPORT_VLAN_FILTER_EX 

    if(forwardOperation < (sizeof(filterFwdOpTbl)/sizeof(filterFwdOpTbl[0]))){
        vlanFilterRule_ptr->untaggedAction = filterFwdOpTbl[forwardOperation].untaggedAction;
        vlanFilterRule_ptr->taggedAction = filterFwdOpTbl[forwardOperation].taggedAction;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"-> fwdOp=%x, untagOp=%x tagOp=%x",forwardOperation,vlanFilterRule_ptr->untaggedAction,vlanFilterRule_ptr->taggedAction);
    }
    else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> forwardOperation= 0x%02x, fail",forwardOperation);
        ret = -1;
        goto end;

    }

#else
    switch(forwardOperation){
    case VLAN_FILTER_OPERATION_TYPE4:
    case VLAN_FILTER_OPERATION_TYPE10:
        vlanFilterRule_ptr->untaggedAction = GPON_VLAN_FILTER_ACTION_DISCARD;
        vlanFilterRule_ptr->taggedAction = GPON_VLAN_FILTER_ACTION_H_VID;
        break;
    case VLAN_FILTER_OPERATION_TYPE3:
    case VLAN_FILTER_OPERATION_TYPE0F:
        vlanFilterRule_ptr->untaggedAction = GPON_VLAN_FILTER_ACTION_BRIDGE;
        vlanFilterRule_ptr->taggedAction = GPON_VLAN_FILTER_ACTION_H_VID;       
        break;
    case VLAN_FILTER_OPERATION_TYPE8:
    case VLAN_FILTER_OPERATION_TYPE12:
        vlanFilterRule_ptr->untaggedAction = GPON_VLAN_FILTER_ACTION_DISCARD;
        vlanFilterRule_ptr->taggedAction = GPON_VLAN_FILTER_ACTION_H_PBIT;
        break;
    case VLAN_FILTER_OPERATION_TYPE7:
    case VLAN_FILTER_OPERATION_TYPE11:
        vlanFilterRule_ptr->untaggedAction = GPON_VLAN_FILTER_ACTION_BRIDGE;
        vlanFilterRule_ptr->taggedAction = GPON_VLAN_FILTER_ACTION_H_PBIT;
        break;
    case VLAN_FILTER_OPERATION_TYPE16:
        vlanFilterRule_ptr->untaggedAction = GPON_VLAN_FILTER_ACTION_BRIDGE;
        vlanFilterRule_ptr->taggedAction = GPON_VLAN_FILTER_ACTION_J_VID;
        break;
    default:
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> forwardOperation= 0x%02x, fail",forwardOperation);
        ret = -1;
        goto end;
    }
#endif      
    /*2 calling vlan filter API*/
    /*2.1 add rule for gem port mapping rule*/
    result = addGemPortMappingRuleByVlanFilter(currInstId, vlanList, maxValidNum, forwardOperation);
    if(result != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> addGemPortMappingRuleByVlanFilter fail");
        ret = -1;           
        goto end;
    }

#if defined(TCSUPPORT_HGU_OMCI_MIB_UPLOAD_PPTP_ETH_UNI)
        if(0 != getPortFromMacBridgeSkipUNIFilterRuleByInstId(&skip_filter_flag, currInstId))
        {
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue----->getPortFromMacBridgeSkipUNIFilterRuleByInstId, fail");
            ret = -1;
            goto end;
        }
#endif
    if(0 == skip_filter_flag)
    {
        /*2.2 :call vlan filter API, add vlan fitler rule*/
        result = addGponVlanFilterRule(vlanFilterRule_ptr);
        if(result == GPON_VLAN_FILTER_FAIL){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-->addGponVlanFilterRule, fail");
            ret = -1;
            goto end;
        }
    }
    
    /*3. update hwnat rule, make restart learn rule*/
    if(updateOmci63365HwNatRule() != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue----->updateOmci63365HwNatRule,  fail");
    }   
    /*4.update UNI gemport mapping rule by vlan filter */
    if(updateGemMappingRuleByMbPortUNI(OMCI_UPDATE_UNI_MAPPING_BY_VLAN_FILTER_ME,currInstId) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue->updateGemMappingRuleByMbPortUNI, fail");
    }
    /*deal with vlan filter and gem port mapping ---------------------------------------------------end*/   
    
    ret = 0;
end:
    if(oldValue != NULL){
        free(oldValue);
    }
    if(newRule_ptr != NULL){
        free(newRule_ptr);
    }   
    if(vlanFilterRule_ptr != NULL){
        free(vlanFilterRule_ptr);
    }
    
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return ret;
    }

}
int setvlanFilterVlanFilterListValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    
    if(value == NULL){
        goto failure;
    }
    return setvlanFilterAttributeValue(value, meInstantPtr, omciAttribute, VLAN_FILTER_ME_VLAN_FILTER_LIST_ATT, flag);
    
failure:
    return -1;
}

int setvlanFilterForwardOPValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 val = 0;
    
    if(value == NULL){
        goto failure;
    }
    
    val = *(uint8 *)value;
    if((val == VLAN_FILTER_OPERATION_TYPE0) && (flag == OMCI_CREATE_ACTION_SET_VAL)){
        return 0;
    }
#ifdef TCSUPPORT_VLAN_FILTER_EX 
    if(val < (sizeof(filterFwdOpTbl)/sizeof(filterFwdOpTbl[0]))){
        return setvlanFilterAttributeValue(value, meInstantPtr, omciAttribute, VLAN_FILTER_ME_FORWARD_OPERATION_ATT, flag);
    }
    else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> value error val=0x%02x",val);
        goto failure;       
    }
#else
    /*get support valid value, only support 0x10, 0x12,0x16*/
    switch(val){
        case VLAN_FILTER_OPERATION_TYPE4:
        case VLAN_FILTER_OPERATION_TYPE10:
        case VLAN_FILTER_OPERATION_TYPE3:
        case VLAN_FILTER_OPERATION_TYPE0F:          
        case VLAN_FILTER_OPERATION_TYPE8:
        case VLAN_FILTER_OPERATION_TYPE12://init for vlan filter and gem port mapping
        case VLAN_FILTER_OPERATION_TYPE7:
        case VLAN_FILTER_OPERATION_TYPE11:      
        case VLAN_FILTER_OPERATION_TYPE16:
            return setvlanFilterAttributeValue(value, meInstantPtr, omciAttribute, VLAN_FILTER_ME_FORWARD_OPERATION_ATT, flag);
            break;
        default:
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setvlanFilterForwardOPValue-> value error val=0x%02x",val);
            goto failure;
    }
#endif  
failure:
    return -1;

}
int setvlanFilterValidNumberEntryValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 val = 0;
    
    if(value == NULL){
        goto failure;
    }
    
    val = *(uint8 *)value;
    if(val > MAX_VLAN_FILTER_TCI_LIST_NUM){
        goto failure;
    }
    return setvlanFilterAttributeValue(value, meInstantPtr, omciAttribute, VLAN_FILTER_ME_NUM_OF_ENTRY_ATT, flag);  
failure:
    return -1;
}
/*******************************************************************************************************************************
9.3.12 vlan tagging operation configuration data

********************************************************************************************************************************/

int omciMeInitForVlanTagOperation(omciManageEntity_t *omciManageEntity_p){
    omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
    
    /* init some param for this ME */
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciVlanTagDeleteAction;

    return 0;
}

int omciVlanTagDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
    int ret = -1;
    uint16 instId = 0;
    uint8 vlanRulePortId = 0;
    
    if(meInstant_ptr == NULL || omciPayLoad == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagDeleteAction-> parameter is NULL, fail");
        ret = -1;
        goto end;
    }

    /*get vlan tag operation instance id*/
    instId = get16((uint8*)&(omciPayLoad->meId)+2);

    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE, instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciVlanTagDeleteAction-> getVlanRulePortIdByInst, fail");
        ret = 0;
        goto end;
    }
    
    /*1. set default rule for this port*/   
    ret = setVlanRuleToDefaultByPort(vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagDeleteAction-> setVlanRuleToDefaultByPort, fail");
    }


    /*2. add 9.3.13 rule to this port*/
    ret = addVlanTagRuleByVlanPortId(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciVlanTagDeleteAction-> addVlanTagRuleByVlanPortId, fail");
    }
    
    ret = 0;
end:
    if(ret == 0){
        return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);

    }else{
        return -1;
    }
}


int setVlanTagOpValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 type){
    int ret = -1;
    uint16 instId = 0;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint8 vlanRulePortId = 0;   
    pon_vlan_rule currVlanRule1;
    pon_vlan_rule currVlanRule2;
    pon_vlan_rule_p currVlanRule1_ptr = NULL;
    pon_vlan_rule_p currVlanRule2_ptr = NULL;
    uint8 * attrValue = NULL;
    uint8 upMode = 0;
    uint16 tci = 0;

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> parameter is NULL, fail");
        goto end;
    }
    
    /*only support modify upstream vlan tag operation mode , tci attribute*/
    if((type == OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_1)
        || (type == OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_2)){
        //nothing
    }else{
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> type fail,  type = %d ",type);
        goto end;
    }
        
    /*whether create action or set action*/
    if(flag == OMCI_CREATE_ACTION_SET_VAL){
        ret = 0;
        goto end;
    }
    
    /*malloc for saving attribute value*/
    attrValue =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
    if(attrValue == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> attrValue calloc error ");
        ret = -1;
        goto end;
    }

    /*init vlan tag opration rule*/
    currVlanRule1_ptr = &currVlanRule1;
    currVlanRule2_ptr = &currVlanRule2; 
    ret = initVlanTagOpRule(currVlanRule1_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> initVlanTagOpRule currVlanRule1_ptr error ");
        ret = -1;
        goto end;
    }
    ret = initVlanTagOpRule(currVlanRule2_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> initVlanTagOpRule currVlanRule2_ptr error ");
        ret = -1;
        goto end;
    }
    

    /*get vlan tag operation instance id*/
    instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

    /*get 9.3.12 Upsream tag operation mode attribute value*/
    ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE, instId, 1, attrValue, 1);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> getAttributeValueByInstId  Upsream tag operation mode  fail ");
        ret = -1;
        goto end;
    }   
    upMode = *attrValue;
    
    /*get 9.3.12 Upstream TCI Value attribute value*/
    ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE, instId, 2, attrValue, 2);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> getAttributeValueByInstId  Upsream tag operation mode  fail ");
        ret = -1;
        goto end;
    }
    tci = get16(attrValue);

    /*get old vlan tag rule*/
    ret = setVlanTagOpRuleValByTci(upMode, tci, currVlanRule1_ptr, currVlanRule2_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> setVlanTagOpRuleValByTci 1 fail ");
        ret = -1;
        goto end;
    }

    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE,instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> getVlanRulePortIdByInst, error");
        ret = -1;
        goto end;
    }


    /*1. del current vlan operation rule in  this port */
    ret = delVlanTagOpRuleByRuleInfo(vlanRulePortId, currVlanRule1_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> delAllVlanTagOpRuleByRuleInfo currVlanRule1_ptr fail ");
    }
    ret = delVlanTagOpRuleByRuleInfo(vlanRulePortId, currVlanRule2_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> delAllVlanTagOpRuleByRuleInfo currVlanRule2_ptr fail ");
    }
    
    /*2. update vlan tag rule*/
    if(type == OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_1){
        upMode = *value;        
    }else if(type == OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_2){
        tci = get16((uint8 *)value);
    }else{
        //nothing
    }
    
    ret = setVlanTagOpRuleValByTci(upMode, tci, currVlanRule1_ptr, currVlanRule2_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> setVlanTagOpRuleValByTci 2  fail ");
        ret = -1;
        goto end;
    }   

    // FIX this, default tpid for 9.3.12 should be 8100 or 88a8
    /*3. set 9.3.12 vlan TPID  rule*/
    ret = pon_set_vlan_tpid(vlanRulePortId, OMCI_VLAN_TAG_OP_RULE_TPID_8100, OMCI_VLAN_TAG_OP_RULE_TPID_8100);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> 9.3.12 pon_set_vlan_tpid, fail");
        ret = -1;
        goto end;
    }
        
    /*4. set  current vlan operation rule*/
    ret = pon_add_vlan_rule(vlanRulePortId, currVlanRule1);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> pon_add_vlan_rule->currVlanRule1, fail");
        ret = -1;
        goto end;
    }
    ret = pon_add_vlan_rule(vlanRulePortId, currVlanRule2);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpValue-> pon_add_vlan_rule->currVlanRule2, fail");
        ret = -1;
        goto end;
    }
    
    ret = 0;
end:
    if(attrValue != NULL){
        free(attrValue);
    }
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return -1;
    }
}
int setVlanTagOpUpModeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    return setVlanTagOpValue(value, meInstantPtr, omciAttribute, flag, OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_1);
}

int setVlanTagOpTCIValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    return setVlanTagOpValue(value, meInstantPtr, omciAttribute, flag, OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_2);
}
int setVlanTagOpDownModeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
    uint8 downMode = 0;
    uint16 instId = 0;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint8 vlanRulePortId = 0;

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpDownModeValue-> parameter is NULL, fail");
        ret = -1;
        goto end;
    }

    /*get downstream mode*/
    downMode = *(uint8 *)value;
    switch(downMode){
        case OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_0:
            break;
        case OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_ATT_STRIP_VAL:
            downMode = OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_2;
            break;
        default:
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpDownModeValue-> downMode doesn't support, fail");
        ret = -1;
        goto end;
    }

    /*get vlan tag operation instance id*/
    instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
    if(flag != OMCI_CREATE_ACTION_SET_VAL){ 
        /*get vlan rule port id*/
        ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE,instId, &vlanRulePortId);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpDownModeValue-> getVlanRulePortIdByInst, fail");
            ret = -1;
            goto end;
        }

        /*set downstream mode rule*/
        ret = pon_set_vlan_downstream_mode(vlanRulePortId, downMode);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpDownModeValue-> pon_set_vlan_downstream_mode, fail");
            ret = -1;
            goto end;
        }
    }
    ret = 0;
end:    
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);

    }else{
        return -1;
    }
}

int setVlanTagAssociaValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 type){
    int ret = -1;
    uint16 instId = 0;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint8 vlanRulePortId = 0;   

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue-> parameter is NULL, fail");
        goto end;
    }
    
    /*only support modify associate type or associate pointer  attribute*/
    if((type != OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_4)
        && (type != OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_5)){
        ret = -1;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue-> type fail,  type = %d ",type);
        goto end;
    }
        
    /*whether create action or set action*/
    if((type == OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_4) && (flag == OMCI_CREATE_ACTION_SET_VAL)){
        ret = 0;
        goto end;
    }

    /*get vlan tag operation instance id*/
    instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

    /*when set associate pointer in create action, set new value to instance memory*/
    if((type == OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_5) && (flag == OMCI_CREATE_ACTION_SET_VAL)){
        ret = setGeneralValue(value, meInstantPtr , omciAttribute, flag);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue->create setGeneralValue, error");        
            ret = -1;
            goto end;
        }
    }
    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE,instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue-> getVlanRulePortIdByInst, error");      
        ret = -1;
        goto end;
    }
    
    /*1. set default rule for this port*/
    ret = setVlanRuleToDefaultByPort(vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue-> setVlanRuleToDefaultByPort, fail");
    }
    /*2. add 9.3.13 rule to this port*/
    ret = addVlanTagRuleByVlanPortId(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue-> addVlanTagRuleByVlanPortId, fail");
    }
    
    if(flag != OMCI_CREATE_ACTION_SET_VAL){
        /*3. update attribute value*/       
        ret = setGeneralValue(value, meInstantPtr , omciAttribute, flag);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue->set action--> setGeneralValue, error");     
            ret = -1;
            goto end;
        }
    
        /*4. update vlan rule port id*/
        ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE,instId, &vlanRulePortId);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue-> getVlanRulePortIdByInst, error");      
            ret = -1;
            goto end;
        }       
    }
    /*5. add 9.3.12 vlan rule to this port*/
    ret = addVlanTagRuleByVlanPortId(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE, vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagAssociaValue-> 9.3.13, addVlanTagRuleByVlanPortId, fail");
    }

    ret = 0;
end:
    if((ret == 0)
        && (type == OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_4)
        && (flag == OMCI_CREATE_ACTION_SET_VAL)){
            ret = setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }

    if(ret != 0){
        ret = -1;
    }
    return ret;
}
int setVlanTagOpAssociaTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 associationType = 0;

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpAssociaTypeValue-> parameter is NULL, fail");
        return -1;
    }

    associationType = *(uint8 *)value;
    switch(associationType){
        case OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_0:
        case OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_3:
        case OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_10:
#ifdef TCSUPPORT_PON_IP_HOST
        case OMCI_VLAN_TAG_OP_ASSOCIATION_TYPE_1:
#endif
            break;
        default: //don't support this type
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagOpAssociaTypeValue-> not support type %d, fail", associationType);
            return -1;
    }   

    return setVlanTagAssociaValue(value, meInstantPtr, omciAttribute, flag, OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_4);

}

int setVlanTagOpAssociaPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){

    return setVlanTagAssociaValue(value, meInstantPtr, omciAttribute, flag, OMCI_VLAN_TAG_OP_CFG_DATA_ATTR_5);
}
/*******************************************************************************************************************************
9.3.13 extended vlan tagging operation configuration data

********************************************************************************************************************************/
int omciMeInitForVlanTagExtendOperation(omciManageEntity_t *omciManageEntity_p){
    omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
    
    /* init some param for this ME */
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciExtVlanTagDeleteAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;

    return 0;
}
int omciExtVlanTagDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
    int ret = -1;
    uint16 instId = 0;
    uint8 vlanRulePortId = 0;

    if(meInstant_ptr == NULL || omciPayLoad == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciExtVlanTagDeleteAction-> parameter is NULL, fail");
        ret = -1;
        goto end;
    }
    
    /*get instance id*/ 
    instId = get16((uint8*)&(omciPayLoad->meId)+2);
    
    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciExtVlanTagDeleteAction-> getVlanRulePortIdByInst 1, fail");
        ret = 0;
        goto end;
    }
    
    /*1. set default rule for this port*/   
    ret = setVlanRuleToDefaultByPort(vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciExtVlanTagDeleteAction-> setVlanRuleToDefaultByPort, fail");
    }

    /*2. add 9.3.12 rule to this port*/
    ret = addVlanTagRuleByVlanPortId(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE, vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpAssociaTypeValue-> addVlanTagRuleByVlanPortId, fail");
    }

    ret = 0;
end:
    if(ret == 0){
        return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);
    }else{
        return -1;
    }
}

int setExtVlanTagOpAssociaValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 attrIndex){
    int ret = -1;
    uint16 instId = 0;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint8 vlanRulePortId = 0;
    uint16 newValue = 0;
    uint16 oldValue = 0;
    uint8 *attributeValuePtr = NULL;
    uint16 length = 0;
//  uint16 inputTPID = 0;
//  uint16 outputTPID = 0;
//  uint8 downMode = 0;

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpAssociaTypeValue-> parameter is NULL, fail");
        ret = -1;
        goto end;
    }

    if(attrIndex == OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_1){
        newValue = *(uint8 *)value;
    }else if(attrIndex == OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_7){
        newValue = get16((uint8 *)value);
    }else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpAssociaTypeValue-> attrbute index parameter error, fail");
        ret = -1;
        goto end;   
    }

    if(flag != OMCI_SET_ACTION_SET_VAL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpAssociaTypeValue-> flag = %d don't support, fail", flag);
        ret = -1;
        goto end;
    }
    
    /*execute set action, update vlan rule of this port*/
    /*get old value*/
    attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, omciAttribute->attriName, &length);
    if(attributeValuePtr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpAssociaTypeValue-> omciGetInstAttriByName, fail");
        ret = -1;
        goto end;
    }
    if(attrIndex == OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_1){
        oldValue = *attributeValuePtr;
    }else if(attrIndex == OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_7){
        oldValue = get16(attributeValuePtr);
    }
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpAssociaTypeValue-> oldValue = 0x%02x, newValue=0x%02x", oldValue, newValue);
    if(oldValue == newValue){//no change
        ret = 0;
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpAssociaTypeValue->oldValue == newValue");
        goto end;
    }

    /*get instance id*/
    instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);    
    
    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpAssociaTypeValue-> getVlanRulePortIdByInst 1, fail");
        ret = 0;
        goto end;
    }

    /*1. set default rule for this port*/
    ret = setVlanRuleToDefaultByPort(vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpAssociaTypeValue-> setVlanRuleToDefaultByPort, fail");
    }

    /*2. add 9.3.12 rule to this port*/
    ret = addVlanTagRuleByVlanPortId(OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE, vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpAssociaTypeValue-> addVlanTagRuleByVlanPortId, fail");
    }
    
    /*3. update attribute value*/
    ret =  setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpAssociaTypeValue-> setGeneralValue fail");
        ret = -1;
        goto end;
    }

    /*4. update vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE,instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpAssociaTypeValue-> getVlanRulePortIdByInst 2, fail");
        ret = 0;
        goto end;
    }   
    
    /*5. add 9.3.13 rule to this port*/
    ret = addVlanTagRuleByVlanPortId(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpAssociaTypeValue-> 9.3.13, addVlanTagRuleByVlanPortId, fail");
    }

    ret = 0;
end:
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        ret = -1;
        return ret;
    }
            
}
int setExtVlanTagOpAssociaTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    uint8 newValue = 0;

    newValue = *(uint8 *)value;
    switch(newValue){
        case OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_0://support
        case OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_2:
#ifdef TCSUPPORT_PON_IP_HOST
        case OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_3:
#endif
            break;
        case OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_10:
            return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
        default:
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setExtVlanTagOpAssociaTypeValue-> 9.3.13, not support this type, type = %d, fail", newValue);
            return -1;
    }

    if(flag == OMCI_CREATE_ACTION_SET_VAL){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return setExtVlanTagOpAssociaValue(value, meInstantPtr, omciAttribute, flag, OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_1);            
    }
}
int32 getExtVlanTagOpMaxNumValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
    uint16 maxNum = OMCI_MAX_PORT_VLAN_RULE_NUM;

    return getTheValue(value, (char *)&maxNum, 2, omciAttribute);
}

int setExtVlanTagOpTPIDValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 attrIndex){
    int ret = -1;
    uint16 instId = 0;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;   
    uint8 vlanRulePortId = 0;
    uint16 inputTPID = 0;
    uint16 outputTPID = 0;
    uint8 *attrValue = NULL;
    uint8 currAttrIndex = 0;

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTPIDValue-> parameter is NULL, fail");
        ret = -1;
        goto end;
    }
    
    /*malloc for saving attribute value*/
    attrValue =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
    if(attrValue == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTPIDValue-> attrValue calloc error ");
        ret = -1;
        goto end;
    }

    if(attrIndex == OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_3){
        inputTPID = get16((uint8 *)value);
        currAttrIndex = OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_4;
    }else if(attrIndex == OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_4){
        outputTPID = get16((uint8 *)value);
        currAttrIndex = OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_3;
    }else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTPIDValue-> attrIndex parameter error, fail");
        ret = -1;
        goto end;
    }
    
    /*get instance id*/
    instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpTPIDValue-> getVlanRulePortIdByInst 1, fail");
        ret = 0;
        goto end;
    }

    /*get 9.3.13 input or output TPID value*/
    ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, instId, currAttrIndex, attrValue, 2);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTPIDValue-> getAttributeValueByInstId  9.3.13 input TPID fail ");
        ret = -1;
        goto end;
    }

    if(currAttrIndex == OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_3){
        inputTPID= get16(attrValue);
    }else if(currAttrIndex == OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_4){
        outputTPID= get16(attrValue);
    }

    /*set 9.3.13 vlan TPID  rule*/
    ret = pon_set_vlan_tpid(vlanRulePortId, inputTPID, outputTPID);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTPIDValue-> 9.3.13 pon_set_vlan_tpid, fail");
        ret = -1;
        goto end;
    }   

    ret = 0;
end:
    if(attrValue != NULL){
        free(attrValue);
    }
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        ret = -1;
        return ret;
    }
}
int setExtVlanTagOpInputTPIDValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){

    return setExtVlanTagOpTPIDValue(value, meInstantPtr, omciAttribute, flag, OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_3);
}
int setExtVlanTagOpOutputTPIDValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){

    return setExtVlanTagOpTPIDValue(value, meInstantPtr, omciAttribute, flag, OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_4);
}
int setExtVlanTagOpDownModeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
    uint16 instId = 0;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;   
    uint8 vlanRulePortId = 0;
    uint8 downMode = 0;
    uint8 newValue = 0;

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpDownModeValue-> parameter is NULL, fail");
        ret = -1;
        goto end;
    }

    newValue = *(uint8 *)value;
    if(newValue == OMCI_EXT_VLAN_TAG_OP_DOWN_MODE_0){//reverse
        downMode = OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_1;
    }else if(newValue == OMCI_EXT_VLAN_TAG_OP_DOWN_MODE_1){//transparent
        downMode = OMCI_VLAN_TAG_OP_RULE_DOWNSTREAM_MODE_0;
    }else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpDownModeValue-> 9.3.13 downstream Mode value is wrong,  fail ");
        ret = -1;
        goto end;
    }   
    
    /*get instance id*/
    instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpDownModeValue-> getVlanRulePortIdByInst 1, fail");
        ret = 0;
        goto end;
    }
    /*set 9.3.13  downstream mode rule*/
    ret = pon_set_vlan_downstream_mode(vlanRulePortId, downMode);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setVlanTagRuleByInst-> 9.3.13 pon_set_vlan_downstream_mode, fail");
        ret = -1;
        goto end;
    }

    ret = 0;
end:
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        ret = -1;
        return ret;
    }   
}

int32 getExtVlanTagOpTblValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
    omciTableAttriValue_t *tmp_ptr = NULL;
    omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr; 
    char *tableBuffer = NULL;
    uint32 tableSize = 0;
    uint16 instanceId = 0;
    int ret = -1;
    omciManageEntity_t *me_p = NULL;

    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE);
    if(me_p ==  NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getExtVlanTagOpTblValue->omciGetMeByClassId fail");
        ret = -1;
        goto end;
    }
    
    tmp_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, me_p->omciAttriDescriptList[6].attriIndex);
    if(tmp_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getExtVlanTagOpTblValue->omciGetTableValueByIndex fail");
        ret = -1;
        goto end;   
    }
    
    /*1.calculate the table size*/
    tableSize = tmp_ptr->tableSize;
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getExtVlanTagOpTblValue: tableSize = %d",tableSize);

    
    /*2.get table content*/
    tableBuffer = (char *)tmp_ptr->tableValue_p;
    if(tableBuffer == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getExtVlanTagOpTblValue->tableBuffer is NULL");
        ret = -1;
        goto end;
    }

    /*3. add the table to omci archtecture*/
    instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
    ret = omciAddTableContent(tmpomciMeInst_ptr->classId, instanceId, (uint8 *)tableBuffer, tableSize ,omciAttribute->attriName);
    if (ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getExtVlanTagOpTblValue->omciAddTableContent fail");
        ret = -1;
        goto end;
    }

    ret = 0;
end:
    if(ret != 0){
        ret = -1;
        tableSize = 0;
    }
//  omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getExtVlanTagOpTblValue->tableSize = %d",tableSize);
    return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}

#ifdef TCSUPPORT_PON_IP_HOST
int setWanNode(uint16 vlan_id)
{
    char temp[32] = {0};
	
    sprintf(temp,"%d",vlan_id);
    tcapi_set("Wan_PVC1", "VLANID", temp);
    tcapi_set("Wan_PVC1_Entry0", "VLANID", temp);
    tcapi_get("Wan_PVC0_Entry0","ServiceList",temp);
    if (strcmp(temp,"VOICE_INTERNET") != 0)
    {
        tcapi_set("Wan_PVC0_Entry0","ServiceList","VOICE_INTERNET");
    }

    if(delayOmciCommitNode("Wan") !=0){
		OMCI_ERROR("addOmciCommitNodeList and start the Timer fail\n");
		return -1;  
	}	
	return 0;
}


int setExtVlantoWanNode(pon_vlan_rule_p currVlanRule1_ptr,uint16 ipHostIstId)
{
    int ret = 0;
    char value1[10] = {0};
    char value2[10] = {0};

	if(currVlanRule1_ptr->filter_outer_pri != OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8
		&& currVlanRule1_ptr->filter_outer_vid!= OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096
		&& currVlanRule1_ptr->filter_outer_vid!= 0)
	{
    	snprintf(value1, 10,"%d", currVlanRule1_ptr->filter_outer_vid);
    	snprintf(value2, 10,"%d", currVlanRule1_ptr->filter_outer_pri);
        

    	/* update vlanid and dot1p to tr69 wan connection */
    	setWanInfo(WAN_IF_TYPE_VLANID, ipHostIstId,value1); /* set vlan id */
    	setWanInfo(WAN_IF_TYPE_DOT1P, ipHostIstId,value2); /* set dot1p */
	}
    else if(currVlanRule1_ptr->filter_inner_pri != OMCI_VLAN_TAG_OP_RULE_FILTER_PRI_8
		&& currVlanRule1_ptr->filter_inner_vid != OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096
		&& currVlanRule1_ptr->filter_inner_vid != 0)
	{
    	snprintf(value1, 10,"%d", currVlanRule1_ptr->filter_inner_vid);
    	snprintf(value2, 10,"%d", currVlanRule1_ptr->filter_inner_pri);

    	/* update vlanid and dot1p to tr69 wan connection */
    	setWanInfo(WAN_IF_TYPE_VLANID, ipHostIstId,value1); /* set vlan id */
    	setWanInfo(WAN_IF_TYPE_DOT1P, ipHostIstId, value2); /* set dot1p */
    }
	else if(currVlanRule1_ptr->add_sec_pri != OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_15
		&& currVlanRule1_ptr->add_sec_vid != OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096
		&& currVlanRule1_ptr->add_sec_vid != 0)
	{
    	snprintf(value1, 10,"%d", currVlanRule1_ptr->add_sec_vid);
    	snprintf(value2, 10,"%d", currVlanRule1_ptr->add_sec_pri);

    	/* update vlanid and dot1p to tr69 wan connection */
    	setWanInfo(WAN_IF_TYPE_VLANID, ipHostIstId, value1); /* set vlan id */
    	setWanInfo(WAN_IF_TYPE_DOT1P, ipHostIstId, value2); /* set dot1p */  
    }
    else if (currVlanRule1_ptr->add_fst_pri != OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PBIT_15
		&& currVlanRule1_ptr->add_fst_vid != OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_VID_4096
		&& currVlanRule1_ptr->add_fst_vid != 0)
	{
    	snprintf(value1, 10,"%d", currVlanRule1_ptr->add_fst_vid);
    	snprintf(value2, 10,"%d", currVlanRule1_ptr->add_fst_pri);

    	/* update vlanid and dot1p to tr69 wan connection */
    	setWanInfo(WAN_IF_TYPE_VLANID, ipHostIstId, value1); /* set vlan id */
    	setWanInfo(WAN_IF_TYPE_DOT1P, ipHostIstId, value2); /* set dot1p */
    }

    return ret;
}
#endif


int setExtVlanTagOpTblValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
    uint8 *temp_ptr = NULL;
    uint8 *temp2_ptr = NULL;
    uint8 *delRule_ptr = NULL;
    uint8 *entryValue_ptr = NULL;
    pon_vlan_rule currVlanRule1;
    pon_vlan_rule_p currVlanRule1_ptr = NULL;
    uint8 defaultRuleFlag = 0;
    uint8 vlanRulePortId = 0;
    uint8 currVlanRulePortId = 0;
    uint16 instId = 0;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;   
    omciManageEntity_t *me_p=NULL;  

    omciTableAttriValue_t *tableConent_ptr = NULL; //table content pointer
    uint32 tableSize= 0;
    uint32 length = 0;
    uint32 tempU32 = 0;
    uint8 findFlag = 0;
    uint8 setDefaultVlanRuleFlag = 0;
    int i = 0;
    uint8 delEntryFlag = OMCI_EXT_VLAN_TAG_NO_DEL_ENTRY;
	
#ifdef TCSUPPORT_PON_IP_HOST
    omciManageEntity_t *macBridgePortConDataMe_p=NULL;
    uint16 macBridgePortConDatainstId = 0;	
    omciMeInst_t *macBridgePortConDataInst_p=NULL;
    uint8 *associationTypePtr=NULL;
    uint8 *associatedMePtr=NULL;
    uint8 *macBridgePortConDataValue=NULL;
    uint16 valueLength = 0;
    uint8 tpTypeValue = 0;
    uint8 associationTypeValue = 0;
    uint16 associatedMeId = 0;
    char iphostNode[32] = {0};
#endif

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue-> parameter is NULL, fail");
        ret = -1;
        goto end;
    }

    me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE);
    if(me_p ==  NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue-> omciGetMeByClassId , fail");
        ret = -1;
        goto end;
    }

    /*init*/
    currVlanRule1_ptr = &currVlanRule1;
    ret = initVlanTagOpRule(currVlanRule1_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue-> initVlanTagOpRule currVlanRule1_ptr error ");
        ret = -1;
        goto end;
    }   
    /*get instance id*/
    instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);    
    temp_ptr = (uint8 *)value;

    /*The OLT deletes a tables entry by setting all of its last eight bytes to 0xFF*/
    delRule_ptr = temp_ptr + OMCI_EXT_VLAN_TAG_ATTR_JUST_8BYTES;
    for(i = 0; i < OMCI_EXT_VLAN_TAG_ATTR_LAST_8BYTES; i++){
        if(*(delRule_ptr+i) != 0xFF){
            break;
        }
    }
    if(i >= OMCI_EXT_VLAN_TAG_ATTR_LAST_8BYTES){
        delEntryFlag = OMCI_EXT_VLAN_TAG_DEL_ENTRY;
    }

    /*set vlan rule information to pon_vlan_rule structure*/
    ret = setExtVlanTagRuleByPkt(currVlanRule1_ptr, &defaultRuleFlag, temp_ptr, OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue-> 9.3.13 setExtVlanTagRuleByPkt, fail");
        ret = -1;
        goto end;
    }

#if defined(TCSUPPORT_PON_IP_HOST)
	associationTypePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , 
		me_p->omciAttriDescriptList[1].attriName, &valueLength);
	associationTypeValue = *((uint8*)associationTypePtr);
	
	if(OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_3 == associationTypeValue){
        
        associationTypePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , 
		    me_p->omciAttriDescriptList[7].attriName, &valueLength);
        associatedMeId = get16(associationTypePtr);
		ret = setExtVlantoWanNode(currVlanRule1_ptr,associatedMeId);
		if(ret != 0){
			OMCI_ERROR("\r\n setExtVlanTagOpTblValue-> 9.3.13 setExtVlanTagRuleByPkt, fail");
			ret = -1;
			goto end;
		}

	}

	if(OMCI_EXT_VLAN_TAG_OP_ASSOCIATION_TYPE_0 == associationTypeValue){
		associatedMePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , 
			me_p->omciAttriDescriptList[7].attriName, &valueLength);
		associatedMeId = get16(associatedMePtr);
		macBridgePortConDataMe_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
		if(NULL == macBridgePortConDataMe_p){
			OMCI_ERROR("\r\n setExtVlanTagOpTblValue-> omciGetMeByClassId , fail");
			ret = -1;
			goto end;
		}
		macBridgePortConDataInst_p = macBridgePortConDataMe_p->omciInst.omciMeInstList;
		while(NULL != macBridgePortConDataInst_p){
			macBridgePortConDatainstId = get16(macBridgePortConDataInst_p->attributeVlaue_ptr);
			if(macBridgePortConDatainstId == associatedMeId)
				break;
			macBridgePortConDataInst_p = macBridgePortConDataInst_p->next;
		}
		if(NULL != macBridgePortConDataInst_p){
			macBridgePortConDataValue = omciGetInstAttriByName(macBridgePortConDataInst_p,
					macBridgePortConDataMe_p->omciAttriDescriptList[3].attriName, &valueLength);
			tpTypeValue = *((uint8*)macBridgePortConDataValue);
			if(OMCI_MAC_BRIDGE_PORT_TP_TYPE_4 == tpTypeValue){
                macBridgePortConDataValue = omciGetInstAttriByName(macBridgePortConDataInst_p,
					macBridgePortConDataMe_p->omciAttriDescriptList[4].attriName, &valueLength);
			    associatedMeId = get16(macBridgePortConDataValue);
				ret = setExtVlantoWanNode(currVlanRule1_ptr,associatedMeId);
				if(ret != 0){
					OMCI_ERROR("\r\n setExtVlanTagOpTblValue-> 9.3.13 setExtVlanTagRuleByPkt, fail");
					ret = -1;
					goto end;
				}
			}
		}
	}

    
    sprintf(iphostNode, "XPON_IPHost%d", associatedMeId);
	if(delayOmciCommitNode(iphostNode) !=0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addOmciCommitNodeList and start the Timer fail");
		ret = -1;
		goto end;
	}
#endif


    /*get current table content*/
    tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_p, me_p->omciAttriDescriptList[6].attriIndex);
    if(tableConent_ptr == NULL){
        findFlag = 0;
        tableSize = 0;
        temp2_ptr = NULL;
    }else{
        tableSize = tableConent_ptr->tableSize;
        temp2_ptr = tableConent_ptr->tableValue_p;
    }

    if(tableSize == 0){
        setDefaultVlanRuleFlag = OMCI_EXT_VLAN_TAG_OP_TBL_SET_DEFAULT_RULE_FLAG_1;
    }

    /*search record*/
    length = OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
    while(temp2_ptr  != NULL){      
        if(memcmp(temp2_ptr, temp_ptr, OMCI_EXT_VLAN_TAG_ATTR_JUST_8BYTES) == 0){
            findFlag = 1;//find
            break;
        }
        length += OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
        if(length > tableConent_ptr->tableSize){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpTblValue->length > tableConent_ptr->tableSize");
            findFlag = 0;//not find
            break;
        }
        temp2_ptr += OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
    }       

    /*get real length*/
//  length -= OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;

    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpTblValue-> getVlanRulePortIdByInst 1, fail");
        goto end;
    }
    if(defaultRuleFlag == OMCI_EXT_VLAN_TAG_OP_TBL_DEFAULT_RULE_FLAG_1){
        currVlanRulePortId = OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT;        
    }else{
        currVlanRulePortId = vlanRulePortId;
    }

    /*find and add  rule then return success,
    no find and delete rule then return success.
    */
    if(findFlag == 1){//find
    //  if(delEntryFlag != OMCI_EXT_VLAN_TAG_DEL_ENTRY){// add entry
    //      ret = 0;
    //      goto end;
    //  }
    }else{ //no find
        if(delEntryFlag == OMCI_EXT_VLAN_TAG_DEL_ENTRY){ //del entry
            ret = 0;
            goto end;
        }
    }
    
    /*2. prepare value for table*/
    /*malloc new space*/
    entryValue_ptr = calloc(1 , tableSize+OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN);          
    if(entryValue_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->entryValue_ptr calloc failure");
        ret = -1;
        goto end;       
    }
    memset(entryValue_ptr, 0, tableSize+OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN);

    /*call PON VLAN API and fill new space*/
    if(delEntryFlag == OMCI_EXT_VLAN_TAG_DEL_ENTRY){//del entry
        /*1. execute VLAN operation API*/
        if(delVlanTagOpRuleByRuleInfo(currVlanRulePortId, currVlanRule1_ptr) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->delVlanTagOpRuleByRuleInfo  failure");
        }

        if(currVlanRule1_ptr->filter_ethertype == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PPPoE_8863){
            currVlanRule1_ptr->filter_ethertype = 0x8864;
            if(delVlanTagOpRuleByRuleInfo(currVlanRulePortId, currVlanRule1_ptr) != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->delVlanTagOpRuleByRuleInfo 8864 failure");
            }
        }
        

        /*2 handle table in OMCI*/
        if(tableConent_ptr != NULL){
            temp2_ptr = tableConent_ptr->tableValue_p;
            temp_ptr = (uint8 *)value;
            /*search record*/
            length = OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
            while(temp2_ptr  != NULL){      
                if(memcmp(temp2_ptr, temp_ptr, OMCI_EXT_VLAN_TAG_ATTR_JUST_8BYTES) == 0){ //find and delete entry in  table
                    tempU32 = length - OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
                    if(tableConent_ptr->tableSize > OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN){//after delete 1 entry, if still have entry, process it
                    //if(tempU32 > 0){ // the number more than a entry
                        temp_ptr = entryValue_ptr;
                        memcpy(temp_ptr, tableConent_ptr->tableValue_p, tempU32);
                        temp_ptr += tempU32;
                        
                        temp2_ptr = tableConent_ptr->tableValue_p+length;
                        tempU32 = tableConent_ptr->tableSize - length;// copy length
                        if(tempU32 > 0){
                            memcpy(temp_ptr, temp2_ptr, tempU32);
                        }
                    }           
                    
                    tableSize -= OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
                    break;
                }
                length += OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
                if(length > tableConent_ptr->tableSize) {//no find and no happen
                    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->del entry and no find entry, fail");
                    break;
                }
                temp2_ptr += OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
            }
        }
        
    }else {// add entry
        /*1. execute VLAN operation API*/
        if((defaultRuleFlag == OMCI_EXT_VLAN_TAG_OP_TBL_DEFAULT_RULE_FLAG_1)
            && (currVlanRulePortId == OMCI_VLAN_TAG_OP_RULE_DEFAULT_PORT)){
            if(pon_enable_default_vlan_rule(vlanRulePortId) != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->9.3.13  pon_enable_default_vlan_rule, fail");
                ret = -1;
                goto end;
            }
        }
        if(delVlanTagOpRuleByRuleInfo(currVlanRulePortId, &currVlanRule1) != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->delVlanTagOpRuleByRuleInfo failure");
        }

        /*ZTE OLT IOT problem vlan tag mode */
        if (currVlanRule1.tag_num == 0)
        {
            currVlanRule1.add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
            currVlanRule1.add_fst_dei = OMCI_VLAN_TAG_DEI_0;
        }
        ret = pon_add_vlan_rule(currVlanRulePortId, currVlanRule1);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->9.3.13  pon_add_vlan_rule->currVlanRule1, fail");
            ret = -1;
            goto end;
        }
        if(currVlanRule1_ptr->filter_ethertype == OMCI_EXT_VLAN_TAG_OP_TBL_FILTER_PPPoE_8863){
            currVlanRule1_ptr->filter_ethertype = 0x8864;
            if(delVlanTagOpRuleByRuleInfo(currVlanRulePortId, &currVlanRule1) != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->delVlanTagOpRuleByRuleInfo failure");
            }
            /*ZTE OLT IOT problem vlan tag mode */
            if (currVlanRule1.tag_num == 0)
            {
                currVlanRule1.add_fst_tpid = OMCI_VLAN_TAG_OP_RULE_ADD_TPID_1;
                currVlanRule1.add_fst_dei = OMCI_VLAN_TAG_DEI_0;
            }
            ret = pon_add_vlan_rule(currVlanRulePortId, currVlanRule1);
            if(ret != 0){
                omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->9.3.13  pon_add_vlan_rule->currVlanRule1, 2 fail");
                ret = -1;
                goto end;
            }
        }       
        /*2 handle table in OMCI*/
        if(tableSize > 0){
            memcpy(entryValue_ptr, tableConent_ptr->tableValue_p, tableSize);
        }
        memcpy(entryValue_ptr+tableSize, value, OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN);
        tableSize += OMCI_EXT_VLAN_TAG_OP_TBL_ENTRY_LEN;
    }
    /*3. update table content in instance table list*/
    if(tableConent_ptr != NULL){//del table content  in instance table list
        ret = omciDelTableValueOfInst(tmpomciMeInst_p, me_p->omciAttriDescriptList[6].attriIndex);
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->omciDelTableValueOfInst, failure");
            ret = -1;
            goto end;
        }
    }
        
    if(tableSize == 0){//current table is empty
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpTblValue->current table is empty");
        ret = 0;
        goto end;
    }
#if 0 //LHS DBG
    omciDumpTable(entryValue_ptr, tableSize);
#endif
    tableConent_ptr = omciAddTableValueToInst(tmpomciMeInst_p, me_p->omciAttriDescriptList[6].attriIndex, tableSize, entryValue_ptr);
    if(tableConent_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->omciAddTableValueToInst failure");
        ret = -1;
        goto end;   
    }

    /*set untagged, tagged default rule, When set first rule.*/
    if(setDefaultVlanRuleFlag == OMCI_EXT_VLAN_TAG_OP_TBL_SET_DEFAULT_RULE_FLAG_1){
        ret = setExtVlanOpTBLDefaultRule();
        if(ret != 0){
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpTblValue->setExtVlanOpTBLDefaultRule failure");
        }
    }

    ret = 0;

end:
    if(entryValue_ptr != NULL){
        free(entryValue_ptr);
    }
    if(ret != 0){
        ret = -1;
    }
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpTblValue->tableSize = %d",tableSize); 
    return ret;
}

int setExtVlanTagOpAssociaPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){

    if(flag == OMCI_CREATE_ACTION_SET_VAL){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        return setExtVlanTagOpAssociaValue(value, meInstantPtr, omciAttribute, flag, OMCI_EXT_VLAN_TAG_OP_ATTR_INDEX_7);            
    }
}

int setExtVlanTagOpDSCPMappingValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
    int ret = -1;
    uint16 instId = 0;
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;   
    uint8 vlanRulePortId = 0;
    uint32 * dscpMapU32_ptr = NULL;
    uint8 *temp_ptr = NULL;
    int i = 0;

    if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpDSCPMappingValue-> parameter is NULL, fail");
        ret = -1;
        goto end;
    }

    /*malloc for DSCP mapping*/
    dscpMapU32_ptr = (uint32 *)calloc(OMCI_DSCP_MAP_U32_NUM, sizeof(uint32));
    if(dscpMapU32_ptr == NULL){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpDSCPMappingValue-> dscpMapU32_ptr calloc error ");
        ret = -1;
        goto end;
    }

    /*get instance id*/
    instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

    /*get vlan rule port id*/
    ret = getVlanRulePortIdByInst(OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE, instId, &vlanRulePortId);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n seExtVlanTagOpDSCPMappingValue-> getVlanRulePortIdByInst 1, fail");
        ret = 0;
        goto end;
    }
    
    temp_ptr = (uint8 *)value;
    for(i = 0; i<OMCI_DSCP_MAP_U32_NUM; i++){
        *(dscpMapU32_ptr+i) = get32(temp_ptr);
        temp_ptr += 4;
    }
    
    /*set 9.3.13 dscp mapping rule*/
    ret = pon_set_vlan_dscp_map(vlanRulePortId, dscpMapU32_ptr);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n seExtVlanTagOpDSCPMappingValue-> 9.3.13 pon_set_vlan_dscp_map, fail");
        ret = -1;
        goto end;
    }

    ret = 0;
end:
    if(dscpMapU32_ptr != NULL){
        free(dscpMapU32_ptr);
    }
    if(ret == 0){
        return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    }else{
        ret = -1;
        return ret;
    }   
}

