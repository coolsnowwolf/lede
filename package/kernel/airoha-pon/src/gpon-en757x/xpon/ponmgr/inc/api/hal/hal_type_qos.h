/**
*@file
*@brief This file defines qos related macro and struct  that used by hal interface
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
#ifndef _HAL_TYPE_QOS_H_
#define _HAL_TYPE_QOS_H_

#include "hal_type_common.h"
#include "hal_type_flow.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif 

/** defined in hal_itf_qos.c, used for check the traceLevel when print the trace*/
extern unsigned int g_hal_qos_trc_level;

/**@def HAL_QOS_DBGPRT(fmt, args...) 
*print the qos module debug trace*/
#define HAL_QOS_DBGPRT(fmt, args...)  if (g_hal_qos_trc_level >= 2) \
                                            printf("DBG %s:" fmt, __FUNCTION__, ##args);

#define HAL_QOS_ERRPRT(fmt, args...)  if (g_hal_qos_trc_level >= 0) \
                                            printf("ERR %s:" fmt, __FUNCTION__, ##args);

/**
*@def HAL_QOS_US
*define the  macro used for cofngiure upstream qos*/
#define HAL_QOS_US 0
/**
*@def HAL_QOS_DS
*define the  macro used for cofngiure downstream qos*/
#define HAL_QOS_DS 1

/*@def HAL_QOS_INVOLID_RATE_CONTROLLER_ID
*define the  macro used for involid rate controller id*/
#define  HAL_QOS_INVOLID_RATE_CONTROLLER_ID   255

/*@def HAL_QOS_INVOLID_ETHFLOW_ID
*define the  macro used for involid ethflow id*/
#define  HAL_QOS_INVOLID_ETHFLOW_ID           255

/*@def HAL_QOS_MAX_PQ_NUM
*define the maxinum queue nums used for queue */
#define HAL_QOS_MAX_PQ_NUM  512

/**
*@enum hal_qos_policy_t
*Define the qos policy values*/
typedef enum
{
    /** not configured any policy, in this case, the SP policy will be used as default*/                      
   HAL_NO_POLICY_E=0,
    /** Strict-Priority */                      
   HAL_SP_POLICY_E,
    /** weighted round-robin */                      
   HAL_WRR_POLICY_E
}hal_qos_policy_t;

/**
*@brief queue inforamtion
*
*Deinfe the structure priority queue information*/
typedef struct
{
    /** the qos policy type of the queue */                      
    hal_qos_policy_t  queue_qos_policy;
    /** the size of the queue */                      
    unsigned int          queue_size;
    /** the id of the queue */                      
    unsigned short          queue_id;
    /** the priority of the queue when use SP */                      
    unsigned short          priority;
     /** the tcont id that the queue used by */                      
    unsigned short          tcontid;
     /** 1 means many pq's share 1 buffer of maxQueueSize and 0 means each pq has individual buffer of maxQueueSize*/                      
    unsigned char           queue_cfg_opt;
     /** the traffic scheduler point */   
    unsigned short          tsched_ptr;
    /** the weight of the queue when use WRR */                      
    unsigned char           weight;
}hal_queue_t;

/**
*@brief qos policy
*
*Deinfe the structure used to configure qos policer*/
typedef struct
{
     /** the policer id  */                      
    //unsigned short policer_id;
    unsigned int policer_id;  //uint in SAL, make it consistent here
     /**enable or disable  the policer */                      	 
    int policer_en;
     /** Committed Information Rate */                      
    unsigned int cir;
     /** Committed Burst Size */                      
    unsigned int cbs;
     /** Excess Burst Size */                      
    unsigned int ebs;
     /** Peak Information Rate*/                      
    unsigned int pir;
     /** Peak Burst Size */                      
    unsigned int pbs;
     /** meter type(defined, but not used, can be removed later)*/                      
    unsigned char  meter_type;
}hal_qos_policer_t;

/**
*@brief tsched info
*
*Define the structure about tsched info when configured qos*/
typedef struct
{
  /** the tcont point  */  
  unsigned short   tcont_ptr; 
  /** the tsched point  */  
  unsigned short   tsched_ptr; 
} hal_qos_tsched_info_t;

/**
*@brief gemport info
*
*Deinfe the structure about gemport info when cofnigured qos*/
typedef struct
{
     /** the gemport id  */                      
    unsigned short gemport_id;
     /** the direction of gemport, up/down/both */                      
    int gemport_dir;
     /** the upstream policer  id  used by the gemport*/                      
    unsigned int us_policer_id;
     /** the downstream policer  id  used by the gemport*/                      
    unsigned int ds_policer_id;
     /** the tcont id for the upstream  */                      
    unsigned short tcont_id;
     /** the priority queue id  */                      
    unsigned int pq_id;
} hal_qos_gemport_info_t;

/**
*@brief upstream policy attribute
*
*Deinfe the structure about qos upstream policy attribute. It is similar with hal_qos_gemport_info_t,
need remove one later*/
typedef struct
{
    /* a index of gemport array, from pbit */
    unsigned int index;
    /** the  gemport id  */                      
    unsigned short gemport_id;
    /** the  policer  id  used by the gemport*/                      
    unsigned int policer_id;
    /** the tcont id for the upstream  */                      
    unsigned short tcont_id;
    /** the priority queue id  */                      
    //unsigned int pq_id;
    /** the priority of the queue */                      
    unsigned char  pri_level;
    /** the rate controller id */  
    unsigned int  rc_id;
    /** the maxinum of weight */  
    unsigned int  weight_max;
    /** the scheduling type for tcont e.g. SP, WRR, HYBRID */  
    unsigned int  traffic_mgmt;
    /* True:Configured, False:Not configured  */
    unsigned char gem_flag;   
    /** the info of priority queue, e.g. queue_id, weight and so on */  
    hal_queue_t pq_info;
} hal_qos_us_policer_attr_t;

/**
*@brief donwstream policy attribute
*
*Deinfe the structure about qos downstream policy attribute.*/
typedef struct
{
     /** the  gemport id  */                      
    unsigned short gemport_id;
     /** the  policer  id  used by the gemport*/                      
    unsigned int policer_id;
     /** the  ethnet flow id  */ 
    unsigned int ethflow_id;
    /** the  rate controller id  */ 
    unsigned int rc_id;
    /** the  uni port */ 
    unsigned char port;
     /** the  port type(SFU/HGU)*/                      
    unsigned int port_type;
} hal_qos_ds_policer_attr_t;

/**
*@brief gemport info
*
*Deinfe the structure about gemport*/
typedef struct
{
     /** the gemport id  */                      
    unsigned short gemport_id;
     /** the inforamtion of gemport when do qos*/                      	 
    hal_qos_gemport_info_t  gemport_info;
}hal_qos_gemport_info_db_t;

/**
*@brief traffic classify information
*
*Deinfe the structure about qos traffic classify. Only used by HGU with brdlt chipset now. Can be
delete later*/
typedef struct
{
     /** the information of structure header */                      
    hal_header_t halHeader;
     /** the uni port id the traffic classify configurd to */                      
    unsigned short portId;
     /** the vlan id  */                      
    unsigned short vlanId;
     /** the subnet id  */                      
    unsigned int subnetId;
     /** the ethernet flow id get from hal*/                      
    unsigned int flowId;
     /** the gem flow id,get  from hal */                      
    unsigned int gemFlowId;
     /** the pbit */                      
    unsigned int pBit;
      /** the ethernet */                      
   unsigned int ethType;
   /** If bVid is 1, means vlan is one of the condition when do traffic classify */
    unsigned char bVId;
   /** If bPBit is 1, means pbit is one of the condition when do traffic classify */
    unsigned char bPBit;
   /** If bEthType is 1, means ethernet type is one of the condition when do traffic classify */   
    unsigned char bEthType;
   /**Only used when bEthType is 1.If bEthTypeMissed is 1, mean configure the traffic classify rule
   for the packets without ethernet type header when use ethernet type to do traffic classify */   
    unsigned char bEthTypeMissed;
   /** the unmathc flow id in hal*/   
    unsigned short unmatchedFlowId;
   /**the qos method, if 1 then use qosMethod, else use pktBasedMethod (maybe not necessary,
   only one qosMethod is enough)*/   
    unsigned char bQosMethod;
   /** the value is only set by hal now, used for downsteam trafic classify method*/   
    unsigned int qosMethod;
    /** the value is only set by sal(HGU) now, used for downsteam trafic classify method*/   
   unsigned int pktBasedMethod;
} hal_qos_traffic_classify_t;


/**
*@brief upstream configuration infomation of wifi bridge mode
*
*Define the structure about wifi bridge mode.*/
typedef struct
{
    unsigned int        flow_id;
    unsigned short      gemport_id;
    unsigned int        rc_id;
    unsigned short      tcont_id;
    unsigned char       pri_level;
    unsigned short      lan_port;
	unsigned short      vlan_id;
    unsigned short      tpid;  
} hal_qos_us_wifi_bridge_info_t;

typedef struct
{
    unsigned int class_id;
    int enable;
    int order;
    //char lanif[32];
    int lan_port;
    char wanif[32];
    struct in_addr dest_ip;
    unsigned int dest_ip_mask;
    int dest_ip_exclude;
    struct in_addr src_ip;
    unsigned int src_ip_mask;
    int src_ip_exclude;
    int proto;
    int proto_exclude;	
    int dport;
    unsigned int dport_max;
    int dport_exclude;
    int sport;
    unsigned int sport_max;
    int sport_exclude;
    char smac[6];
    unsigned int smac_mask;
    char dmac[6];
    unsigned int dmac_mask;
    int priority;
    int dscp;
    int dscp_max;	
    int tos;
    int ether_type;
    hal_tr069_voip_info_t tr069_voip_classify;
}hal_qos_classify_t; 

typedef struct
{
    unsigned int class_id;
    int enable;
    int lan_port;
    struct in6_addr *dest_ip;
    struct in6_addr *src_ip;
    int proto;
    int dport;
    int tc;
    int vlan;
    int pri;	
}hal_v6_qos_classify_t; 

typedef struct
{
    unsigned int flow_id;
    int class_id;
    int policer_id;
    int queue_id;
    unsigned int dscp_remark_enable;
    unsigned int dscp_remark_value;
    unsigned int pri_remark_enable;
    unsigned int pri_remark_value;	
}hal_qos_flow_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 

#endif

