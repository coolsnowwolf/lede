/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	omci_external_msg.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/6/28	Create
*/

#ifndef _OMCI_EXTERNAL_MSG_H
#define _OMCI_EXTERNAL_MSG_H

/* OMCI message type define */
#define MT_OMCI_MSG_TYPE_CREAT                  4
#define MT_OMCI_MSG_TYPE_DELETE                 6
#define MT_OMCI_MSG_TYPE_SET                    8
#define MT_OMCI_MSG_TYPE_GET                    9
#define MT_OMCI_MSG_TYPE_GET_ALL_ALARMS         11
#define MT_OMCI_MSG_TYPE_GET_ALL_ALARMS_NEXT    12
#define MT_OMCI_MSG_TYPE_MIB_UPLOAD             13
#define MT_OMCI_MSG_TYPE_MIB_UPLOAD_NEXT        14
#define MT_OMCI_MSG_TYPE_MIB_RESET              15
#define MT_OMCI_MSG_TYPE_ALARM                  16
#define MT_OMCI_MSG_TYPE_AVC                    17
#define MT_OMCI_MSG_TYPE_TEST                   18
#define MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD      19
#define MT_OMCI_MSG_TYPE_DOWNLOAD_SEC           20
#define MT_OMCI_MSG_TYPE_END_SW_DOWNLOAD        21
#define MT_OMCI_MSG_TYPE_ACTIVE_SW              22
#define MT_OMCI_MSG_TYPE_COMMIT_SW              23
#define MT_OMCI_MSG_TYPE_SYNC_TIME              24
#define MT_OMCI_MSG_TYPE_REBOOT                 25
#define MT_OMCI_MSG_TYPE_GET_NEXT               26
#define MT_OMCI_MSG_TYPE_TEST_RST               27
#define MT_OMCI_MSG_TYPE_GET_CURR_DATA          28
#define MT_OMCI_MSG_TYPE_SET_TABLE              29
#define MT_OMCI_MSG_TYPE_MAX                    30

#define MT_OMCI_MSG_TYPE_SW_TIMEOUT             255

/* OMCI IPC command Type */
#define OMCI_IPC_CMD_TYPE_ME_CMD            1  //internal use
#define OMCI_IPC_CMD_TYPE_DEBUG_LEVEL       2  //internal use
#define OMCI_IPC_CMD_TYPE_DUMP_ALL_ME       3  //internal use
#define OMCI_IPC_CMD_TYPE_DUMP_INST         4  //internal use
#define OMCI_IPC_CMD_TYPE_TERMINATE_APP     5  //internal use
#define OMCI_IPC_CMD_TYPE_MIB_RESET         6  //internal use




#ifndef GNU_PACKED
#define GNU_PACKED __attribute__ ((packed))
#endif//GNU_PACKED


#define OMCI_BASELINE_PKT_LEN  48
#define OMCI_BASELINE_CONT_LEN  32
#define OMCI_BASELINE_MSG  0x0A
#define OMCI_EXTENED_MSG  0x0B

#define OMCI_PROJID 9
#define OMCI_MQ_FLAG_PATH "/tmp/omci/omci_cmd_mq" 

//  GNU_PACKED omciBaselinePayload_s
typedef struct GNU_PACKED omciBaselinePayload_s{
	short tranCorrelatId;/* transaction correlation identifier */
	char msgType;/*message type*/
	char devId;/*device ID*/
	long meId;/*ME ID*/
	char msgContent[OMCI_BASELINE_CONT_LEN];/* pointer to message content 32 byte*/
	char trailer[8];//OMCI trailer
}omciBaselinePayload_t,*omciBaselinePayload_ptr;

#ifdef TCSUPPORT_OMCI_EXTENDED_MSG
#define OMCI_EXTENDED_MSG_LEN           1980
#define OMCI_EXTENDED_MSG_CONT_LEN      1966
#define OMCI_EXTENED_MSG_HEADER         14
#define OMCI_EXEENTED_SIZE_OF_MIC       4

typedef struct GNU_PACKED omciExtendedPayload_s{
	short tranCorrelatId;/* transaction correlation identifier */
	char msgType;/*message type*/
	char devId;/*device ID*/
	long meId   ;   /*HIGH 16 bit: ME ID; LOW 16 bit: Instance ID*/
	short msgContentLength;/*message content length*/
	char msgContent[OMCI_EXTENDED_MSG_CONT_LEN];/* pointer to message content*/
	char mic[4];//OMCI message integrity check
}omciExtendedPayload_t,*omciExtendedPayload_ptr;

#define omciPayload_t omciExtendedPayload_t
#define omciPayload_ptr omciExtendedPayload_ptr
#else
#define omciPayload_t omciBaselinePayload_t
#define omciPayload_ptr omciBaselinePayload_ptr
#endif


typedef struct omci_cmd_msg_s{
	long cmdType;//  cmd; 
	omciBaselinePayload_t payload;
}omci_cmd_msg_t;

typedef struct omci_ipc_cmd_msg_s{
	long msgType;
	omci_cmd_msg_t msg;
}omci_ipc_cmd_msg_t;



/*****************IPC for notification******************/
#define OMCI_NOTIFY_Q_FLAG_PATH "/tmp/omci/omci_notify_mq"


#define OMCI_NOTIFY_TYPE_ALARM          1
#define OMCI_NOTIFY_TYPE_AVC            2
#define OMCI_NOTIFY_TYPE_TCA            3
#define OMCI_NOTIFY_TYPE_TEST_RST       4
#define OMCI_NOTIFY_TYPE_PERIOD_AVC     5

