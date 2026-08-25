/**
*@file
*@brief This file defines traffic flow related macro and struct that used by hal interface
*@author Shen Jingguo
*@date     2013.3.10
*@copyright 
*******************************************************************
* Copyright (c) 2013 Alcatel Lucent Shanghai Bell Software, Inc.
*             All Rights Reserved
*
* This source is confidential and proprietary and may not
* be used without the written permission of Alcatel Lucent
* Shanghai Bell, Inc.
********************************************************************
*@par Module History:
*/


#ifndef _HAL_TYPE_FLOW_H_
#define _HAL_TYPE_FLOW_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif 

#ifndef __KERNEL__
#include <netinet/in.h>
#else
#include <linux/in.h>
#include <linux/in6.h>
#endif

#include "hal_type_common.h"

/** defined in hal_itf_flow.c, used for check the traceLevel when print the trace*/
extern unsigned int g_hal_sfueth_trc_level;

/**@def HAL_SFUETH_DBGPRT(fmt, args...) 
*print the SFU ethernet module debug trace*/
#define HAL_SFUETH_DBGPRT(fmt, args...) if (g_hal_sfueth_trc_level >0) \
                                printf("DBG %s:" fmt, __FUNCTION__, ##args);

/**
*@enum hal_vlan_tag_action_t
*Define the vlan tag action*/
typedef enum
{
    /** the min value of the enum, never used*/
    HAL_VLAN_ACTION_MIN_E,
    /** the vlan action is transparent*/
    HAL_VLAN_ACTION_TRANSPARENT_E=0,
    /** the vlan action is add a vlan tag*/
    HAL_VLAN_ACTION_ADD_E=1,
    /** the vlan action is add a vlan tag and do pbit remark*/
    HAL_VLAN_ACTION_ADD_AND_REMARK_E = 2,
    /** the vlan action is replace a vlan tag*/
    HAL_VLAN_ACTION_REPLACE_E = 3,
    /** the vlan action is replace a vlan tag and do pbit remark*/
    HAL_VLAN_ACTION_REPLACE_AND_REMARK_E = 4,
    /** the vlan action is replace a vlan tag and copy the  pbit*/
    HAL_VLAN_ACTION_REPLACE_AND_COPY_E = 5,
    /** the vlan action is remove a vlan tag*/
    HAL_VLAN_ACTION_REMOVE_E = 6,

    HAL_VLAN_ACTION_REMAP_PBIT_E = 7,
    
    /** the max value of the enum, never used*/

    HAL_VLAN_ACTION_MAX_E
} hal_vlan_tag_action_t;

/**
*@enum hal_stream_type_t
*Define the type of the stream rule*/


// add by lbliang

typedef enum
{
    /** the min value of the enum, never used*/
    VLAN_ACTION_MIN_E = -1,
    /** the vlan action is transparent*/
    VLAN_ACTION_TRANSPARENT_E=0,
    /** the vlan action is add a vlan tag and do pbit remark*/
    VLAN_ACTION_ADD_AND_REMARK_E = VLAN_ACTION_TRANSPARENT_E+1,
    /** the vlan action is add a vlan and cpoy pbit from inner vlan*/
    VLAN_ACTION_ADD_AND_COPY_INNER_PBIT_E = VLAN_ACTION_TRANSPARENT_E+2,    
    /** the vlan action is add a vlan and cpoy pbit from outer vlan*/
    VLAN_ACTION_ADD_AND_COPY_OUTER_PBIT_E = VLAN_ACTION_TRANSPARENT_E+3,    
     /** the vlan action is add a vlan and cpoy derive pbit from dscp*/
    VLAN_ACTION_ADD_AND_DERIVE_PBIT_FROM_DSCP_E = VLAN_ACTION_TRANSPARENT_E+4,    
   
    /** the vlan action is remove a vlan tag*/
    VLAN_ACTION_REMOVE_E = 100+VLAN_ACTION_TRANSPARENT_E,
    /** the vlan action is replace a vlan tag and do pbit remark*/
    VLAN_ACTION_REPLACE_AND_REMARK_E = VLAN_ACTION_REMOVE_E+1,
    /** the vlan action is replace a vlan tag and pbit from inner vlan*/
    VLAN_ACTION_REPLACE_AND_COPY_INNER_PBIT_E = VLAN_ACTION_REMOVE_E+2,
    /** the vlan action is replace a vlan tag and pbit from outer vlan*/
    VLAN_ACTION_REPLACE_AND_COPY_OUTER_PBIT_E = VLAN_ACTION_REMOVE_E+3,
    /** the vlan action is replace a vlan tag and derive pbit from dscp*/
    VLAN_ACTION_REPLACE_AND_DERIVE_PBIT_FROM_DSCP_E = VLAN_ACTION_REMOVE_E+4,
    /** the max value of the enum, never used*/
    VLAN_ACTION_UNSUPPORT_E
} t_hal_vlan_tag_action;






typedef enum
{
     /** the traffic rule is used for untag packets*/                      
    NO_TAG_E = 0x0,
     /** the traffic rule is used for single tag packets*/                           
    SINGLE_TAG_E = 0x1,
      /** the traffic rule is used for double tag packets*/                           
    DOUBLE_TAG_E = 0x2
}t_hal_tag_mode;

typedef enum
{
    REMOVE_NO_TAG_E,
    REMOVE_ONE_TAG_E,
    REMOVE_TWO_TAG_E,
    REMOVE_FRAME_E
}t_hal_tag_num_to_remove;




typedef struct
{
    t_hal_tag_mode tagMode;
    t_hal_tag_num_to_remove remove_tag_num;
    t_hal_vlan_tag_action outervlan;
    t_hal_vlan_tag_action innervlan;
}t_hal_vlan_mode;


typedef enum
{
    STREAM_TYPE_UNTAG_INSERT_1TAG_E = 0,
    STREAM_TYPE_UNTAG_DO_NOTHING_E ,
    STREAM_TYPE_UNTAG_INSERT_2TAGS_E ,
    STREAM_TYPE_UNTAG_DROP_E,
   
    STREAM_TYPE_1TAG_INSERT_1TAG_E,
    STREAM_TYPE_1TAG_INSERT_1TAG_COPY_PBIT_E,
    STREAM_TYPE_1TAG_INSERT_2TAGS_E,
    STREAM_TYPE_1TAG_MODIFY_TAG_E,
    STREAM_TYPE_1TAG_MODIFY_TAG_COPY_PBIT_E,
    STREAM_TYPE_1TAG_MODIFY_TAG_INSERT_1TAG_E,
    STREAM_TYPE_1TAG_REMOVE_TAG_E,
    STREAM_TYPE_1TAG_DO_NOTHING_E,
    STREAM_TYPE_1TAG_DROP_E,

    STREAM_TYPE_2TAG_INSERT_1TAG_E,
    STREAM_TYPE_2TAG_INSERT_1TAG_COPY_PBIT_E,
    STREAM_TYPE_2TAG_INSERT_2TAGS_E,
    STREAM_TYPE_2TAG_INSERT_2TAGS_COPY_PBIT_E,
    STREAM_TYPE_2TAG_MODIFY_STAG_E,
    STREAM_TYPE_2TAG_MODIFY_STAG_COPY_PBIT_E,
    STREAM_TYPE_2TAG_MODIFY_BOTH_TAGS_E,
    STREAM_TYPE_2TAG_MODIFY_BOTH_TAGS_KEEP_PBIT_E,
    STREAM_TYPE_2TAG_SWAP_BOTH_TAGS_E,
    STREAM_TYPE_2TAG_REMOVE_STAG_E,
    STREAM_TYPE_2TAG_REMOVE_BOTH_TAGS_E,
    STREAM_TYPE_2TAG_DO_NOTHING_E,
    STREAM_TYPE_2TAG_DROP_E,

    STREAM_TYPE_UNTAG_INSERT_1TAG_TLS_E,	 
    STREAM_TYPE_1TAG_INSERT_1TAG_TLS_E,
    STREAM_TYPE_1TAG_INSERT_1TAG_COPY_PBIT_TLS_E,
    STREAM_TYPE_2TAG_INSERT_1TAG_TLS_E,
    STREAM_TYPE_2TAG_INSERT_1TAG_COPY_PBIT_TLS_E,
    
    STREAM_TYPE_MAX_TAG_ACTION
}hal_stream_type_t;

/**
*@brief tr069 or voip inforamtion
*
*Deinfe the structure used for classify of tr069 voip traffic*/
typedef struct
{
    /** 1: tr069, 2: voip */
    unsigned int app;	 
    /** the vlan id used for traffic classify*/
    unsigned int vlan;
    /** the pbit used for traffic classify*/
    unsigned int pri;
} hal_tr069_voip_info_t;

/**
*@brief l2 filter rule
*
*Deinfe the structure of the layer 2 traffic classify rule, used by omci*/
typedef struct
{
    /**  the outer vlan id  */
    unsigned short f_outer_vid;
    /**  the inner vlan id  */
    unsigned short f_inner_vid;
    /**  the outer pbit */
    unsigned short f_outer_pbit;
    /**  the inner pbit */
    unsigned short f_inner_pbit;
    /**  the outer tpid */
    unsigned short f_outer_tpid;
    /**  the inner tpid */
    unsigned short f_inner_tpid;
    /**  the ethernet type */
    unsigned short f_ether_type; 
} hal_flow_rule_filter_l2_t;

/**
*@brief l3 filter rule
*
*Deinfe the structure of the layer 3 traffic classify rule, used by tr069 */
typedef struct
{
    /**  the rule is enable or not */
    int enable;
    /**  the index or the rule*/
    int order;
    /**  the uni port of such rule configured to */	
    int lan_port;
    /**  the wan interface name of such rule*/	
    char wanif[32];
    /**  the destination ip address*/	
    struct in_addr dest_ip;
    /**  the mask of the destination ip address*/	
    unsigned int dest_ip_mask;
    /**  the exclude destination ip address*/		
    int dest_ip_exclude;
    /**  the source ip address*/		
    struct in_addr src_ip;
    /**  the mask of the source ip address*/		
    unsigned int src_ip_mask;
    /**  the exclude source ip address*/			
    int src_ip_exclude;
    /**  the protocol type*/			
    int proto;
    /**  the exclude protocol type*/			
    int proto_exclude;
    /**  the destination udp/tcp port*/			
    int dport;
     /**  the max destination udp/tcp port*/			
    unsigned int dport_max;
    /**  the exclude destination udp/tcp port*/			
    int dport_exclude;
    /**  the source udp/tcp port*/			
    int sport;
     /**  the max source udp/tcp port*/			
    unsigned int sport_max;
    /**  the exclude source udp/tcp port*/			
    int sport_exclude;
    /**  the source mac address*/			
    char smac[6];
    /**  the mask of source mac address*/			
    unsigned int smac_mask;
    /**  the destination mac address*/			
    char dmac[6];
    /**  the mask of destination mac address*/			
    unsigned int dmac_mask;
    /**  the tos*/			
    int tos;
    /**  the ethernet type*/			
    int ether_type;
    /**  the tro069 or voip classify*/			
    hal_tr069_voip_info_t tr069_voip_classify;
} hal_flow_rule_filter_l3_t;

/**
*@brief l3 vlan action rule
*
*Deinfe the structure of the layer 2 vlan action rule, same as omci EVTOCD treatment action*/
typedef struct
{
    /**  tag treatment  */
    unsigned short t_tag; 
    /**  outer vlan id  */
    unsigned short t_outer_vid;
    /**  inner vlan id  */
    unsigned short t_inner_vid;
    /**  outer pbit  */
    unsigned short t_outer_pbit;
    /**  inner pbit  */
    unsigned short t_inner_pbit;
    /**  outer tpid  */
    unsigned short t_outer_tpid;
    /**  inner tpid  */
    unsigned short t_inner_tpid;
    /**  output tpid  */
    unsigned short output_tpid;
    /**  input tpid  */
    unsigned short input_tpid;
} hal_flow_rule_action_l2_t;

/**
*@brief l3 vlan action rule
*
*Deinfe the structure of the layer 3 vlan action rule, usd by tr098*/
typedef struct
{
    /** the priority used for remark*/
    int priority;
    /** the dscp used for remark*/
    int dscp;
} hal_flow_rule_action_l3_t;

/**
*@brief stream flow info
*
*Deinfe the structure of stream flow info, include basic informtion and vlan filter and vlan action*/
typedef struct
{
    /** the application flow id*/
    unsigned int flow_id;
    /** the port*/
    hal_port_info_t port_info;
    /** the mask of filter and ation*/
    unsigned int layer_mask;
    /** the l2 traffic classify inforamtion*/
    hal_flow_rule_filter_l2_t l2_filter;
    /** the l3 traffic classify inforamtion*/
    hal_flow_rule_filter_l3_t l3_filter;
    /** the l2 vlan action inforamtion*/
    hal_flow_rule_action_l2_t l2_action;
    /** the l3 vlan action inforamtion*/
    hal_flow_rule_action_l3_t l3_action;
    /** the stream mode*/
    hal_stream_type_t stream_hanlde_mode;
    /** the dscp to pbit map*/
    unsigned char dscp_mapping[24];
    /** use dscp to pbit mapping or not*/
    unsigned int isDscp2PbitMapping; 
	//pbit to pon id mapping
	unsigned short pbit_to_pon_id_mapping[8];

    t_hal_vlan_mode vlan_action_mode ; // add    
} hal_stream_flow_info_t;

/**
*@brief multicast flow info
*
*Deinfe the structure of multicast flow info*/
typedef struct
{
    /** the ethernet flow id in hal, should be update later*/
    unsigned short ethFlow;
    /** the vlan id list for multicast*/
    unsigned short vlanActionId[5];
}hal_mcast_flow_info_t;

/**
*@brief dsp forward control
*
*Deinfe the structure used to control forward the broadcast to dsp or not*/
typedef struct
{
    /**1 one add, 2 means remove*/
    unsigned short cmd;
    /** the inforamtion include uni port, gemport and vlan id*/	
    unsigned int uni_gem_vid;
}hal_dspforward_ctrl_t;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif



