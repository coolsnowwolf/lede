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
	omci_me_voip.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2013/9/26	Create
*/

#ifndef  _OMCI_VOIP_ME_H_
#define _OMCI_VOIP_ME_H_
#include "omci_types.h"
#include "omci_me.h"

/*******************************************************************************************************************************
defined
********************************************************************************************************************************/
enum omciVoipStatus {
	OMCI_VOIP_FAIL = -1,
	OMCI_VOIP_SUCC = 0,	
	OMCI_VOIP_NO_FIND =1,
	OMCI_VOIP_FIND = 2,
	OMCI_VOIP_CHANGE_ENTRY = 3,
	OMCI_VOIP_NO_CHANGE_ENTRY = 4
};

enum omciVoipParaEnable{
	OMCI_VOIP_PARA_DISABLE = 0,
	OMCI_VOIP_PARA_ENABLE = 1
};

#if defined (TCSUPPORT_OMCI_ALCATEL) && defined (TCSUPPORT_VOIP)
typedef struct callHistory_s {
    char date[8];
    char time[8];
    int32 callDuration;
    char calledNumber[25];
    char callingNumber[25];
    int32 RTPTxPackets;
    int32 RTPRxPackets;
    int32 RTPRxPacketsLost;
    int32 RTPPacketsDiscards;
    int32 RTPOverruns;
    int32 RTPUnderruns;
    int32 AverageJitter;
    uint8 RTCPParticipation;
    int32 PeakJitter;
    int32 AverageJitterBufferDepth;
    uint8 RTCP_XRParticipation;
    int32 AverageMOS;
    int32 PeakRoundTripDelay;
    int32 AverageRoundTripDelay;
}callHistory_t,*callHistory_ptr;
#endif

#define OMCI_TCAPI_GET_ACTION			0
#define OMCI_TCAPI_SET_ACTION			1

#define OMCI_ME_DEFAULT_STR_ATTR_LEN	32
#define OMCI_ME_MAX_STR_ATTR_LEN		25
#define OMCI_ME_MAX_LARGE_STR_PART_NUM	15

/*0:fxs1,1:fxs2,2:fxo*/

#define OMCI_VOIP_BASIC       "VoIPBasic"
#define OMCI_VOIP_ADVANCED    "VoIPAdvanced"
#define OMCI_VOIP_BASIC_ENTRY "VoIPBasic_Entry"
#define OMCI_VOIP_BASIC_COMMON "VoIPBasic_Common"
#define OMCI_VOIP_ADVANCED_ENTRY "VoIPAdvanced_Entry"
#define OMCI_VOIP_ADVANCED_COMMON "VoIPAdvanced_Common"

#define OMCI_VOIP_CODECS_ENTRY "VoIPCodecs_Entry"
#define OMCI_VOIP_CODECS_PVC_ENTRY "VoIPCodecs_PVC0_Entry"
#define OMCI_VOIP_CODECS_PVC "VoIPCodecs_PVC"

#define OMCI_VOIP_MEDIA_COMMON "VoIPMedia_Common"
#define OMCI_VOIP_MEDIA_ENTRY "VoIPMedia_Entry"

#define OMCI_VOIP_H248_ENTRY	"VoIPH248_Entry"
#define OMCI_VOIP_H248_COMMON	"VoIPH248_Common"

#define OMCI_VOIP_SYS_PARAM_ENTRY	"VoIPSysParam_Entry%d"
#define OMCI_VOIP_INFO_H248_ENTRY	"InfoVoIPH248_Entry%d"


#define OMCI_VOIP_DIGIT_MAP_ENTRY	"VoIPDigitMap_Entry"

#define OMCI_VOIP_ATTR_ENABLE_NAME	"1"
#define OMCI_VOIP_ATTR_DISABLE_NAME	"0"

/*9.9.1 PPTP ports UNI*/
#define OMCI_PPTP_UNI_STATE_UNLOCK  0
#define OMCI_PPTP_UNI_STATE_LOCK    1

/*9.9.2 sip user data*/
#define OMCI_MAX_SIP_DISP_NAME_LEN	24
#define OMCI_SIP_DISP_NAME_ATTR_LEN	25
#define OMCI_REGISTER_EXPIRERS_ATTR_LEN	4

#define OMCI_SIP_DISP_NAME		"SIPDisplayName"
#define OMCI_SIP_USER_PART_AOR	"SIPUserPartAor"
#define OMCI_SIP_AUTH_USER_NAME	"SIPAuthenticationName"
#define OMCI_SIP_PASSWORD		"SIPPassword"
#define OMCI_SIP_VOICEMAIL_EXPIRE	"SIPVoicemailExpire"
#define OMCI_SIP_URI			"URI"



/*9.9.5 VoIP media profile*/
#define OMCI_MEDIA_1ST	0
#define OMCI_MEDIA_2ND	1
#define OMCI_MEDIA_3RD	2
#define OMCI_MEDIA_4TH	3
#define OMCI_MEDIA_NUM  4

#define OMCI_FAX_MODE_PASSTHRU	0
#define OMCI_FAX_MODE_T38		1
#define OMCI_FAX_CODEC_NAME		"FaxCodec"
#define OMCI_FAX_CODEC_T38		"T38"
#define OMCI_FAX_CODEC_PCMA		"PCMA"
#define OMCI_FAX_CODEC_PCMU		"PCMU"

#define OMCI_MEDIA_CODEC_PCMU	0
/*G726-32 --reserved(1)*/
#define OMCI_MEDIA_CODEC_G726	1
#define OMCI_MEDIA_CODEC_G723	4
#define OMCI_MEDIA_CODEC_PCMA	8
#define OMCI_MEDIA_CODEC_G722	9
#define OMCI_MEDIA_CODEC_G729	18
#define OMCI_MEDIA_SIPSUPPORTED_CODECS_PCMA		"G.711 a-law"
#define OMCI_MEDIA_SIPSUPPORTED_CODECS_PCMU		"G.711 u-law"
#define OMCI_MEDIA_SIPSUPPORTED_CODECS_G722		"G.722"
#define OMCI_MEDIA_SIPSUPPORTED_CODECS_G723		"G.723"
#define OMCI_MEDIA_SIPSUPPORTED_CODECS_G726		"G.726"
#define OMCI_MEDIA_SIPSUPPORTED_CODECS_G726_32	"G.726 - 32"
#define OMCI_MEDIA_SIPSUPPORTED_CODECS_G726_24	"G.726 - 24"
#define OMCI_MEDIA_SIPSUPPORTED_CODECS_G729		"G.729"

#define OMCI_MEDIA_CODEC_NAME					"codec"
#define OMCI_MEDIA_PRIORITY                     "priority"
#define OMCI_MEDIA_SIP_PACKETIZATION_TIME_NAME	"SIPPacketizationTime"
#define OMCI_MEDIA_SILENCE_SUPPRESSION_NAME		"SilenceSuppression"
#define OMCI_MEDIA_SILENCE_COMPRESSIONENABLE    "SilenceCompressionEnable"

#define OMCI_MEDIA_711APTIME_NAME	"711APTime"
#define OMCI_MEDIA_711UPTIME_NAME	"711UPTime"
#define OMCI_MEDIA_729PTIME_NAME	"729PTime"
#define OMCI_MEDIA_726PTIME_NAME	"726-32PTime"
#define OMCI_MEDIA_722PTIME_NAME	"722PTime"

#define MIN_OMCI_MEDIA_PKT_PERIOD_VAL	10
#define MAX_OMCI_MEDIA_PKT_PERIOD_VAL	30

#define OMCI_MEDIA_SILENCE_OFF	0
#define OMCI_MEDIA_SILENCE_ON	1
#define OMCI_MEDIA_VAD_OFF_VAL	"0"
#define OMCI_MEDIA_VAD_ON_VAL	"1"
#define OMCI_MEDIA_SILENCE_DISABLE	"No"
#define OMCI_MEDIA_SILENCE_ENABLE   "Yes"

#define OMCI_MEDIA_DTMF_MODE_NAME	"DTMFTransportMode"
#define OMCI_MEDIA_DTMF_MODE_INBAND	"Inband"
#define OMCI_MEDIA_DTMF_MODE_RFC2833	"RFC2833"

/*9.9.11 VoIP line status*/
#define OMCI_VOIP_LINE_USED_CODEC             1
#define OMCI_VOIP_SERVER_STATUS_INITIAL       0
#define OMCI_VOIP_SERVER_STATUS_REGISTERED    1
#define OMCI_VOIP_SERVER_STATUS_FAILED        5
#define OMCI_VOIP_SERVER_STATUS_NOTCONFIG     13
#define OMCI_VOIP_PORT_SESSION_TYPE_IDLE      0
#define OMCI_VOIP_PORT_SESSION_TYPE_2WAY      1 
#define OMCI_VOIP_CALL_PACKET_PERIOD_ONE      1
#define OMCI_VOIP_CALL_PACKET_PERIOD_TWO      2

/*9.9.16 MGC config data*/
#define OMCI_H248_PRIMARY_ATTR_INDEX 				1
#define OMCI_H248_SECNDARY_ATTR_INDEX 				2

#define OMCI_H248_PRIMARY_MGC_IP_NAME		"MediaGatewayControler"
#define OMCI_H248_PRIMARY_MGC_PORT_NAME		"MediaGatewayControlerPort"
#define OMCI_H248_SECONDARY_MGC_IP_NAME		"SBMediaGatewayControler"
#define OMCI_H248_SECONDARY_MGC_PORT_NAME	"SBMediaGatewayControlerPort"

#define OMCI_H248_DEVICE_ID_NAME			"DeviceID"

#define OMCI_H248_MSG_FORMAT_NAME	"MsgFormat"
#define OMCI_H248_MSG_FORMAT_TEXT_LONG	0
#define OMCI_H248_MSG_FORMAT_TEXT_SHORT	1
#define OMCI_H248_MSG_FORMAT_BINARY  	2
#define OMCI_H248_MSG_FORMAT_TEXT_LONG_VAL	"Long"
#define OMCI_H248_MSG_FORMAT_TEXT_SHORT_VAL	"Short"

#define OMCI_H248_DEFAULT_MAX_RETRANS_TIME	3
#define OMCI_H248_MAX_RETRANS_TIME_NAME	"MaxreTransTime"

/*9.9.18 VoIP config data*/
#define OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_NAME	"SignalingProtocols"
#define OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_NONE		0
#define OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_SIP		(1<<0)
#define OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_H248		(1<<1)
#define OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_MGCP		(1<<2)

#define OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_SIP_VAL	"SIP"
#define OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_H248_VAL	"H.248"
#define OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_MGCP_VAL	"MGCP"

#define OMCI_VOIP_CFG_DATA_VOIP_CFG_METHOD_OMCI			(1<<0)
#define OMCI_VOIP_CFG_DATA_VOIP_CFG_METHOD_FILE			(1<<1)
#define OMCI_VOIP_CFG_DATA_VOIP_CFG_METHOD_TR69			(1<<2)
#define OMCI_VOIP_CFG_DATA_VOIP_CFG_METHOD_IETF			(1<<3)

#define OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NAME		"ServerType"
#define OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NONE		0
#define OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_SIP		1
#define OMCI_VOIP_CFG_DATA_PTOCOLS_USED_H248		2
#define OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_MGCP		3
#define OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NO_OMCI	0xFF

#define OMCI_VOIP_CFG_DATA_PROFILE_VERSION_NAME		"Name"

#define OMCI_VOIP_CFG_DATA_CFG_METHOD_NAME		"VoIPConfig"
#define OMCI_VOIP_CFG_DATA_CFG_METHOD_NOT_USED		0
#define OMCI_VOIP_CFG_DATA_CFG_METHOD_USED_OMCI		1
#define OMCI_VOIP_CFG_DATA_CFG_METHOD_USED_TR69		3

#define OMCI_VOIP_CFG_DATA_CFG_METHOD_NOTCFG_VAL	"NotCfg"
#define OMCI_VOIP_CFG_DATA_CFG_METHOD_OMCI_VAL	"OMCI"
#define OMCI_VOIP_CFG_DATA_CFG_METHOD_TR69_VAL	"TR69"

/*9.9.10 network dial plan table*/
#define OMCI_VOIP_NET_DIAL_PLAN_TBL_FORMAT_NO_DEFINED	0
#define OMCI_VOIP_NET_DIAL_PLAN_TBL_FORMAT_H248			1
#define OMCI_VOIP_NET_DIAL_PLAN_TBL_FORMAT_NSC			2
#define OMCI_VOIP_NET_DIAL_PLAN_TBL_FORMAT_VENDOR		3

#define OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN			30
#define OMCI_VOIP_NET_DIAL_PLAN_TBL_ADD_ACTION			1
#define OMCI_VOIP_NET_DIAL_PLAN_TBL_DEL_ACTION			0


/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
int getEntryIdBySipUserData(uint16 instId, uint16 *entryId);
int setSipDispName(char * val, uint16 entryId);
int setSipUserPass(uint16 instId, uint16 entryId);
int setSipURI(uint16 instId, uint16 entryId);
int updateSipInfoByPPTP(uint16 instId,uint16 entryId);


/*******************************************************************************************************************************
9.9.2 sip user data

********************************************************************************************************************************/
int omciMeInitForSipUserData(omciManageEntity_t *omciManageEntity_p);
int setSipUserDataUserPartAOR(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getSipUserDataDispName(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setSipUserDataDispName(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int setSipUserDataUserPass(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int setSipUserDataSipURI(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getSipUserDataExpTime(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setSipUserDataExpTime(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int setSipUserDataPPTPPointer(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
9.9.3 SIP agent config data

********************************************************************************************************************************/
#define VOIP_SIP_STATUTS_OK_INITIAL                 0
#define VOIP_SIP_STATUTS_CONNECTED                  1
#define VOIP_SIP_STATUTS_FAIL_ICMP_ERROR            2
#define VOIP_SIP_STATUTS_FAIL_MALFORMED_RES         3
#define VOIP_SIP_STATUTS_FAIL_INADEQUATE_INFO_RES   4
#define VOIP_SIP_STATUTS_FAIL_TIMEOUT               5
    
#define VOIP_SIP_SZ_STATUTS_OK_INITIAL                 "Ok/initial"
#define VOIP_SIP_SZ_STATUTS_CONNECTED                  "Connected"
#define VOIP_SIP_SZ_STATUTS_FAIL_ICMP_ERROR            "Failed-ICMP error"
#define VOIP_SIP_SZ_STATUTS_FAIL_MALFORMED_RES         "Failed-Malformed response"
#define VOIP_SIP_SZ_STATUTS_FAIL_INADEQUATE_INFO_RES   "Failed-Inadequate info response"
#define VOIP_SIP_SZ_STATUTS_FAIL_TIMEOUT               "Failed-Timeout"

#define VOIP_SIP_TR69_SZ_STATUS_UP              "Up"
#define VOIP_SIP_TR69_SZ_STATUS_INITING         "Initializing"
#define VOIP_SIP_TR69_SZ_STATUS_REGING          "Registering"
#define VOIP_SIP_TR69_SZ_STATUS_UNREGING        "Unregistering"
#define VOIP_SIP_TR69_SZ_STATUS_ERROR           "Error"
#define VOIP_SIP_TR69_SZ_STATUS_TESTING         "Testing"
#define VOIP_SIP_TR69_SZ_STATUS_QUIESCENT       "Quiescent"
#define VOIP_SIP_TR69_SZ_STATUS_DISABELED       "Disabled"


int omciMeInitForSIPAgentConfigData(omciManageEntity_t *omciManageEntity_p);
int Set_SipAgentCfgData_ProxyAddr(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int Set_SipAgentCfgData_OutBoundProxyAddr(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int Get_SipAgentCfgData_SipRegExpTime(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int Set_SipAgentCfgData_SipRegExpTime(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int Get_SipAgentCfgData_SIPReregTime(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int Set_SipAgentCfgData_SIPReregTime(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int Get_SipAgentCfgData_SIPStatus(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

int Set_SipAgentCfgData_SIPRegistrar(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int Set_SipAgentCfgData_TcpUdp(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
9.9.4 Voip voice CTP

********************************************************************************************************************************/
int omciMeInitForVoipVoiceCTP(omciManageEntity_t *omciManageEntity_p);

/*******************************************************************************************************************************
9.9.5 VoIP media profile

********************************************************************************************************************************/
int omciMeInitForVoipMediaPro(omciManageEntity_t *omciManageEntity_p);
int32 getVOIPMediaProFaxMode(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProFaxMode(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProCodecSele(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index);
int setVOIPMediaProCodecSele(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index);

int32 getVOIPMediaProPktPeriodSele(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index);
int setVOIPMediaProPktPeriodSele(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index);

int32 getVOIPMediaProSlience(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index);
int setVOIPMediaProSlience(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index);


int32 getVOIPMediaProCodecSele1st(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProCodecSele1st(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProPktPeriodSele1st(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProPktPeriodSele1st(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProSlience1st(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProSlience1st(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProCodecSele2nd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProCodecSele2nd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProPktPeriodSele2nd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProPktPeriodSele2nd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProSlience2nd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProSlience2nd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProCodecSele3rd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProCodecSele3rd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProPktPeriodSele3rd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProPktPeriodSele3rd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProSlience3rd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProSlience3rd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProCodecSele4th(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProCodecSele4th(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProPktPeriodSele4th(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProPktPeriodSele4th(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProSlience4th(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProSlience4th(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPMediaProOOBDtmf(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPMediaProOOBDtmf(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);


/*******************************************************************************************************************************
9.9.13: Rtp performance monitoring history data

********************************************************************************************************************************/
extern omciAttriDescript_t omciAttriDescriptListRtpPMHistoryData[];

int omciMeInitForRtpPMHistoryData(omciManageEntity_t *omciManageEntity_p);
int32 setRTPPMHistoryDataThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataRtpErrors(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataPacketLoss(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataMaxJitter(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataMaxTimeBetRTCP(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataNoRTCPPacket(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataTimeout(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataBufferUnderflows(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataBufferOverflows(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

/*******************************************************************************************************************************
9.9.14: SIP agent performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForSipAgentPMHistoryData(omciManageEntity_t * omciManageEntity_p);
int32 setSipAgentPMHistoryDataThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataRxInviteReqs(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRTPPMHistoryDataTxInviteReqs(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

/*******************************************************************************************************************************
9.9.15: SIP call initation performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForSipCallInitPMHistoryData(omciManageEntity_t * omciManageEntity_p);
int32 setSipCallInitPMHistoryDataThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getVoIPPMDataCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

extern omciAttriDescript_t omciAttriDescriptListSipAgentPMHistoryData[];
extern omciAttriDescript_t omciAttriDescriptListSipCallInitPMHistoryData[];

/*******************************************************************************************************************************
9.9.16 MGC config data


********************************************************************************************************************************/
int omciMeInitForMgcCfgData(omciManageEntity_t *omciManageEntity_p);

int setMGCCfgDataPriMGC(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
int setMGCCfgDataSecMGC(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getMGCCfgDataDevId(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMGCCfgDataDevId(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

#if 0
int32 getMGCCfgDataMsgFormat(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMGCCfgDataMsgFormat(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getMGCCfgDataMaxRetryTime(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setMGCCfgDataMaxRetryTime(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);
#endif

/*******************************************************************************************************************************
9.9.18 VoIP config data


********************************************************************************************************************************/
int omciMeInitForVoipCfgData(omciManageEntity_t *omciManageEntity_p);
int omciInitInstForVoipCfgData(void);
int omciInternalCreateForVoipCfgData(uint16 classId, uint16 instanceId);

int32 getVOIPCfgDataAvaiProto(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPCfgDataProtoUsed(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setVOIPCfgDataProtoUsed(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPCfgDataAvaVoipCfgMethods(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setVOIPCfgDataMethodsUsed(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getVOIPCfgDatProVersion(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);


/*******************************************************************************************************************************
9.9.10 network dial plan table

********************************************************************************************************************************/
int omciMeInitForNetDialPlanTbl(omciManageEntity_t *omciManageEntity_p);

int32 getNetDialPlanTblDialNumber(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

int setNetDialPlanTblDialFormat(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

int32 getNetDialPlanTblDialTbl(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int setNetDialPlanTblDialTbl(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag);

/*******************************************************************************************************************************
extern 

********************************************************************************************************************************/
extern alarm_id_map_entry_t alarmIdTableSipUserData[];
extern alarm_id_map_entry_t alarmIdTableMgcCfgData[];
extern alarm_id_map_entry_t alarmIdTableVoipCfgData[];
extern alarm_id_map_entry_t alarmIdTableSIPAgentConfigData[];

extern omciAttriDescript_t omciAttriDescriptListSipUserData[];
extern omciAttriDescript_t omciAttriDescriptListVoipMediaPro[];
extern omciAttriDescript_t omciAttriDescriptListVoipVoiceCTP[];
extern omciAttriDescript_t omciAttriDescriptListMgcCfgData[];
extern omciAttriDescript_t omciAttriDescriptListVoipCfgData[];
extern omciAttriDescript_t omciAttriDescriptListSIPAgentConfigData[];
extern omciAttriDescript_t omciAttriDescriptListNetDialPlanTbl[];

/*******************************************************************************************************************************
9.9.1 Physical path termination point POTS UNI

********************************************************************************************************************************/
extern omciAttriDescript_t	omciAttriDescriptListPptpPotsUNI[];
int32 getPptpPotsUNIAdminStatus(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 setPptpPotsUNIAdminStatus(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 setPptpPotsUNIARCValue(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 setPptpPotsUNIARCIntervalValue(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getPptpPotsUNIRxGain(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 setPptpPotsUNIRxGain(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getPptpPotsUNITxGain(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 setPptpPotsUNITxGain(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getPptpPotsUNIHookState(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int omciMeInitForPptpPotsUNI(omciManageEntity_t * omciManageEntity_p);
int omciInitInstForPPTPPotsUNI(void);

/*******************************************************************************************************************************
9.9.6 Voice Service profile

********************************************************************************************************************************/
extern omciAttriDescript_t omciAttriDescriptListVoIPAppSvcPro[];
int omciMeInitForVoiceServiceProfile(omciManageEntity_t *omciManageEntity_p);


/*******************************************************************************************************************************
9.9.7 RTP profile data

********************************************************************************************************************************/
extern omciAttriDescript_t omciAttriDescriptListRtpProfileData[];
int omciMeInitForRtpProfileData(omciManageEntity_t *omciManageEntity_p);


/*******************************************************************************************************************************
9.9.8 VoIP application service profile

********************************************************************************************************************************/
extern omciAttriDescript_t omciAttriDescriptListVoiceServiceProfile[];
int omciMeInitForVoipAppSvcProfile(omciManageEntity_t *omciManageEntity_p);


/*******************************************************************************************************************************
9.9.11: VoIP line status

********************************************************************************************************************************/
extern omciAttriDescript_t omciAttriDescriptListVoIPLineStatus[];
int32 getVoIPStatusDataCodecsUsed(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getVoIPLineStatusVoiceServerStatus(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getVoIPLineStatusPortSessionType(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getVoIPLineStatusCall1PacketPeriod(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getVoIPLineStatusCall2PacketPeriod(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int omciMeInitForVoIPLineStatus(omciManageEntity_t *omciManageEntity_p);
int omciInternalCreateActionForVoIPLineStatus(uint16 classId,uint16 instanceId);

/*
VoIP Call Statistics
*/
#if defined (TCSUPPORT_OMCI_ALCATEL) && defined (TCSUPPORT_VOIP)
extern omciAttriDescript_t omciAttriDescriptListVoIPCallStatistics[];
int omciMEInitForVoIPCallStatistics(omciManageEntity_t *omciManageEntity_p);
int32 getCallHistoryTable(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
#endif

/*******************************************************************************************************************************
9.9.12: Call Control performance monitoring history data

********************************************************************************************************************************/
enum omciCallCtrlPMIdx {
	PM_SETUP_FAILURE = 1,
    PM_SETUP_TIMER,
    PM_TERMINATE_FAILURE,
    PM_PORT_RELEASE,
    PM_PORT_OFFHOOK_FAILURE,     
};
enum omciCallCtrlThrholdIdx {
	THRHOLD_SETUP_FAILURE = 1,
    THRHOLD_SETUP_TIMER,
    THRHOLD_TERMINATE_FAILURE,
    THRHOLD_PORT_RELEASE,
    THRHOLD_PORT_OFFHOOK_FAILURE,     
};

#define CALL_CTRL_MAX_ATTRI_NUM		5

typedef struct ThrholdPmMap_s 
{
	uint 	threhold_idx;
    uint 	pm_idx;
}ThrholdPmMap_t;

extern omciAttriDescript_t omciAttriDescriptListCallCtrlPMHistory[];
int omciMeInitForCallCtrlPMHistory(omciManageEntity_t * omciManageEntity_p);
int32 setCallCtrlPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPMCommonFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
/*******************************************************************************************************************************
POTS test

********************************************************************************************************************************/
uint8 pptp_pots_uni_test_support(pthreadArgType *arg);
int omciPPTP_POTS_UNITestRstMsgHandleBaseline(pthreadArgType *pthreadArg);
int omciPPTP_POTS_UNITestRstMsgHandleExtended(pthreadArgType *pthreadArg);

#endif


