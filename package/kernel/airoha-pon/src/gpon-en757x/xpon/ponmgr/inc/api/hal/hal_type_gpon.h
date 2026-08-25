/**
*@file
*@brief This file defines GPON MAC related macro and struct  that used by hal interface
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

#ifndef HAL_TYPE_GPON_H_
#define HAL_TYPE_GPON_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#include "hal_type_common.h"
#include "hal_type_qos.h"


/**
*@def HAL_SN_STATUS_ON
*define the  macro used indicate sn is enable*/
#define HAL_SN_STATUS_ON 0
/**
*@def HAL_SN_STATUS_OFF
*define the  macro used indicate sn is disable*/
#define HAL_SN_STATUS_OFF 1

/** defined in hal_itf_gpon.c, used for check the traceLevel when print the trace*/
extern unsigned int g_hal_pon_trc_level;

/**@def HAL_GPON_DBGPRT(fmt, args...) 
*print the GPON module debug trace*/
#define HAL_GPON_DBGPRT(fmt, args...) if (g_hal_pon_trc_level > 0) \
                                printf("DBG %s:" fmt, __FUNCTION__, ##args);

#define HAL_GPON_ERRPRT(fmt, args...) if (g_hal_pon_trc_level >= 0) \
                                printf("ERR %s:" fmt, __FUNCTION__, ##args);

/**
*@brief fec status
*
*Deinfe the structure used to indicate transmit/receive fec is enable or not*/
typedef struct
{
   /** Fec in downstream is enable or not.If 1 means enable, else disable*/
    unsigned int rx;
   /** Fec in upstream is enable or not.If 1 means enable, else disable*/
    unsigned int tx;
} hal_pon_fec_status_t;

/**
*@enum hal_pon_link_opt_status_t
*Define the ploam link operate status*/
typedef enum
{
    /** ploam in O1 state, the link is loss*/
    HAL_LINK_OPERATION_STATE_IINIT_O1_E=0,
    /** ploam in O2 state, the link is active, and it is standby now */
    HAL_LINK_OPERATION_STATE_STANDBY_O2_E,
    /** ploam in O3 state, the link is ok, sn is sent by ONU*/
    HAL_LINK_OPERATION_STATE_SN_O3_E,
    /** ploam in O4 state, the onu id is arranged by olt and ranging is onging*/
    HAL_LINK_OPERATION_STATE_RANGING_O4_E,
    /** ploam in O5 state, in the working state*/
    HAL_LINK_OPERATION_STATE_OPERATION_O5_E,
    /** ploam in O6 state, the link is popup*/
    HAL_LINK_OPERATION_STATE_POPUP_O6_E,
    /** ploam in O7 state, the link is emergency stop*/
    HAL_LINK_OPERATION_STATE_EMERGENCY_STOP_O7_E
}hal_pon_link_opt_status_t;

/**
*@enum hal_pon_link_adm_status_t
*Define the ploam link operate status*/
typedef enum
{
    /** the ploam link administrative state is ready*/
    HAL_LINK_ADMINISTRATIVE_STATE_READY_E=1,
    /** the ploam link administrative state is inactive*/
    HAL_LINK_ADMINISTRATIVE_STATE_INACTIVE_E,
    /** the ploam link administrative state is active*/
    HAL_LINK_ADMINISTRATIVE_STATE_ACTIVE_E
}hal_pon_link_adm_status_t;

/**
*@brief pon link
*
*Deinfe the structure about the pon link inforamtion*/
typedef struct
{
    /** the hal header*/
    hal_header_t halHeader;
    /** the pon link administrative status*/
    hal_pon_link_adm_status_t status;    
    /** the pon link operate statusr*/
    hal_pon_link_opt_status_t subStatus; 
    /** the serial no used when onu register*/
    unsigned char* serialNo;
    /** the password used for SLID register*/
    unsigned char* password;
    /** the init pon link operate status*/
    hal_pon_link_opt_status_t initState;
} hal_pon_link_t;

/**
*@def HAL_OMCI_BUFF_LEN
*define the  macro about the max length of omci message*/
#define HAL_OMCI_BUFF_LEN 2048


/**
*@brief omci message
*
*Deinfe the structure when receive/send the omci message*/
typedef struct
{
    /** the crc header of omci message*/
    unsigned int crc;
    /** the length of omci message*/
    unsigned short mesLen;
    /** the data of omci message*/
    unsigned char Omcibuff[HAL_OMCI_BUFF_LEN];
} hal_write_omci_t;

/**
*@enum hal_eth_filter_port_type_t
*Define the ethernet filter port id*/
typedef enum
{
    /** the min value of the port*/
    FILTER_PORT_MIN_E,
     /** the ethernet port 1*/
    FILTER_PORT_EMAC0_E = FILTER_PORT_MIN_E,
     /** the ethernet port 2*/
    FILTER_PORT_EMAC1_E,
     /** the ethernet port 3*/
    FILTER_PORT_EMAC2_E,
     /** the ethernet port 4*/
    FILTER_PORT_EMAC3_E,
     /** the ethernet port 5*/
    FILTER_PORT_EMAC4_E,
     /** the filter is for the bridge*/
    FILTER_PORT_BRIDGE_E,
    /** the max value, never used*/
    FILTER_PORT_MAX_E,

    FILTER_PORT_WAN_0 =  8,
 /* Filter Set WAN subnet 1 */
    FILTER_PORT_WAN_1 = 9,
 /* Filter Set WAN subnet 2 */
    FILTER_PORT_WAN_2 = 10,
 /* Filter Set WAN subnet 3 */
    FILTER_PORT_WAN_3 = 11,
 /* Filter Set WAN subnet 4 */
    FILTER_PORT_WAN_4 = 12,
 /* Filter Set WAN subnet 5 */
    FILTER_PORT_WAN_5 = 13,
 /* Filter Set WAN subnet 6 */
    FILTER_PORT_WAN_6 = 14,
 /* Filter Set WAN subnet 7 */
    FILTER_PORT_WAN_7 = 15



}hal_eth_filter_port_type_t;

typedef enum
{
    /* Local DHCP filter LAN side */
    DHCP_FILTER_LAN,
    /* Local DHCP filter WAN side */
    DHCP_FILTER_WAN
} hal_dhcp_filter_id_t;

typedef enum
{
    /* Disable DHCP filter */
    DHCP_FILTER_DISABLE,
    /* Enable DHCP filter */
    DHCP_FILTER_ENABLE
} hal_dhcp_filter_action_t;


/**
*@enum hal_eth_filter_action_t
*Define the traffic filter action*/
typedef enum
{
    /** the filter action is forward the packet by the traffic classify rule*/
    FILTER_ACTION_FWD_E =0, 
    /** the filter action is forward the packet to CPU*/
    FILTER_ACTION_CPU_E = 1,
    FILTER_ACTION_DROP_E = 2,
    /** to disable the filter */
    FILTER_ACTION_DISABLE_E = 3, 
    /** the max value, never used*/
    FILTER_ACTION_MAX_E
}hal_eth_filter_action_t;

/**
*@brief ethernet type filter
*
*Define the structure used for ethernet type filter*/
typedef struct
{
    /** the uni port for filter*/
    unsigned short phyPort;
    /** the ethernet filter header*/
    unsigned short ethType;
    /** the ethernet filter action*/
    hal_eth_filter_action_t filterCmd;
    /** the flow id in hal, should remove from this structure later,sal not know the hal flow id */
    unsigned int ethFlowId;

    hal_port_info_t hal_port_info;

}hal_cfg_eth_type_filter_t;

/* Structure used for upstream DHPC filter */
typedef struct
{
    /** filter id: lan or wan */
    unsigned int filterId;
    /** filter action: enable or disable */
    unsigned int filterAct;
} hal_cfg_dhcp_filter_t;

/**
*@enum hal_gemport_type_t
*Define the gemport attribute*/
typedef enum
{
    /** the gemport is used for unicast*/
    GEMPORT_TYPE_UNICAST_E = 0,
    /** the gemport is used for broadcast*/
    GEMPORT_TYPE_BROADCAST_E,
    /** the gemport is used for multicast*/
    GEMPORT_TYPE_MULTICAST_E,
    /** the gemport is used for omci*/
    GEMPORT_TYPE_OMCI_E,
    /** the max value, never used*/
    GEMPORT_TYPE_MAX_E
}hal_gemport_type_t;

typedef enum
{
    NNI_OP_MODE = 0,
    UNI_OP_MODE = 0x10,
    INVALID_MODE = 0xFF
}hal_op_mode_t;

#define INVALID_GEMID 0xFFFF
#define BC_GEMID 2046

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


