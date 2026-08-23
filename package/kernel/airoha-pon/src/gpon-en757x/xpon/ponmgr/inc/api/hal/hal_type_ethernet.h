/**
*@file
*@brief This file defines ethernet port related macro and struct  that used by hal interface
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

#ifndef _HAL_TYPE_ETHERNET_H_
#define _HAL_TYPE_ETHERNET_H_


#include "hal_type_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif 

/**
*@brief hal forward mask
*
*Deinfe the mask of foward or not inter uni ports*/
typedef struct
{
   /** the id of the uni port*/   
    unsigned int portIndex;
   /** the new mask of the forward uni port inter networking*/   
    unsigned int newFwdMask;
   /** the oldmask of the forward uni port inter networking*/   
    unsigned int oldFwdMask;
} hal_fwd_port_mask_t;

/**
*@brief hal ethernet flow struct
*
*Deinfe the struct used when create ethernet flow.Only used by brdlt HGU and HAL internal.
Brdlt HGU should also not use such struct later, because in such struct, it defined may parameters 
which only known by HAL only*/
typedef struct
{
   /** the header information of the struct*/
    hal_header_t halHeader;
   /** the id in hal about vlan action*/   
    unsigned int vlanActionId;
   /** the id in hal about ethernet flow*/   
    unsigned int flowId;
   /** the mappering method of gemflow and ethernet flow.Support PBITMapper and flow base*/      
    unsigned int mappingMethod;
   /** the id in hal about gemflow, only valid when use Flow base method*/      
    unsigned int gemFlowId;
   /** the id in hal about gemflow table, only valid when use PBITMapper method*/      
    unsigned int gemTableId;
   /** the id of the uni port*/   
    unsigned short portId;
} hal_eth_flow_t;

/**
*@enum hal_lan_port_t
*
*Define all the uni ports*/
#if 0
typedef enum
{
    HAL_LAN_MIN_E = 0,
    /** uni port 1*/
    HAL_LAN0_E = HAL_LAN_MIN_E,		
    /** uni port 2*/
    HAL_LAN1_E ,
    /** uni port 3*/    
    HAL_LAN2_E,
    /** uni port 4*/
    HAL_LAN3_E,
    /** uni port 5*/
    HAL_LAN4_E,
    /** uni port 6*/
    HAL_LAN5_E,
    HAL_LAN6_E,
    HAL_LAN7_E,   
    HAL_LAN_MAX_E = HAL_LAN7_E+1,   
    HAL_LAN_PCI_E = HAL_LAN_MAX_E+1,    

    HAL_WIFI_MIN_E=HAL_LAN_PCI_E+1,
    HAL_WIFI0_E = HAL_WIFI_MIN_E,
    HAL_WIFI1_E,
    HAL_WIFI2_E,
    HAL_WIFI3_E,
    HAL_WIFI_MAX_E=HAL_WIFI3_E
    
}hal_lan_port_t;
#endif
/**
*@enum hal_phy_speed_t
*
*Define the speed mode of the phy*/
typedef enum
{
     /** the phy speed is 10M*/
    HAL_SPEED_10M_E = 0,
     /** the phy speed is 100M*/
    HAL_SPEED_100M_E,
     /** the phy speed is 1000M*/
    HAL_SPEED_1000M_E,
     /** the phy speed is Auto, it is defined by negotiate */
    HAL_SPEED_AUTO_E,
     /** the phy is down*/
    HAL_SPEED_DOWN_E,
     /** the phy is in loop mode*/
    HAL_SPEED_LOOP_E,
     /** the phy speed is 10M or 100M*/
    HAL_SPEED_10_100M_E
}hal_phy_speed_t;

/**
*@enum hal_phy_mode_t
*
*Define the transfer mode of the phy*/
typedef enum
{
     /** the phy transfer mode  is half duplex*/
    HAL_HALF_DUPLEX_E = 0,
     /** the phy transfer mode  is full duplex*/
    HAL_FULL_DUPLEX_E,
     /** the phy transfer mode  is auto duplex, use full or half duplex defined by negotiate*/
    HAL_AUTO_DUPLEX_E
}hal_phy_mode_t;

/**
*@enum hal_phy_status_t
*
*Define the operate state of the phy*/
typedef enum
{
     /** the phy operate state is down*/
    HAL_LINK_DOWN_E = 0,
     /** the phy operate state is up*/
    HAL_LINK_UP_E
}hal_phy_status_t;

/**
*@brief  lan atrribute struct
*
*Deinfe the struct of the lan information, include speed, mode and operate status*/
typedef struct 
{
   /** the speed of the uni port*/   
    hal_phy_speed_t  	speed;
   /** the transfer mode of the uni port*/   
    hal_phy_mode_t  	mode;
   /** the operate status of the uni port*/   
    hal_phy_status_t        status;
}hal_lan_attr_t;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif




