/**
*@file
*@brief This file defines misc related macro and struct  that used by hal interface
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

#ifndef HAL_TYPE_MISC_H_
#define HAL_TYPE_MISC_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#include "hal_type_common.h"


/**
*@def HAL_CFM_BUFF_LEN
*define the  macro of the max size of cfm message*/
#define HAL_CFM_BUFF_LEN  2048
#define IF_NAME_LEN_MAX 16

/**
*@brief cfm message
*
*Deinfe the structure of cfm message*/
typedef struct
{
    /** the length of cfm message*/
    unsigned int mesLen;
    /** the data of cfm message*/
    unsigned char Cfmbuff[HAL_CFM_BUFF_LEN];
} hal_write_cfm_t;

/**
*@brief gpon capability
*
*Deinfe the structure of onu capability of the resource*/
typedef struct
{
    /**The number of T-CONT's available*/
    unsigned short totalTcontNum;
    /**The number of gem ports available.Equivalent to today's numPhyPortsAvail*/
    unsigned short totalGemPortNum;
    /**The number of upstream priority queue available*/
    unsigned short totalUSPQNum;
    /**The number of priority queue available per T-CONT*/
    unsigned short numPqPerTcont;
    /**The number of priority queue available per UNI*/
    unsigned short numPqPerUni;
    /**The number of traffic scheduler available*/
    unsigned short totalTSchedNum;
} hal_gpon_conf_info_t;

typedef struct
{
    char ifName[IF_NAME_LEN_MAX];
    /* Refer net_device_stats */
    struct {
        unsigned int rxPackets;
        unsigned int rxBytes;
        unsigned int rxErrors;
        unsigned int rxDropped;
        unsigned int rxCrcErrors;
        unsigned int rxLenErrors;
        unsigned int rxOverErrors;
        unsigned int rxFrameErrors;
		unsigned int rxFifoErrors;
        unsigned int rxMissedErrors;
        unsigned int muticast;
        unsigned int txPackets;
        unsigned int txBytes;
        unsigned int txErrors;
        unsigned int txDropped;
		unsigned int txFifoErrors;
        unsigned int txAbortedErrors;
        unsigned int txCarrierErrors;
        unsigned int txHbErrors;
        unsigned int txWinEerrors;
    } stats;
    //struct net_device_stats stats;
}hal_wan_netdev_stats_t;
/**
*@enum hal_write_ploam_type_t
*Define the type of ploam message*/
typedef enum
{
    HAL_WRITE_PLOAM_TYPE_MIN,
    HAL_WRITE_PLOAM_TYPE_PEE,
    HAL_WRITE_PLOAM_TYPE_DYING_GASP,
    HAL_WRITE_PLOAM_TYPE_PP,
    HAL_WRITE_PLOAM_TYPE_MAX
} hal_write_ploam_type_t;

/**
*@brief the type of ploam message
*
*Deinfe the structure of ploam message type*/
typedef struct
{
    hal_write_ploam_type_t messagetype;
} hal_write_ploam_t;

/**
*@brief MSG_RTP_GEMFLOW_NOTIFY_TO_OMCILIB message
*
*Deinfe the structure of RTP gemflow notify message*/
#define MAX_IPC_MSG_SIZE  16384
#define VPP_EVENT_IPC_QUEUE_FILE "/configs/alcatel/config/vpp_event"
#define MSG_RTP_GEMFLOW_NOTIFY_TO_OMCILIB            8
#define INVALID_IPC_QID ((unsigned long)(-1))
#define VLANMGR_MAIN_Q_ID 0x000C0000

typedef struct
{
    unsigned short   gemflowId;
} hal_voipRtpGemflowNotify_t;

typedef struct {
    unsigned int msg_id;
    unsigned short orig_mod_id;
    unsigned short dest_mod_id;
    char ack_required;
    unsigned char priority;
    unsigned short length;
    char bcast;
    unsigned char dummy;
    unsigned int src_qid;
    unsigned int txn_id;
    signed int dialog_id;
    unsigned char session_state;
} hal_voipExtMsgHeader_t;

typedef struct
{
  unsigned short gemCtpPort0;
  unsigned short gemCtpPort1;
  unsigned short gemCtpPort2;
  unsigned short gemCtpPort3;
  unsigned short gemCtpPort4;
  unsigned short gemCtpPort5;
  unsigned short gemCtpPort6;
  unsigned short gemCtpPort7;
} hal_priorityPortMapping_t;

typedef enum {
    HAL_VOIP_RULE_DIRECTION_RX=0,
    HAL_VOIP_RULE_DIRECTION_TX,
    HAL_VOIP_RULE_DIRECTION_MAX,
} hal_voip_rule_direction_t;

typedef struct
{
   hal_voip_rule_direction_t  tableDir;
   unsigned int nbrOfTags;
   unsigned int tagRuleId;
   char ifname[32];
}hal_voip_rule_node_info_t;

typedef struct
{
   hal_priorityPortMapping_t map;
   unsigned short cvlan;
}hal_voip_gem_cvlan_info_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

