/**
*@file
*@brief This file defines common type related macro and struct  that used by hal interface
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
#ifndef HAL_TYPE_COMMON_H_
#define HAL_TYPE_COMMON_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

typedef unsigned short UserPortId;

/**
*@def HAL_COM_DBGPRT(fmt, args...) 
*common debug print macro
*/
#define HAL_COM_DBGPRT(fmt, args...) printf(fmt, ##args);
/**
*@def HAL_COM_ERRPRT(fmt, args...) 
*common error print macro
*/
#define HAL_COM_ERRPRT(fmt, args...) printf("ERR %s:" fmt, __FUNCTION__, ##args);

/**
*@def MAC_ADDR_LEN
*Define the mac address length
*/
#define MAC_ADDR_LEN 	6

/**
*@def HAL_LAYER_MASK_FILTER_L2
*Define the mask of Layer 2 traffic fiter
*/
#define HAL_LAYER_MASK_FILTER_L2 0x00000001

/**
*@def HAL_LAYER_MASK_FILTER_L3
*
*Define the mask of Layer 3 traffic fiter
*/
#define HAL_LAYER_MASK_FILTER_L3 0x00000002

/**
*@def HAL_LAYER_MASK_ACTION_L2
*
*Define the mask of layer 2 traffic action
*/
#define HAL_LAYER_MASK_ACTION_L2 0x00000004

/**
*@def HAL_LAYER_MASK_ACTION_L3
*Define the mask of layer 3 traffic action
*/
#define HAL_LAYER_MASK_ACTION_L3 0x00000008

/**
*@def P_BIT_NUMBER
*Define the max p-bit number
*/
#define P_BIT_NUMBER    8

#define SAND_HAL_TYPE_INVALID_TM_QUEUE_ID   (0xFF)


/**
* Define ethertype 
*/
#define HAL_ETHERNET_TYPE_IPoEv4        0x0800
#define HAL_ETHERNET_TYPE_IPoEv6        0x86DD
#define HAL_ETHERNET_TYPE_ARP           0x0806
#define HAL_ETHERNET_TYPE_1588          0x88F7
#define HAL_ETHERNET_TYPE_802_1X        0x888E
#define HAL_ETHERNET_TYPE_801_1AG_CFM   0x8902


#define GEMPORT_UNMATCH_VLAN_ID     4567

#define HGU_GPON0_DEV_NAME		 "gpon0"
#define SFU_WAN0_DEV_NAME		 "wan0"


/**
*@enum hal_ret_t
*
*Deinfe the return value of hal module
*/
typedef enum
{
     /** HAL return value: success*/
    HAL_RET_SUCCESS = 0,
     /** HAL return value: failed*/    
    HAL_RET_FAILED
}hal_ret_t;

/**
*@enum hal_bool_t
*
*Deinfe the bool in hal, because  some C style application not support "bool"
*/
typedef enum
{
   /** HAL boolean value: false*/
  HAL_FALSE_E  = 0,
   /** HAL boolean value: true*/
  HAL_TRUE_E =1
} hal_bool_t ;

/**
*@enum hal_olt_type_t
*
*Deinfe the OLT types of ALU
*/
typedef enum
{
   /** Define the ALU OLT Type:7342/FTTU*/
    HAL_FTTU_BASED_E= 1,
   /** Define the ALU OLT Type:7360/FX*/    
    HAL_ISAM_BASED_E = 2
}hal_olt_type_t;

/**
*@enum hal_port_type_t
*
*Deinfe the port domain of the ONU
*/
typedef enum
{
   /** Define the each port type of the ONU, SFU means managed by OMCI*/
    HAL_SFU_DOMAIN_E,
   /** Define the each port type of the ONU, HGU means managed by TR069*/    
    HAL_HGU_DOMAIN_E
}hal_port_domain_mode_t;

/**
*@enum hal_port_class_t
*
*Deinfe the port type of the ONU
*/
typedef enum
{
    /**eth  port*/
    HAL_PORT_ETH_CLASS_E,   
    /**wifi port*/
    HAL_PORT_WIFI_CLASS_E,
    /**pots*/
    HAL_PORT_POTS_CLASS_E,
    /**moca,suchh 821*/
    HAL_PORT_MOCA_CLASS_E,
    /**switch,such as 881*/
    HAL_PORT_SWITCH_CLASS_E,
    /**ds1*/
    HAL_PORT_DS1_CLASS_E,
    /**default*/
    HAL_PORT_UNKNOWN_CLASS_E,
}hal_port_class_t;

/**
*@def HAL_PORT_MAX_PER_CLASS
*@todo cap get from platform
*/
#define HAL_PORT_MAX_PER_CLASS 4


/**
*@port info
*
*Define the structure of port info*/
typedef struct
{
    /** the port id of the traffic*/
    UserPortId port_id;
    /** the port type*/
    hal_port_class_t port_class;
    /** the port type*/
    hal_port_domain_mode_t port_domain;
    
}hal_port_info_t;


typedef enum
{
    /** It is used for no policy */
    HAL_TRAFFIC_MGMT_NO_POLICY_E=0,
    /** It is used for strict priority scheduling */
    HAL_TRAFFIC_MGMT_SP_E,
    /** It is used for weight round-robin scheduling */
    HAL_TRAFFIC_MGMT_WRR_E,
    /** It is used for hybrid(sp+wrr) scheduling */
    HAL_TRAFFIC_MGMT_HYBRID_SP_AND_WRR_E
}hal_qos_schedule_type_t;

/**
*@enum hal_mgmt_domain_t
*
*Define the port managed by which way, TR069 or OMCI
*/
typedef enum 
{ 
     /** Min value, not used*/
    HAL_DOMAIN_MIN_E = 0,
    /** UNKOWN managed domain, initialize value*/
    HAL_DOMAIN_UNKNOWN_E = HAL_DOMAIN_MIN_E,  
    /** ANI means the port managed by OMCI*/
    HAL_DOMAIN_ANI_E,   
     /** UNI means the port managed by TR069*/
    HAL_DOMAIN_UNI_E,  
    /** DMM means the port managed by TR069 and OMCI*/
    HAL_DOMAIN_DMM_E, 
    /** Max value, not used*/
    HAL_DOMAIN_MAX_E 
}hal_mgmt_domain_t ; 

/**
*@enum hal_sec_classification_method_t
*
*Deinfe the classfication moethod*/
typedef enum
{
    /** Use Ethernet Type to do classify*/
    HAL_SEC_CLASSIFICATION_METHOD_ETYPE_E = 0, 
    /** Use wan subnet  to do classify*/    
    HAL_SEC_CLASSIFICATION_METHOD_WAN_SUBNET_E,
     /** Use wan subnet and the inner vid  to do classify*/      
    HAL_SEC_CLASSIFICATION_METHOD_WAN_SUBNET_INNER_VID_E,
     /** Use wan  inner priority  to do classify*/          
    HAL_SEC_CLASSIFICATION_METHOD_INNER_PBIT_E,
     /** Use wan  inner vid  to do classify*/              
    HAL_SEC_CLASSIFICATION_METHOD_INNER_VID_E
}hal_sec_classification_method_t;


/**
*@enum hal_direction_t
*
*Deinfe the dirction of the traffic.But now there are some value also include traffic classification method,
such values should be moved to correct place*/
typedef enum
{
     /** the traffic dirction is up*/          
    HAL_UP_E = 0,
     /** the traffic dirction is down*/              
    HAL_DOWN_E,
     /** the traffic dirction is up, and this is the default traffic(used in brdlt hal internal now, Should be removed later)*/                  
    HAL_UP_DEF_E,
     /** the traffic dirction is down, and this is the default traffic(Should be removed later)*/                      
    HAL_DOWN_DEF_E,
     /** used for traffci classification only ,should be removed later*/                      
    HAL_UP_OUTER_E,
     /** used for traffci classification only ,should be removed later*/                          
    HAL_UP_OUTER_DEF_E,
     /** used for traffci classification only ,should be removed later*/                          
    HAL_UP_OUTER_WAN_SUBNET_E,
     /** used for traffci classification only ,should be removed later*/                          
    HAL_UP_OUTER_INNER_DEF_E,
    /** invalid */     
    HAL_DIRECTION_INVALID
}hal_direction_t;

/**
*@enum hal_tag_mode_t
*
*Deinfe the tag mode of the traffic rule*/
typedef enum
{
     /** the traffic rule is used for untag packets*/                      
     HAL_NO_TAG_E = 0x0,
     /** the traffic rule is used for single tag packets*/                           
     HAL_SINGLE_TAG_E = 0x1,
      /** the traffic rule is used for double tag packets*/                           
    HAL_DOUBLE_TAG_E = 0x2
}hal_tag_mode_t;

/**
*@brief driver file fd
*
*Deinfe the manage domain and the ioctl file description*/
typedef struct 
{ 
     /** the mgnt domain */                      
    hal_mgmt_domain_t halDrvDomain; 
     /** the ioctl file description*/                      	 
    int halDrvFd; 
} hal_drv_file_t;

/**
*@enum hal_action_t
*
*Deinfe the action type in hal*/
typedef enum
{
     /** the action is add, add the object*/                      
    HAL_ADD_E = 0,
     /** the action is modified, modify the object*/                      
    HAL_MODIFY_E,
     /** the action is remove, remove the object*/                      
    HAL_REMOVE_E,
     /** the action is configure(need check the difference with add/modify action)*/                      
    HAL_CFG_E,
     /** the action is map, map two or more objects*/                      
    HAL_MAP_E,
     /** the action is add get, get the inforamtion from hal*/                      
    HAL_GET_E,
     /** the action is set(need check the difference with add/modify/cfg action)*/                      
    HAL_SET_E
}hal_action_t;

/**
*@brief hal common header
*
*Deinfe the common header in hal*/
typedef struct
{
     /**the direction of the traffic*/                      
    hal_direction_t halDirection;
     /**the action want to do for the API */                      	 
    hal_action_t halAction;
    /**the domain name and ioctl fd of the API*/                      	 
    hal_drv_file_t halDrvFile;
} hal_header_t;

/**
*@brief tag information
*
*Deinfe the vlan informtion structure in hal*/
typedef struct
{
     /** the pbit of the tag  */                      
    unsigned short pbit;
     /** the vlan id of the tag  */                      	 
    unsigned int vlan;
}hal_tag_t;

/**
*@brief onu hardware information
*
*Deinfe the onu support LAG, Switch informtions  in hal*/
typedef struct
{
    /** the board support LAG mode for ethport or not */                      
    hal_bool_t supportLAG;

    /** the board support switch chipset or not  */                      	 
    hal_bool_t supportSwitch;	 
}hal_onu_hw_mode;


#define HAL_MAX_IFNAME_LENGTH 64

#define HAL_GPON0_DEV_NAME		 "gpon0"
#define HAL_WAN0_DEV_NAME		 "wan0"
#define HAL_GPON0_ANI_DEV_NAME	 "gpon0.ani"
#define HAL_VEIP_DEV_NAME		 "veip0"
#define HAL_MCAST_DEV_NAME		 "gponm"
#define HAL_SSH_DEV_NAME           "pon_d4097"
#define HAL_VEIP_PORT_ID 9

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 
#endif
