/**
*@file
*@brief This file defines security related macro and struct  that used by hal interface
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

#ifndef _HAL_TYPE_SECURITY_H_
#define _HAL_TYPE_SECURITY_H_

#include "hal_type_common.h"

/*Define max Number ingress for per-port*/
#define MAXNUMOFINGRESS    4

/*Define ingress_class portId map for per-port*/
#define INGRESSCLASS(port_id)					((port_id)*MAXNUMOFINGRESS)
#define VLANINGRESSCLASS(port_id)				((port_id)*MAXNUMOFINGRESS+1)
#define VLANNUMINGRESSCLASS(port_id)			((port_id)*MAXNUMOFINGRESS+2)
#define IPINGRESSCLASS(port_id)				((port_id)*MAXNUMOFINGRESS+3)
#define MACINGRESSCLASS(port_id)              VLANNUMINGRESSCLASS(port_id)
#define IPVLANINGRESSCLASS(port_id)			INGRESSCLASS(port_id)
#define IPMACINGRESSCLASS(port_id)			INGRESSCLASS(port_id)
/*
*port*maxnumofingress == ip+vlan
*port*maxnumofingress + 1  == disable vlan     ACL_VLAN_E
*port*maxnumofingress + 2  == vlan_num        ACL_VLAN_NUM
*port*maxnumofingress + 3 == ip
*/
/**
*@enum hal_acl_mode_t
*
*Define filer mode of acl function*/
typedef enum 
{
    /** support ip filter mode for acl*/
    ACL_IP_E = 0,
    /** support mac filter mode for acl*/
    ACL_MAC_E,
    /** support ip plus mac  filter mode for acl*/
    ACL_IP_MAC_E,
    /** support ip plus vlan filter mode for acl*/
    ACL_IP_VLAN_E,
    /** support MAC group filter mode for acl*/
    ACL_MAC_OUI_E,
    /** support vlan filter mode for acl */
    ACL_VLAN_E,
    /** support vlan number filter mode for acl */
    ACL_VLAN_NUM_E,
}hal_acl_mode_t;

/**
*@enum hal_acl_policy_t
*
*Define the policy mode of acl function*/
typedef enum
{
    /** use white list, only the packets meet the rules in the list can pass through to network*/
    WHITE_LIST_E = 0,
    /** use black list, only the packets not meet the rules in the list can pass through to network*/
    BLACK_LIST_E,
}hal_acl_policy_t;

/**
*@brief  ip plus mac acl
*
*Deinfe the structure that used do ip plus mac acl function*/
typedef struct
{
    /** the ip address used to do acl*/
    unsigned int ip;
    /** the mac address used to do acl*/
    unsigned char mac[6];
}ip_mac_t;
/**
*@brief  ip plus vlan acl
*
*Deinfe the structure that used do ip plus vlan acl function*/
typedef struct
{
    /** In ip + vlan inclusive mode, OLT can disable antispoof for a specifying flow / CVlan.
     ** If cvlanAntispoofDisable equals non-zero, ip address should be ignored. The packet with
     ** the vlan should pass 
     */
    unsigned char cvlanAntispoofDisable;
    /** the ip address used to do acl*/
    unsigned int ip;
    /** the vlan id used to do acl*/
    unsigned int vlan;
    /** the vlan number used to do acl*/
    unsigned int vlan_num;    
}ip_vlan_t;

/**
*@brief acl datal
*
*Deinfe the structure that used do acl filter rule*/
typedef struct
{
   /** the filter mode of acl functionl*/
    hal_acl_mode_t mode;
   /**Deinfe the structure used for the  filter conditions*/
    union 
    {
       /** the ip address when do ip filter*/
        unsigned int ip;
       /** the mac address when do mac filter*/
        unsigned char mac[6];
       /** the ip and mac address when do ip plus mac filter*/
        ip_mac_t ip_mac;
       /** the ip and vlan address when do ip plus vlan filter*/
        ip_vlan_t ip_vlan;
    }u;
}hal_acl_data_t;
/**
*@brief acl structure
*
*Deinfe the structure that used do acl fucntion*/
typedef struct 
{
    /** the uni port info that the rule configured to*/
    hal_port_info_t hal_port_info;
    /** the policy that the acl function is used*/
    hal_acl_policy_t policy;
    /** the index of such rule*/
    int index;
    /** the detail information  of the rule*/
    hal_acl_data_t data;
}hal_acl_t;

#endif