/* alarm ID */
/* alarm ID = CLASS ID<<16 | ALARM INDEX */
#define OMCI_ALARM_ID_EQUIP_ALARM               (OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16 )
#define OMCI_ALARM_ID_POWER_ALARM               ((OMCI_CLASS_ID_ONU_G<<16)|0x1)
#define OMCI_ALARM_ID_BATTERY_MISS_ALARM        ((OMCI_CLASS_ID_ONU_G<<16)|0x2)
#define OMCI_ALARM_ID_EQUIPMENT                 ((OMCI_CLASS_ID_ONU_G<<16) | 0)
#define OMCI_ALARM_ID_POWERING                  ((OMCI_CLASS_ID_ONU_G<<16) | 1)
#define OMCI_ALARM_ID_BATTERY_MISS              ((OMCI_CLASS_ID_ONU_G<<16) | 2)
#define OMCI_ALARM_ID_BATTERY_FAIL              ((OMCI_CLASS_ID_ONU_G<<16) | 3)
#define OMCI_ALARM_ID_BATTERY_LOW               ((OMCI_CLASS_ID_ONU_G<<16) | 4)
#define OMCI_ALARM_ID_PHYSICAL_INSTU            ((OMCI_CLASS_ID_ONU_G<<16) | 5)
#define OMCI_ALARM_ID_ONU_SELFTEST_FAIL         ((OMCI_CLASS_ID_ONU_G<<16) | 6)
#define OMCI_ALARM_ID_ONU_DYING_GASP            ((OMCI_CLASS_ID_ONU_G<<16) | 7)
#define OMCI_ALARM_ID_ONU_TEMPER_YELLOW         ((OMCI_CLASS_ID_ONU_G<<16) | 8)
#define OMCI_ALARM_ID_ONU_TEMPER_RED            ((OMCI_CLASS_ID_ONU_G<<16) | 9)
#define OMCI_ALARM_ID_ONU_VOLTAGE_YELLOW        ((OMCI_CLASS_ID_ONU_G<<16) | 10)
#define OMCI_ALARM_ID_ONU_VOLTAGE_RED           ((OMCI_CLASS_ID_ONU_G<<16) | 11)
#define OMCI_ALARM_ID_ONU_ONU_MANANA_POWER_OFF  ((OMCI_CLASS_ID_ONU_G<<16) | 12)
#define OMCI_ALARM_ID_ONU_INV_IMAGE             ((OMCI_CLASS_ID_ONU_G<<16) | 13)
#define OMCI_ALARM_ID_ONU_PSE_OVERLOAD_YELLOW   ((OMCI_CLASS_ID_ONU_G<<16) | 14)
#define OMCI_ALARM_ID_ONU_PSE_OVERLOAD_RED      ((OMCI_CLASS_ID_ONU_G<<16) | 15)

#define OMCI_ALARM_ID_LOW_RX_OPTICAL            ((OMCI_CLASS_ID_ANI_G<<16) | 0)
#define OMCI_ALARM_ID_HIGH_RX_OPTICAL           ((OMCI_CLASS_ID_ANI_G<<16) | 1)
#define OMCI_ALARM_ID_SF                        ((OMCI_CLASS_ID_ANI_G<<16) | 2)
#define OMCI_ALARM_ID_SD                        ((OMCI_CLASS_ID_ANI_G<<16) | 3)
#define OMCI_ALARM_ID_LOW_TX_OPTICAL            ((OMCI_CLASS_ID_ANI_G<<16) | 4)
#define OMCI_ALARM_ID_HIGH_TX_OPTICAL           ((OMCI_CLASS_ID_ANI_G<<16) | 5)
#define OMCI_ALARM_ID_LASER_BIAS_CURRENT        ((OMCI_CLASS_ID_ANI_G<<16) | 6)

#define OMCI_ALARM_ID_PPTPEthernetUNI_LAN_LOS_ALARM             ((OMCI_CLASS_ID_PPTP_ETHERNET_UNI<<16)|0x0)
#define OMCI_ALARM_ID_VEIP_CONNECTING_FUNCTION_FAIL             ((OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT<<16)|0x0)

/*9.9.2 SIP user data*/
#define OMCI_ALARM_ID_SIP_USER_DATA_SIPUA_REG_AUTH              ((OMCI_CLASS_ID_SIP_USER_DATA<<16) | 0)
#define OMCI_ALARM_ID_SIP_USER_DATA_SIPUA_REG_TIMEOUT           ((OMCI_CLASS_ID_SIP_USER_DATA<<16) | 1)
#define OMCI_ALARM_ID_SIP_USER_DATA_SIPUA_REG_FAIL              ((OMCI_CLASS_ID_SIP_USER_DATA<<16) | 2)

/*9.9.3 SIP agent config data*/
#define OMCI_ALARM_ID_SIP_AGENT_CFG_DATA_SIPUA_REG_NAME         ((OMCI_CLASS_ID_SIP_AGENT_CONFIG_DATA<<16) | 0)
#define OMCI_ALARM_ID_SIP_AGENT_CFG_DATA_SIPUA_REG_REACH        ((OMCI_CLASS_ID_SIP_AGENT_CONFIG_DATA<<16) | 1)
#define OMCI_ALARM_ID_SIP_AGENT_CFG_DATA_SIPUA_REG_CONNECT      ((OMCI_CLASS_ID_SIP_AGENT_CONFIG_DATA<<16) | 2)
#define OMCI_ALARM_ID_SIP_AGENT_CFG_DATA_SIPUA_REG_VALID        ((OMCI_CLASS_ID_SIP_AGENT_CONFIG_DATA<<16) | 3)
                                                                          
/*9.9.16 MGC config data*/                                                
#define OMCI_ALARM_ID_MGC_CFG_DATA_TIMEOUT_ALARM                          ((OMCI_CLASS_ID_MGC_CFG_DATA<<16) | 0)

/*9.9.18 VoIP config data*/
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_NAME                      ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 0)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_REACH                     ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 1)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_CONNECT                   ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 2)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_VALIDATE                  ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 3)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_AUTH                      ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 4)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_TIMEOUT                   ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 5)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_FAIL                      ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 6)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_FILE_ERROR                    ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 7)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_NAME             ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 8)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_REACH            ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 9)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_CONNECT          ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 10)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_VALIDATE         ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 11)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_AUTH             ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 12)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_TIMEOUT          ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 13)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_FAIL             ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 14)
#define OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_REBOOT_REQUEST                ((OMCI_CLASS_ID_VOIP_CFG_DATA<<16) | 15)

/* clock data set*/
#define OMCI_ALARM_ID_CLOCK_DATA_SET_OUN_LOST_ToD_SYNC_ALARM            ((OMCI_CLASS_ID_CLOCK_DATA_SET<<16) | 0)

/* AVC ID  = CLASS ID<<16 | ATTRI INDEX */
#define NO_AVC				0
#define OMCI_AVC_ID_CURRENT_ADDR	((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0x9)
#define OMCI_AVC_ID_CURRENT_MASK	((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xa)
#define OMCI_AVC_ID_CURRENT_GATEWAY	((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xb)
#define OMCI_AVC_ID_CURRENT_PRIM_DNS	((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xc)
#define OMCI_AVC_ID_CURRENT_SECOND_DNS	((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xd)
#define OMCI_AVC_ID_DOMAIN_NAME		((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xe)
#define OMCI_AVC_ID_HOST_NAME		((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xf)
#define OMCI_AVC_ID_ANI_G_ARC 							        ((OMCI_CLASS_ID_ANI_G <<16) | 8)
#define OMCI_AVC_ID_ONU2_G_OMCC_VERSION						((OMCI_CLASS_ID_ONU2_G <<16) | 2)
#define OMCI_AVC_ID_ONU_G_OP_STATE 						((OMCI_CLASS_ID_ONU_G <<16) | 8)
#define OMCI_AVC_ID_ONU_G_LOID							((OMCI_CLASS_ID_ONU_G <<16) | 10)
#define OMCI_AVC_ID_ONU_G_LPW							((OMCI_CLASS_ID_ONU_G <<16) | 11)
#define OMCI_AVC_ID_BUFFER_TABLE	                         ((OMCI_CLASS_ID_GENERAL_BUFFER<<16)|0x2)
#define OMCI_AVC_ID_FILE_TRANSFER_STATUS 			((OMCI_CLASS_ID_FILE_TRANSFER_CONTROLLER<<16)|0x7)
#define OMCI_AVC_ID_STATUS_DOCUMENT 					((OMCI_CLASS_ID_GENERIC_STATUS<<16)|0x1)
#define OMCI_AVC_ID_CONFIGURATION_DOCUMENT 			((OMCI_CLASS_ID_GENERIC_STATUS<<16)|0x2)
#define OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_RANDOM_CHALLENGE_TB	((OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL<<16)|0x5)
#define OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_AUTHENTICATION_RESULT_TB	((OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL<<16)|0x6)
#define OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_AUTHENTICATION_STATUS	((OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL<<16)|0xa)
#define OMCI_AVC_ID_GEM_IWTP_OPERATIONAL_STATE				((OMCI_CLASS_ID_GEM_IWTP<<16|0x6))
#define OMCI_AVC_ID_MULTICAST_GEM_IWTP_OPERATIONAL_STATE	((OMCI_CLASS_ID_MULTICAST_GEM_IWTP<<16|0x6))
#define OMCI_AVC_ID_PPTPEthernetUNI_SENSED_TYPE				((OMCI_CLASS_ID_PPTP_ETHERNET_UNI<<16)|0x2)
#define OMCI_AVC_ID_PPTPEthernetUNI_OPERATIONAL_STATE		((OMCI_CLASS_ID_PPTP_ETHERNET_UNI<<16)|0x6)
#define OMCI_AVC_ID_PPTPEthernetUNI_ARC						((OMCI_CLASS_ID_PPTP_ETHERNET_UNI<<16)|0xB)
#define OMCI_AVC_ID_VEIP_OPERATIONAL_STATE					((OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT<<16)|0x2)
#define OMCI_AVC_ID_IP_HOST_CUR_ADDR							((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0x9)
#define OMCI_AVC_ID_IP_HOST_CUR_MASK							((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xa)
#define OMCI_AVC_ID_IP_HOST_CUR_GATEWAY						((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xb)
#define OMCI_AVC_ID_IP_HOST_CUR_PRIMARY_DNS					((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xc)
#define OMCI_AVC_ID_IP_HOST_CUR_SECONDARY_DNS				((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xd)
#define OMCI_AVC_ID_IP_HOST_DOMAIN_NAME						((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xe)
#define OMCI_AVC_ID_IP_HOST_HOST_NAME						((OMCI_CLASS_ID_IP_HOST_CONFIG_DATA<<16)|0xf)
/*9.9.3 SIP agent config data*/
#define OMCI_AVC_ID_SIP_AGENT_CFG_DATA_SIP_STATUS           ((OMCI_CLASS_ID_SIP_AGENT_CONFIG_DATA<<16)|0x09)
/*9.9.12: Call Ctrl PM history data*/
#define OMCI_TCA_ID_CALL_CTRL_SETUP_FAIL						    (CID_CALL_CTRL_PM_HISTORY<<16|0x1)
#define OMCI_TCA_ID_CALL_CTRL_SETUP_TIMER						    (CID_CALL_CTRL_PM_HISTORY<<16|0x2)
#define OMCI_TCA_ID_CALL_CTRL_TMINAT_FAIL						    (CID_CALL_CTRL_PM_HISTORY<<16|0x3)
#define OMCI_TCA_ID_CALL_CTRL_PORT_RELEASE						    (CID_CALL_CTRL_PM_HISTORY<<16|0x4)
#define OMCI_TCA_ID_CALL_CTRL_PORT_OH_TIMER						    (CID_CALL_CTRL_PM_HISTORY<<16|0x5)

/*9.9.13: RTP PM history data*/
#define OMCI_TCA_ID_RTP_RTP_ERROR						     	    (OMCI_CLASS_ID_RTP_PM_HISTORY_DATA<<16|0x1)
#define OMCI_TCA_ID_RTP_PACKET_LOSS						     	    (OMCI_CLASS_ID_RTP_PM_HISTORY_DATA<<16|0x2)
#define OMCI_TCA_ID_RTP_MAX_JITTER						     	    (OMCI_CLASS_ID_RTP_PM_HISTORY_DATA<<16|0x3)
#define OMCI_TCA_ID_RTP_MAX_BET_RTCP						     	(OMCI_CLASS_ID_RTP_PM_HISTORY_DATA<<16|0x4)
#define OMCI_TCA_ID_RTP_BUFFER_UNDERFLOWS			            	(OMCI_CLASS_ID_RTP_PM_HISTORY_DATA<<16|0x6)
#define OMCI_TCA_ID_RTP_BUFFER_OVERFLOWS			             	(OMCI_CLASS_ID_RTP_PM_HISTORY_DATA<<16|0x7)

/*9.9.14 SIP agent performance monitoring history data*/
#define OMCI_TCA_ID_SIPAMD_RX_INV_REQ                               (OMCI_CLASS_SIP_AGENT_PM_HISTORY<<16|0x1)
#define OMCI_TCA_ID_SIPAMD_RX_INV_REQ_RETRAS			     	    (OMCI_CLASS_SIP_AGENT_PM_HISTORY<<16|0x2)
#define OMCI_TCA_ID_SIPAMD_RX_NOINV_REQ					     	    (OMCI_CLASS_SIP_AGENT_PM_HISTORY<<16|0x3)
#define OMCI_TCA_ID_SIPAMD_RX_NOINV_REQ_RETRAS				     	(OMCI_CLASS_SIP_AGENT_PM_HISTORY<<16|0x4)
#define OMCI_TCA_ID_SIPAMD_RX_RESPONSE  			            	(OMCI_CLASS_SIP_AGENT_PM_HISTORY<<16|0x5)
#define OMCI_TCA_ID_SIPAMD_RX_RESPONS_RETRANS		             	(OMCI_CLASS_SIP_AGENT_PM_HISTORY<<16|0x6)

/*9.9.15 SIP call initiation performance monitoring history data*/
#define OMCI_TCA_ID_SIP_CALL_PM_FAIL_CONNECT                        (OMCI_CLASS_SIP_CALL_INIT_PM_HISTORY<<16|0x1)
#define OMCI_TCA_ID_SIP_CALL_PM_FAIL_VALIDATE   		     	    (OMCI_CLASS_SIP_CALL_INIT_PM_HISTORY<<16|0x2)
#define OMCI_TCA_ID_SIP_CALL_PM_TIMEOUT         		     	    (OMCI_CLASS_SIP_CALL_INIT_PM_HISTORY<<16|0x3)
#define OMCI_TCA_ID_SIP_CALL_PM_FAIL_ERROR_CODE_RECEIVED	     	(OMCI_CLASS_SIP_CALL_INIT_PM_HISTORY<<16|0x4)
#define OMCI_TCA_ID_SIP_CALL_PM_FAILED_AUTH     	            	(OMCI_CLASS_SIP_CALL_INIT_PM_HISTORY<<16|0x5)


/*9.9.18 VoIP config data*/
#define OMCI_AVC_ID_VOIP_CFG_PROFILE_VERSION				((OMCI_CLASS_ID_VOIP_CFG_DATA<<16)|0x8)
#define OMCI_AVC_ID_PPTPPotsUNI_ARC							((OMCI_ME_CLASS_ID_PPTP_POTS_UNI<<16)|0x3)
#define OMCI_AVC_ID_PPTPPotsUNI_OPERATIONAL_STATE			((OMCI_ME_CLASS_ID_PPTP_POTS_UNI<<16)|0x9)

/* TCA ID  = CLASS ID<<16 | TCA NUM */
#define NO_TCA		0
#define OMCI_TCA_ID_IP_HOST_PM_ICMP_ERR		(OMCI_CLASS_ID_IP_HOST_PM_HISTORY<<16|0x1)
#define OMCI_TCA_ID_FEC_PM_CORRECTEDBYTES                         		((OMCI_CLASS_ID_FEC_PM_DATA<<16) | 0x1)
#define OMCI_TCA_ID_FEC_PM_CORRECTEDODEWORDS   				((OMCI_CLASS_ID_FEC_PM_DATA<<16) | 0x2)
#define OMCI_TCA_ID_FEC_PM_UNCORRECTABLECODEWORDS		  	((OMCI_CLASS_ID_FEC_PM_DATA<<16) | 0x3)
#define OMCI_TCA_ID_FEC_PM_FECSECONDS					  	((OMCI_CLASS_ID_FEC_PM_DATA<<16) | 0x4)

#define OMCI_TCA_ID_GEM_PORT_NETWORK_CTP_PM                         		((OMCI_CLASS_ID_GEM_PORT_NETWORK_CTP_PM<<16) | 0x1)

//9.2.6 GEM port performance monitoring PM
#define OMCI_TCA_ID_GEM_PORT_PM_LOST_PACKETS				((OMCI_CLASS_ID_GEM_PORT_PM<<16) | 0x1)
#define OMCI_TCA_ID_GEM_PORT_PM_MISINSERTED_PACKETS				((OMCI_CLASS_ID_GEM_PORT_PM<<16) | 0x2)
#define OMCI_TCA_ID_GEM_PORT_PM_IMPAIRED_PACKETS				((OMCI_CLASS_ID_GEM_PORT_PM<<16) | 0x3)

//Alcatel ME:  GEM port performance monitoring PM
#define OMCI_TCA_ID_TOTAL_GEM_PORT_PM_LOST_COUNTER_DOWNSTREAM			((OMCI_CLASS_ID_TOTAL_GEM_PORT_PM<<16) | 0x1)
#define OMCI_TCA_ID_TOTAL_GEM_PORT_PM_LOST_COUNTER_UPSTREAM				((OMCI_CLASS_ID_TOTAL_GEM_PORT_PM<<16) | 0x2)
#define OMCI_TCA_ID_TOTAL_GEM_PORT_PM_IMPAIRED_BLOCKS					((OMCI_CLASS_ID_TOTAL_GEM_PORT_PM<<16) | 0x3)
#define OMCI_TCA_ID_TOTAL_GEM_PORT_PM_BAD_RECEIVED_CONTERS				((OMCI_CLASS_ID_TOTAL_GEM_PORT_PM<<16) | 0x4)

//Alcatel ME:  Ethernet Traffic PM
#define OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_FRAMES_UPSTREAM_COUNTER 			((OMCI_CLASS_ID_ETHERNET_TRAFFIC_PM<<16) | 0x1)
#define OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_BYTES_UPSTREAM_COUNTER				((OMCI_CLASS_ID_ETHERNET_TRAFFIC_PM<<16) | 0x2)
#define OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_FRAMES_DOWNSTREAM_COUNTER			((OMCI_CLASS_ID_ETHERNET_TRAFFIC_PM<<16) | 0x3)

#define OMCI_TCA_ID_LOST_GEM_FRAGMENT_COUNTER			(OMCI_CLASS_ID_GEMPORT_PROTOCOL_MONITORING_HISTORY_DATA_PART_2<<16|0x1)

/*9.3.3 MAC bridge performance monitoring history data ME*/
#define OMCI_TCA_ID_MAC_BRIDGE_PM_LEARN_ENTRY_DISCARD_CNTS	((OMCI_ME_CLASS_ID_MAC_BRIDGE_PMHD<<16) | 0x1)

/*9.3.9 mac bridge port performance monitoring history data ME*/
#define OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_DELAY_EXCEED_DISCARD_CNTS	((OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_PMHD<<16) | 0x1)
#define OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_MTU_EXCEED_DISCARD_CNTS		((OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_PMHD<<16) | 0x2)
#define OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_RECEIVED_DISCARD_CNTS			((OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_PMHD<<16) | 0x3)

/*9.3.30 ethernet frame performance monitoring history data upstream*/
#define OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_UP_DELAY_EXCEED_DISCARD_CNTS	((OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD<<16) | 0x1)
#define OMCI_TCA_ID_ETH_FRAME_PM_UP_CRC_ERRORS_PKTS						((OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD<<16) | 0x2)
#define OMCI_TCA_ID_ETH_FRAME_PM_UP_UNDERSIZE_PKTS						((OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD<<16) | 0x3)
#define OMCI_TCA_ID_ETH_FRAME_PM_UP_OVERSIZE_PKTS							((OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD<<16) | 0x4)

/*9.3.31 ethernet frame performance monitoring history data downstream*/
#define OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_DOWN_DELAY_EXCEED_DISCARD_CNTS	((OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD<<16) | 0x1)
#define OMCI_TCA_ID_ETH_FRAME_PM_DOWN_CRC_ERRORS_PKTS					((OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD<<16) | 0x2)
#define OMCI_TCA_ID_ETH_FRAME_PM_DOWN_UNDERSIZE_PKTS						((OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD<<16) | 0x3)
#define OMCI_TCA_ID_ETH_FRAME_PM_DOWN_OVERSIZE_PKTS						((OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD<<16) | 0x4)

/*9.2.8 GAL Ethernet performance monitoring history data*/
#define OMCI_TCA_ID_GAL_ETHERNET_PM_DISCARDED_FRAMES					(OMCI_CLASS_ID_GAL_ETHERNET_PM<<16|0x1)

/*9.5.2: Ethernet PM history data*/
#define OMCI_TCA_ID_ETHERNET_PM_2_FEC_ERRORS							(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x1)
#define OMCI_TCA_ID_ETHERNET_PM_2_EXCESSIVE_COLLISION_CNT				(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x2)
#define OMCI_TCA_ID_ETHERNET_PM_2_LATE_COLLISION_CNT					(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x3)
#define OMCI_TCA_ID_ETHERNET_PM_2_FRAMES_TOO_LONG						(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x4)
#define OMCI_TCA_ID_ETHERNET_PM_2_BUFFER_OVERFLOWS_ON_RECEIVE		(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x5)
#define OMCI_TCA_ID_ETHERNET_PM_2_BUFFER_OVERFLOWS_ON_TRANSMIT		(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x6)
#define OMCI_TCA_ID_ETHERNET_PM_2_SINGLE_COLLISION_FRAME_CNT			(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x7)
#define OMCI_TCA_ID_ETHERNET_PM_2_MULTI_COLLISION_FRAME_CNT			(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x8)
#define OMCI_TCA_ID_ETHERNET_PM_2_SQE_CNT								(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0x9)
#define OMCI_TCA_ID_ETHERNET_PM_2_DEFERRED_TRANSMISSION_CNT			(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0xa)
#define OMCI_TCA_ID_ETHERNET_PM_2_INTERNAL_MAC_TRANSMIT_ERROR_CNT	(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0xb)
#define OMCI_TCA_ID_ETHERNET_PM_2_CARRIER_SENSE_ERROR_CNT				(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0xc)
#define OMCI_TCA_ID_ETHERNET_PM_2_ALIGNMENT_ERROR_CNT					(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0xd)
#define OMCI_TCA_ID_ETHERNET_PM_2_INTERNAL_MAC_RECEIVE_ERROR_CNT		(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA<<16|0xe)
/*9.5.3: Ethernet PM history data 2*/
#define OMCI_TCA_ID_ETHERNET_PM_3_PPPOE_FILTERED_FRAME_CNT			(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_2<<16|0x1)
/*9.5.4: Ethernet PM history data 3*/
#define OMCI_TCA_ID_ETHERNET_PM_4_DROP_EVENTS							(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_3<<16|0x1)
#define OMCI_TCA_ID_ETHERNET_PM_4_UNDERSIZE_PACKETS					(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_3<<16|0x2)
#define OMCI_TCA_ID_ETHERNET_PM_4_FRAGMENTS								(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_3<<16|0x3)
#define OMCI_TCA_ID_ETHERNET_PM_4_JABBERS								(OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_3<<16|0x4)
/*9.3.32 Ethernet frame extended PM*/
#define OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_DROP_EVENTS			(OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM<<16|0x1)
#define OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_CRC_ERROR_FRAMES		(OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM<<16|0x2)
#define OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_UNDERSIZE_FRAMES		(OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM<<16|0x3)
#define OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_OVERSIZE_FRAMES		(OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM<<16|0x4)
/*9.3.34 Ethernet frame extended PM64-bit*/
#define OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_64BIT_DROP_EVENTS			(OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM_64_BIT<<16|0x1)
#define OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_64BIT_CRC_ERROR_FRAMES		(OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM_64_BIT<<16|0x2)
#define OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_64BIT_UNDERSIZE_FRAMES		(OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM_64_BIT<<16|0x3)
#define OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_64BIT_OVERSIZE_FRAMES		(OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM_64_BIT<<16|0x4)


/*9.4.2 IP Host config data PM*/
#define OMCI_TCA_ID_IP_HOST_PM_ICMP_ERRORS								(OMCI_CLASS_ID_IP_HOST_PM<<16|0x1)
#define OMCI_TCA_ID_IP_HOST_PM_DNS_ERRORS								(OMCI_CLASS_ID_IP_HOST_PM<<16|0x2)
#define OMCI_TCA_ID_IP_HOST_PM_DHCP_TIMEOUT								(OMCI_CLASS_ID_IP_HOST_PM<<16|0x3)
#define OMCI_TCA_ID_IP_HOST_PM_IP_CONFLICT								(OMCI_CLASS_ID_IP_HOST_PM<<16|0x4)
#define OMCI_TCA_ID_IP_HOST_PM_OUT_OF_MEMORY							(OMCI_CLASS_ID_IP_HOST_PM<<16|0x5)
#define OMCI_TCA_ID_IP_HOST_PM_INTERNAL_ERROR							(OMCI_CLASS_ID_IP_HOST_PM<<16|0x6)
/*9.4.4 TCP/UDP config data PM*/
#define OMCI_TCA_ID_TCP_UDP_PM_SOCKET_FAILED							(OMCI_CLASS_ID_TCP_UDP_PM<<16|0x1)
#define OMCI_TCA_ID_TCP_UDP_PM_LISTEN_FAILED								(OMCI_CLASS_ID_TCP_UDP_PM<<16|0x2)
#define OMCI_TCA_ID_TCP_UDP_PM_BIND_FAILED								(OMCI_CLASS_ID_TCP_UDP_PM<<16|0x3)
#define OMCI_TCA_ID_TCP_UDP_PM_ACCEPT_FAILED							(OMCI_CLASS_ID_TCP_UDP_PM<<16|0x4)
#define OMCI_TCA_ID_TCP_UDP_PM_SELECT_FAILED								(OMCI_CLASS_ID_TCP_UDP_PM<<16|0x5)


/*for set device id in PM*/
#define PM_LAN_INTERFACE_0			0
#define PM_WAN_INTERFACE_1		1
//#define PM_INTERFACE_VALID_BITS	0x8000
//#define PM_INTERFACE_SHIFT_NUM	15

#define OMCI_MIN_PM_ATTR_INDEX	3 // jump me id, interval end time, threshold data 1/2 id
#define PMGR_MAC_BRIDGE_PORT_ENTRY	"OMCI_Entry"
#define PMGR_ATTR_MACBRIDGEPORT_PREFIX	"macBridgePort" //set prefix of mac bridge port attribute in PMGR_MAC_BRIDGE_PORT_ENTRY node
#define OMCI_RTP_PM_RTPERROR        "rtpError"
#define OMCI_RTP_PM_PacketLoss		"packetLossRate"
#define OMCI_RTP_PM_MaxJitter	    "maxJitter"
#define OMCI_RTP_PM_MaxTimeBetRTCP  "maxRTCPInterval"
#define OMCI_RTP_PM_BUFUNDERFLOW    "bufUnderflow"
#define OMCI_RTP_PM_BUFOVERFLOW     "bufOverflow"
#define PMGR_ATTR_TOTALGEMPORT	"totalGemPort"
/* test result ID = CLASS ID<<16|  */


#define OMCI_CLASS_ID_ONU_DATA                                  2
#define OMCI_CLASS_ID_PON_IF_LINE_CARDHOLDER                    3
#define OMCI_CLASS_ID_CARDHOLDER                                5
#define OMCI_CLASS_ID_CIRCUIT_PACK                              6
#define OMCI_CLASS_ID_SOFTWARE_IMGAE                            7
#define OMCI_CLASS_ID_PPTP_ETHERNET_UNI                         11
#define OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA                  24
/*9.3.1 mac bridge service profile*/
 #define OMCI_ME_CLASS_ID_MAC_BRIDGE_SERVICE_PROFILE            45
/*9.3.2 mac bridge configuration data*/
#define OMCI_ME_CLASS_ID_MAC_BRIDGE_CON_DATA                    46
/*9.3.4 mac bridge port configuration data*/
#define OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA               47
/*9.3.5 mac bridge port designation data*/
#define OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_DESIGNATION_DATA       48
/*9.3.6 mac bridge port filter table table*/
#define OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA      49
/*9.3.8 mac bridge port bridge table data*/
#define OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_TABLE_DATA             50
/*9.3.3 mac bridge performance monitoring history data
   PMHD=performance monitoring history data*/
#define OMCI_ME_CLASS_ID_MAC_BRIDGE_PMHD                        51
/*9.3.9 mac bridge port performance monitoring history data
   PMHD=performance monitoring history data*/
#define OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_PMHD                   52
/*9.9.1 Physical path termination point POTS UNI*/
#define OMCI_ME_CLASS_ID_PPTP_POTS_UNI                          53
/*9.9.6 Voice service profile*/
#define CID_VOICE_SERVICE_PROFILE                               58
/*9.3.12 vlan tagging operation configuration data*/
#define OMCI_ME_CLASS_ID_VLAN_TAG_OPERATE                       78
/*9.3.7 mac bridge port filter preassign table*/
#define OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_PREASSIGN_DATA  79
/*9.3.11 vlan tagging filter data*/
#define OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA                   84
#define OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_2                89
/*9.3.10 802.1p mapper service profile*/
#define OMCI_ME_CLASS_ID_VLAN_802_1P                            130

#define OMCI_CLASS_ID_OLT_G                                     131
#define OMCI_CLASS_ID_ONU_POWER_SHEDDING                        133
#define OMCI_CLASS_ID_IP_HOST_CONFIG_DATA                       134
#define OMCI_CLASS_ID_IP_HOST_PM                                135
#define OMCI_CLASS_ID_TCP_UDP_CFGDATA                           136
#define OMCI_CLASS_ID_NETWORK_ADDRESS                           137

/*9.9.18 VoIP config data*/
#define OMCI_CLASS_ID_VOIP_CFG_DATA                             138
#define OMCI_CLASS_ID_VOIP_VOICE_CTP                           139 		/* 9.9.4 */
#define CID_CALL_CTRL_PM_HISTORY                                140        /* 9.9.12 */
#define OMCI_CLASS_ID_VOIP_LINE_STATUS                          141
/*9.9.5 VoIP media profile*/
#define OMCI_CLASS_ID_VOIP_MEDIA_PROFILE                        142
/*9.9.7 	RTP profile data*/
#define CID_RTP_PROFILE_DATA                                    143
/*9.9.13 RTP PM*/
#define OMCI_CLASS_ID_RTP_PM_HISTORY_DATA                       144
/*9.9.10 Network dial plan table*/
#define OMCI_CLASS_ID_NETWORK_DIAL_PLAN_TBL                     145
/*9.9.8  VoIP application service profile*/
#define CID_VOIP_APP_SVC_PROFILE                                146


#define OMCI_CLASS_ID_AUTHENTICATION_METHOD                     148

/*9.9.3 SIP agent config data*/
#define OMCI_CLASS_ID_SIP_AGENT_CONFIG_DATA                     150
/*9.9.14 SIP agent performance monitoring history data*/
#define OMCI_CLASS_SIP_AGENT_PM_HISTORY                         151
/*9.9.15 SIP call initiation performance monitoring history data*/
#define OMCI_CLASS_SIP_CALL_INIT_PM_HISTORY                     152
/*9.9.2 SIP user data*/
#define OMCI_CLASS_ID_SIP_USER_DATA                             153
/*9.9.16 MGC config data*/
#define OMCI_CLASS_ID_MGC_CFG_DATA                              155

#define OMCI_CLASS_ID_LARGE_STRING                              157
#define OMCI_CLASS_ID_ONU_REMOTE_DEBUG                          158
#define OMCI_CLASS_ID_PROTECTION_PROFILE                        159
#define OMCI_CLASS_ID_EXTENSION_PACKAGE                         160
/*9.3.13 extended vlan tagging operation configuration data*/
#define OMCI_ME_CLASS_ID_VLAN_TAG_EXTEND_OPERATE                171
        
#define OMCI_CLASS_ID_ONU_G                                     256
#define OMCI_CLASS_ID_ONU2_G                                    257
#define OMCI_CLASS_ID_T_CONT                                    262
#define OMCI_CLASS_ID_ANI_G                                     263
#define OMCI_CLASS_ID_UNI_G                                     264
#define OMCI_CLASS_ID_GEM_INTERWORK_TP                          266
/*9.2.4 GEM interworking termination point*/        
#define OMCI_CLASS_ID_GEM_IWTP                                  266
/*9.2.6 GEM port performance monitoring history data*/      
#define OMCI_CLASS_ID_GEM_PORT_PM                               267
#define OMCI_CLASS_ID_GEM_PORT_CTP                              268
/*9.2.7 GAL Ethernet profile*/      
#define OMCI_CLASS_ID_GAL_ETHERNET_PROFILE                      272
#define OMCI_CLASS_ID_THRESHOLD_DATA1                           273
#define OMCI_CLASS_ID_THRESHOLD_DATA2                           274
/*9.2.8 GAL Ethernet performance monitoring history data*/
#define OMCI_CLASS_ID_GAL_ETHERNET_PM                           276
#define OMCI_CLASS_ID_PRIORITY_QUEUE                            277
#define OMCI_CLASS_ID_TRAFFIC_SCHEDULER                         278
#define OMCI_CLASS_ID_PROTECTION_DATA                           279
#define OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR                        280
#define OMCI_CLASS_ID_MULTICAST_GEM_INTERWORK_TP                281
/*9.2.5 Multicast GEM interworking termination point*/      
#define OMCI_CLASS_ID_MULTICAST_GEM_IWTP                        281
#define OMCI_CLASS_ID_OMCI                                      287
#define OMCI_CLASS_ID_MANAGED_ENTITY                            288
#define OMCI_CLASS_ID_ATTRIBUTE                                 289
#define OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_3                296
#define OMCI_CLASS_ID_PORT_MAPPING                              297
/*9.3.18: Dot1 rate limiter*/       
#define OMCI_CLASS_ID_DOT1_RATE_LIMITER                         298
/*9.3.19 Dot1ag maintenance domain*/
#define OMCI_CLASS_ID_DOT1AG_MAINTENANCE_DOMAIN                 299
/*9.3.20 Dot1ag maintenance association*/       
#define OMCI_CLASS_ID_DOT1AG_MAINTENANCE_ASSOCIATION            300    
#define OMCI_CLASS_ID_OCTET_STRING                              307
#define OMCI_CLASS_ID_GENERAL_BUFFER                            308

/*9.3.27 multicast operations profile*/
#define OMCI_ME_CLASS_ID_MULTICAST_OPERATE_PROFILE              309
/*9.3.28 multicast subscriber config info*/
#define OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO          310
/*9.3.29 multicast subscriber monitor*/
#define OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_MONITOR           311

#define OMCI_CLASS_ID_FEC_PM_DATA                               312
#define OMCI_CLASS_ID_FILE_TRANSFER_CONTROLLER                  318
/*9.3.31 ethernet frame performance monitoring history data downstream
   PMHD=performance monitoring history data*/
#define OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD               321
/*9.3.30 ethernet frame performance monitoring history data upstream
   PMHD=performance monitoring history data*/
#define OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD                 322

#define OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT          329
#define OMCI_CLASS_ID_GENERIC_STATUS                            330
#define OMCI_CLASS_ID_ONU_E                                     331
#define OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL                 332

#define OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM                334
     
#define OMCI_CLASS_ID_SNMP_CONFIG_DATA                          335     
#define OMCI_CLASS_ID_ONU_DYNAMIC_POWER                         336      
#define OMCI_CLASS_ID_TR069_MANAGE_SERVER                       340    
#define OMCI_CLASS_ID_GEM_PORT_NETWORK_CTP_PM                   341
#define OMCI_CLASS_ID_TCP_UDP_PM                                342
#define OMCI_CLASS_ID_ENERGY_CONSUMPTION_PM                     343
#define OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6                    348

#define OMCI_CLASS_ID_RESERVED_351_PM                       	351

#ifdef TCSUPPORT_VNPTT
#define OMCI_CLASS_ID_RESERVED_250_ME                       	250
#define OMCI_CLASS_ID_RESERVED_347_ME                      		347
#endif

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
#define OMCI_CLASS_ID_RESERVED_350_ME							350
#define OMCI_CLASS_ID_RESERVED_352_ME                      		352
#define OMCI_CLASS_ID_RESERVED_353_ME							353
#define OMCI_CLASS_ID_RESERVED_367_ME							367
#define OMCI_CLASS_ID_RESERVED_373_ME							373
#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
#define OMCI_CLASS_ID_RESERVED_370_ME							370
#define OMCI_CLASS_ID_RESERVED_65408_ME							65408
#define OMCI_CLASS_ID_RESERVED_65414_ME							65414
#define OMCI_CLASS_ID_RESERVED_65425_ME							65425
#endif
#endif
/*9.3.34 ethernet frame extended pm 64-bit*/
#define OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM_64_BIT         425 
/*9.12.17 threshold data 64 bit*/
#define OMCI_CLASS_ID_THRESHOLD_DATA_64_BIT						426          
/* CTC ME class ID */          
#define OMCI_CLASS_ID_CTC_ONU_CAPABILITY                        65529
#define OMCI_CLASS_ID_CTC_LOID_AUTHEN                           65530
#define OMCI_CLASS_ID_CTC_EXTENDED_MUTICAST_PROFILE             65531

#define OMCI_CLASS_ID_VOIP_CALL_STATISTICS                      65284
#define OMCI_CLASS_ID_CLOCK_DATA_SET                            65314
#define OMCI_CLASS_ID_PTP_MASTER_CONFIG_DATA_PROFILE            65315
#define OMCI_CLASS_ID_PTP_Port                                  65316
#define OMCI_CLASS_ID_TOTAL_GEM_PORT_PM                         65281
#define OMCI_CLASS_ID_ETHERNET_TRAFFIC_PM                       65282
#define OMCI_CLASS_ID_VLAN_TAG_DOWNSTREAM_EGRESS_BEHAVIOR_SUPPLEMENTAL1         65305
#define OMCI_CLASS_ID_VLAN_TAG_UPSTREAM_POLICER                 65306
#define OMCI_CLASS_ID_ONT_OPTICAL_SUPERVISION_STATUS            65295
#define OMCI_CLASS_ID_UNI_SUPPLEMENTAL_1_V2                     65297
#define OMCI_CLASS_ID_ONT_GENERIC_V2                            65296
#define OMCI_CLASS_ID_GEMPORT_PROTOCOL_MONITORING_HISTORY_DATA_PART_2           65300

#define OMCI_CLASS_ID_ONU_CAPABILITY                            65457


/* alarm flag */
#define OMCI_ALARM_ERR		1
#define OMCI_ALARM_RECOVERY	0


#define MAX_OMCI_MB_INTER_ATTR_NUM	5

typedef struct notify_msg_s{
	char notifyType;// OMCI_NOTIFY_TYPE_ALARM;
	//alarm
	long alarmId;
	char alarmFlag;//OMCI_ALARM_ERR;OMCI_ALARM_RECOVERY
	//AVC
	long avcId;
	//TCA
	long tcaId;
	//test result
	long testRstId;//class ID | transId ;
	short deviceId;//mapping to instant ID
	char msgFormat;		//0x0A:baseline format   0X0B:extended format	
	char otherInfo[128];	
}notify_msg_t;



typedef struct omci_ipc_notify_msg_s{
	long msgType;// 
	notify_msg_t notifyMsg;
}omci_ipc_notify_msg_t;

#endif//_OMCI_EXTERNAL_MSG_H



