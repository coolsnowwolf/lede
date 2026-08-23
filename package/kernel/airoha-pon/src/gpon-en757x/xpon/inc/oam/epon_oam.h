/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    epon_oam.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    JQ.Zhu  2012-06-06      Initial version
*/

#ifndef __EPON_OAM
#define __EPON_OAM
#include "epon_oam_types.h"
#include "epon_oam_util.h"
#include "epon_oam_timer.h"
#include "epon_oam_cmd.h"

#define INFO_OAM   "Info_Oam"
#define OAM_ENTRY "Oam_Entry"
#define OAM_DEBUG_LEVEL "DebugLevel"
#define OAM_DEBUG_LLID_MASK "DebugLlidMask"
#define OAM_LAN_DEBUG "LanDbg"
#define OAM_OPERATOR_ID "OperatorId"
#define OAM_LATEST_VERSION "LatestVer"

#define	OAM_AUTH_TIME	"authtime"

#define LINK_UP 1
#define LINK_DOWN 0
#define OAM_LINK_STAT_PATH "/tmp/oam_stat"


#ifdef TCSUPPORT_PONMGR
#define PONCOMMON_ATTR_XPON_PHY_STATUS	"phyStatus"
#define PONCOMMON_XPON_PHY_DOWN			"down"
#define PONCOMMON_XPON_PHY_GPON_DOWN	"gpon_phy_down"
#define PONCOMMON_XPON_PHY_GPON_UP		"gpon_phy_up"
#define PONCOMMON_XPON_PHY_EPON_DOWN	"epon_phy_down"
#define PONCOMMON_XPON_PHY_EPON_UP		"epon_phy_up"

#define PONCOMMON_ATTR_DATA_TRAFFIC_STA  	"trafficStatus"
#define ONU_WAN_DATA_LINK_UP 				"up"
#define ONU_WAN_DATA_LINK_DOWN 				"down"

#define PON_LINK_CFG_ATTR_XPON_MODE  	"Mode"
#define PONCOMMON_ATTR_XPON_LINK_STA  	"LinkSta"
#endif /* TCSUPPORT_PONMGR */

#define EPON_OAM_PIDFILE "/var/run/epon_oam.pid"
#define EPON_MAX_OAMPDU_SIZE	1536
#define EPON_LLID_MAX_NUM		8
#define EPON_OAM_VERSION		0x01
#define EPON_OAM_OUI			0x000f3e//fake OUI for ZTE

#define EPON_OAM_ETH_TYPE		0x8809
#define EPON_OAM_SUBTYPE		0x03


/*  refer to 802.3ah CH.57 */
/* OAM discover */
#define EPON_OAM_DISC_STATE_FAULT						0
#define EPON_OAM_DISC_STATE_PASSIVE_WAIT				1
#define EPON_OAM_DISC_STATE_SEND_LOCAL_REMOT		2
#define EPON_OAM_DISC_STATE_SEND_LOCAL_REMOT_OK	3
#define EPON_OAM_DISC_STATE_SEND_ANY					4


/* OAM transmit FSM state */
#define EPON_OAM_TRANS_STATE_RESET			0
#define EPON_OAM_TRANS_STATE_WAIT_TX			1
#define EPON_OAM_TRANS_STATE_DEC_PDU_CNT	2
#define EPON_OAM_TRANS_STATE_TX_OAMPDU		3


/* OAM multiplexer FSM */
#define EPON_OAM_MLTPLX_STATE_WAIT_TX			0
#define EPON_OAM_MLTPLX_STATE_TX_FRAME			1
#define EPON_OAM_MLTPLX_STATE_CHECK_PHY_LINK	2


typedef struct eponOamDisc_s{
	u_char state;
}eponOamDisc_t, *eponOamDisc_p;

/* OAM transmit */
typedef struct eponOamTransmit_s{
	u_char state;
}eponOamTransmit_t, *eponOamTransmit_p;

/* OAM control */
typedef struct eponOamCtrl_s{
	eponOamDisc_t eponOamDiscover;
	eponOamTransmit_t eponOamTransmit;
}eponOamCtrl_t, *eponOamCtrl_p;

/* Oam Multiplexer */
typedef struct eponOamMultiplex_s{
	u_char state;
}eponOamMultiplex_t, *eponOamMultiplex_p;

/* Oam parser */
typedef struct eponOamParser_s{
	u_char state;
}eponOamParser_t, *eponOamParser_p;


//EPON MUX ACTION
#define EPON_MUX_ACT_FWD   	0
#define EPON_MUX_ACT_DISCARD  1

//EPON OAM MODE
#define EPON_OAM_MODE_PASSIVE 	0
#define EPON_OAM_MODE_ACTIVE	  	1


//EPON OAM PAR ACTION
#define EPON_OAM_PAR_ACT_FWD		0
#define EPON_OAM_PAR_ACT_LB		1
#define EPON_OAM_PAR_ACT_DISCARD	2


//EPON OAM LOCAL_PDU
#define EPON_OAM_LF_INFO			0
#define EPON_OAM_RX_INFO			1
#define EPON_OAM_INFO				2
#define EPON_OAM_ANY				3


//EPON OAM pduReq
#define EPON_OAM_NONE				0
#define EPON_OAM_CRITICAL			1
#define EPON_OAM_NORMAL			2


typedef union{
	struct{
		u_short reserved:5;
		u_short maxOamPduSize:11;
	}bits;
	u_short value;
}oamPduCfg_t;


///////////OAMPDU ////////////////////
typedef  union{
	struct{
		u_char reserved1: 8;
		u_char reserved2:1;
		u_char remoteStable:1;
		u_char remoteEvlt:1;
		u_char localStable:1;
		u_char localEvlt:1;
		u_char criticlEvent:1;
		u_char dyingGasp:1;
		u_char linkFault:1;
	}bits;
	u_short value;
}oamPduHdr_Flags_t;


/* OAMPDU CODE define */
#define EPON_OAM_HDR_CODE_INFO			0x00
#define EPON_OAM_HDR_CODE_EVT_NOTIFY	0x01
#define EPON_OAM_HDR_CODE_VAR_REQ		0x02
#define EPON_OAM_HDR_CODE_VAR_RESP		0x03
#define EPON_OAM_HDR_CODE_LB_CTRL		0x04
#define EPON_OAM_HDR_CODE_ORG_SPEC		0xFE
#define EPON_OAM_HDR_CODE_RSV			0xFF


/******OAMPDU header********/
typedef struct oamPduHdr_s {
    u_char daddr[6];
    u_char saddr[6];
    u_short ethtype;
    u_char subType;
    oamPduHdr_Flags_t flags;
    u_char code;

} __attribute__ ((packed)) oamPduHdr_t;


/* OAM PDU define */
typedef struct oamPacket_s{
	u_char fromLan;
	u_int pduLen;
	u_int rawLen;
	u_char llidIdx;//0~7
	u_char eponExtendOam;// TRUE or FALSE
	u_char *currentPtr;
	u_char pduReq;
	u_char validPduReq;
	u_char  *data;
	u_char rawData[EPON_MAX_OAMPDU_SIZE];
	u_char fcs[4];
}oamPacket_t, *oamPacket_p;


/**info TLV**/
#define EPON_OAM_TLV_INFO_TP_END				0x00
#define EPON_OAM_TLV_INFO_TP_LOCL_INFO		0x01
#define EPON_OAM_TLV_INFO_TP_REMT_INFO		0x02
#define EPON_OAM_TLV_INFO_TP_ORG_SPEC_INFO	0xFE

/* TLV state */
typedef union{
	struct{
		u_char reserve:5;
		u_char muxAction:1;
		u_char parAction:2;
	}bits;
	u_char value;
}oamTlvSts_t;

/* oamConfig */
typedef  union{
	struct{
		u_char reserve:3;
		u_char varRetri:1;
		u_char linkEvt:1;
		u_char rmtLpbk:1;
		u_char unidirectional:1;
		u_char oamMode:1;
	}bits;
	u_char value;
}oamTlvCfg_t;


typedef struct oamInfoTlv_s{
	u_char infoType;
	u_char infoLen;
	u_char oamVer;
	u_short revision;
	oamTlvSts_t state;
	oamTlvCfg_t oamConfig;
	u_short oampduConfig;
	u_char oui[3];
	u_char vendorSepcInfo[4];
}__attribute__ ((packed)) oamInfoTlv_t;


/** link Event TLV **/
/////////EVT notification TYPE////////////
#define EPON_OAM_TLV_EVT_END						0x00
#define EPON_OAM_TLV_EVT_ERR_SYM_PERIOD			0x01//Errored Symbol Period Event
#define EPON_OAM_TLV_EVT_ERR_FRAME				0x02//Errored Frame Event
#define EPON_OAM_TLV_EVT_ERR_FRAME_PERIOD		0x03//Errored Frame Period Event
#define EPON_OAM_TLV_EVT_ERR_FRAME_SEC_SUMM	0x04//Errored Frame Seconds Summary Event
#define EPON_OAM_TLV_EVT_ORG_SPEC				0xFE//Organization Specific Event


typedef struct oamLinkEvtTlvErrSymPerid_s{
	u_char 	 evtType;
	u_char 	 evtLen;
	u_short 	 errSymevtTimeStamp;
	u_int64_t errSymWindow;
	u_int64_t errSymThreshold;
	u_int64_t errSymError;
	u_int64_t errSymRunTotal;
	u_int 	 errSymEvtRunTotal;
}__attribute__ ((packed)) oamLinkEvtTlvErrSymPerid_t;


typedef struct oamLinkEvtTlvErrFrame_s{
	u_char evtType;
	u_char evtLen;
	u_short evtTimeStamp;
	u_short errFrameWindow;
	u_int errFrameThreshold;
	u_int errFrames;//Errored Frames
	u_int64_t errFrameRunTotal;//Error Running Total.
	u_int errFrameEvtRunTotal;//Event Running Total.
}__attribute__ ((packed)) oamLinkEvtTlvErrFrame_t;


typedef struct oamLinkEvtTlvErrFramePeriod_s{
	u_char 	evtType;
	u_char 	evtLen;
	u_short 	evtTimeStamp;
	u_int 	errFramePeriodWindow;
	u_int 	errFramePeriodThreshold;
	u_int 	errFramesPeriod;//Errored Frames
	u_int64_t errFramePeriodRunTotal;//Error Running Total.
	u_int 	errFramePeriodEvtRunTotal;//Event Running Total.
}__attribute__ ((packed)) oamLinkEvtTlvErrFramePeriod_t;


typedef struct oamLinkEvtTlvErrFrameSecSumm_s{
	u_char 	evtType;
	u_char 	evtLen;
	u_short 	evtTimeStamp;
	u_short 	errFrameSecWindow;//Errored Frame Seconds Summary Window.
	u_short 	errFrameSecThreshold;
	u_short 	errFrameSecSumm;//Errored Frame Seconds Summary.
	u_int 	errFrameSecRunTotal;//Error Running Total.
	u_int 	errFramePeriodEvtRunTotal;//Event Running Total.
}__attribute__ ((packed)) oamLinkEvtTlvErrFrameSecSumm_t;


typedef struct oamErrCounter_s{
	// ErrSymPerid
	u_short 		errSymevtTimeStamp;
	u_int64_t 	errSymWindow;
	u_int64_t 	errSymThreshold;
	u_int64_t 	errSymError;
	u_int64_t 	errSymRunTotal;
	u_int 		errSymEvtRunTotal;

	// ErrFrame
	u_short 		errFrameTimeStamp;
	u_short 		errFrameWindow;
	u_int 		errFrameThreshold;
	u_int 		errFrames;//Errored Frames
	u_int64_t 	errFrameRunTotal;//Error Running Total.
	u_int 		errFrameEvtRunTotal;//Event Running Total.

	// ErrFramePeriod
	u_short 		errFramePeriodevtTimeStamp;
	u_int 		errFramePeriodWindow;
	u_int 		errFramePeriodThreshold;
	u_int 		errFramesPeriod;//Errored Frames
	u_int64_t 	errFramePeriodRunTotal;//Error Running Total.
	u_int 		errFramePeriodEvtRunTotal;//Event Running Total.

	// ErrFrameSecSumm
	u_short 		errFrameSecSummevtTimeStamp;
	u_short 		errFrameSecWindow;//Errored Frame Seconds Summary Window.
	u_short 		errFrameSecThreshold;
	u_short 		errFrameSecSumm;//Errored Frame Seconds Summary.
	u_int 		errFrameSecRunTotal;//Error Running Total.
	u_int 		errFrameSecEvtRunTotal;//Event Running Total.
}oamErrCounter_t;


/////////variable descriptor///////////
typedef struct oamVarDesptor_s{
	u_char 	varBranch;
	u_short 	varLeaf;
}__attribute__ ((packed)) oamVarDesptor_t;


/////////Variable Branch///////////
#define EPON_OAM_VAR_END				0x00
#define EPON_OAM_VAR_BRANCH_ATTR		0x07
#define EPON_OAM_VAR_BRANCH_OBJ		0x03
#define EPON_OAM_VAR_BRANCH_PKT		0x04


/////////Variable Leaf/////////////////////////
#define EPON_VAR_LEAF_OAMID				236
#define EPON_VAR_LEAF_OAM_ADMIN_STATE	237
#define EPON_VAR_LEAF_OAM_MODE			238
#define EPON_VAR_LEAF_OAM_DISCV_STATE	333


////////Variable indication/////////////////////////
//0x00 Reserved - shall not be transmitted, should be ignored on reception by OAM client
#define EPON_VAR_INDICT_EXCED_PDU_DATA			0x01//0x01 Length of requested Variable Container(s) exceeded OAMPDU data field.
//0x02 to 0x1F Reservedshall not be transmitted, should be ignored on reception by OAM client
//Attribute Indications
//0x20 Requested attribute was unable to be returned due to an undetermined error.
#define EPON_VAR_INDICT_ATTR_NOT_SUPORT			0x21//0x21 Requested attribute was unable to be returned because it is not supported by the local DTE.
#define EPON_VAR_INDICT_ATTR_CORRUPT_BY_RESET	0x22//0x22 Requested attribute may have been corrupted due to reset.
#define EPON_VAR_INDICT_ATTR_HW_FAIL				0x23//0x23 Requested attribute unable to be returned due to a hardware failure.
#define EPON_VAR_INDICT_ATTR_OVER_FLOW			0x24//0x24 Requested attribute experienced an overflow error.
//0x25 to 0x3F Reservedshall not be transmitted, should be ignored on reception by OAM client
//Object Indications
#define EPON_VAR_INDICT_OBJ_END					0x40//0x40 End of object indication.
#define EPON_VAR_INDICT_OBJ_UNDETERMIN_ERROR	0x41//0x41 Requested object was unable to be returned due to an undetermined error.
#define EPON_VAR_INDICT_OBJ_NOT_SUPPORT			0x42//0x42 Requested object was unable to be returned because it is not supported by the local DTE.
#define EPON_VAR_INDICT_OBJ_CORRUPT_BY_RESET	0x43//0x43 Requested object may have been corrupted due to reset.
#define EPON_VAR_INDICT_OBJ_CORRUPT_HW_FAIL		0x44//0x44 Requested object unable to be returned due to a hardware failure.
//0x45 to 0x5F Reservedshall not be transmitted, should be ignored on reception by OAM client
//Package Indications
#define EPON_VAR_INDICT_PKT_END					0x60//0x60 End of package indication.
#define EPON_VAR_INDICT_UNDETERMIN_ERROR			0x61//0x61 Requested package was unable to be returned due to an undetermined error.
#define EPON_VAR_INDICT_NOT_SUPPORT				0x62//0x62 Requested package was unable to be returned because it is not supported by the local DTE.
#define EPON_VAR_INDICT_CORRUPT_BY_RESET			0x63//0x63 Requested package may have been corrupted due to reset.
#define EPON_VAR_INDICT_HW_FAIL					0x64//0x64 Requested package unable to be returned due to a hardware failure.
//0x65 to 0x7F Reservedshall not be transmitted, should be ignored on reception by OAM client

#define EPON_MPCP_STATE_INIT	0
#define EPON_MPCP_STATE_REG	1
#define EPON_MPCP_STATE_NACK	2
#define EPON_MPCP_STATE_DREG	3

/*** EPON OAM main struct  **/
typedef struct eponOamLlid_s{

	u_char 	llidIdx;
	//EPON LLID global param
	u_char 	enableFlag;//
	u_char	mpcpState; // the llid 's mpcp state
	u_char 	macAddr[6];
	u_short 	llid;
	u_int 	rxCount;
	u_int 	txCount;

	//run time variables
	u_int 	localOamId;
	u_char 	begin;
	//int indSubType;  // subtype of oam is 0x03

	// local status & config
	u_char 			localCriticalEvent;
	u_char 			localDyingGasp;
	u_char 			localLinkStatus;

	u_char 			localOamEnable:2;
	u_char 			localPdu;			// the Rx&Tx action status
	u_char 			localSatisfied;

	u_char 			localStable;
	u_char 			localOui[3];
	u_char 			localVendorSpecInfo[4];
	oamTlvSts_t  		localState;
	oamTlvCfg_t 		localCfg;
	oamPduCfg_t 		localOamPduCfg;
	u_short 			localRevision;
	u_short 			localEvtSeqNum;
	oamErrCounter_t 	localErrCnt;

	// remote status & config
	u_char 			remoteEvlt;
	u_char 			remoteStateValid;
	u_char 			remoteOamVer;

	u_char 			remoteStable;
	u_char 			remoteOui[3];
	u_char 			remoteVendorSpecInfo[4];
	oamTlvSts_t  		remoteState;
	oamTlvCfg_t  		remoteCfg;
	oamPduCfg_t 		remoteOamPduCfg;
	u_short 			remoteRevision;
	u_short 			remoteEvtSeqNum;
	oamErrCounter_t 	remoteErrCnt;

	u_char pduReq;
	u_char validPduReq;

	u_char pduCnt; // the count of oam pdu we send in 1's.
	u_char pduTimerDone;
	u_char localLostLinkTimerDone;

	//timer
	int  pduTimer;
	int  localLostLinkTimer;

	//OAM param
	eponOamCtrl_t 		eponOamCtrl;
	eponOamMultiplex_t 	eponOamMultiplex;
	eponOamParser_t 		eponOamParser;

	int 		lastKeyInUse ;
	u_char	lastSettingKey[3];
	u_char 	lastSettingKey2[3];
	u_char	lastSettingKey3[3] ;
}eponOamLlid_t, *eponOamLlid_p;

//OAM MIB struct
typedef struct eponOamVar_s{
	u_char varBranch;
	u_short varLeaf;
	u_char width;//0xff is variable length
	//callback function
	u_char (*oamGetFunc)(IN struct eponOamLlid_s *pOam , OUT u_char *width , OUT struct oamPacket_s *pTxOamPkt);
}eponOamVar_t;


typedef struct eponOam_s{
	u_int llidMask;
	u_int rxCnt;
	u_int txCnt;
	u_int state;
	eponOamLlid_t eponOamLlid[EPON_LLID_MAX_NUM];
}eponOam_t, *eponOam_p;

/* **********************EPON OAM CFG STRUCT ********************** */
#define EPON_OAM_LOST_LINK_TIME 5000 // 5s
#define EPON_OAM_MAX_PDU_NUM_PER_SEC	10000

#define EPON_OAM_TESTMASK_TMPMAC 1
#define EPON_OAM_TESTMASK_TMPLOID 2


#define MAX_SUPPORT_OPERATOR_VER_CNT 32
#define MAX_SUPPORT_OPERATOR_CNT 32

typedef struct
{
    u_char verCnt;
    u_char data[MAX_SUPPORT_OPERATOR_VER_CNT];
}operVersion_t;

typedef struct {
    u_char latestVer;
    u_char id;
    operVersion_t vers[MAX_SUPPORT_OPERATOR_CNT];
}operatorInfo_t;

typedef struct eponOamCfg_s{
	u_char onuMacAddr[6];
	u_char dstMacAddr[6];
	u_char isHgu;	// default is SFU:FALSE
	u_char dbgLvl;
	u_char dbgLlidMask;
	u_char isInfoPduSent; // True / False
	u_char isExtOrg;
	u_int   lostLinkTime;	// default is 5000ms
	u_int   maxPduPerSec;	// default is 10pps

	u_int tmpTestMask;

	u_char isLanDbgOn; // default is False

    operatorInfo_t operators;
}eponOamCfg_t, *eponOamCfg_p;

/* **********************EPON OAM CFG STRUCT ********************** */
extern eponOam_t eponOam;
extern eponOamCfg_t eponOamCfg;

extern void tcdbg_printf(char *fmt,...);
int getOamVarTableSize(void);
oamPacket_t *eponOamAllocPkt(void);
OUT int eponOamInitHeader(OUT oamPduHdr_t *newPduHdr, IN eponOamLlid_t *pOam, IN u_char code);
int eponOamLlidStop(u_char llidIdx);
int eponOamLlidInit(u_char llidIdx);
void eponOamExit(int sig);
void eponOamRestart();
int eponOamShowStatus(void);
int eponOamDumpLlidPkt(oamPacket_p pOamPkt, int dir);
int eponOamDumpPkt(oamPacket_p pOamPkt, int dir);
int eponOamTransFsm(eponOamLlid_t *pOamLlid ,  oamPacket_t *pOamPkt);
int eponOamFreePkt(oamPacket_t *pOamPkt);
int eponOamStopPduTimer(u_char llidIdx);
int eponOamSendNormalPdu(u_char llidIdx , u_char count);
int eponOamSendCriticalPdu(u_char llidIdx , u_char count);
int eponOamSetLocalSatisfy(u_char llidIdx , u_char satisfy);
int eponOamSendDyGaspPdu(u_char llidIdx , u_char type, u_char count);
int eponOamPktTx(oamPacket_t *pOamPkt, u_char llidIdx);

int eponOamPduTmout(void *param);
int eponOamLostLinkTmout(void *param);
int eponOamExtendCodeHandler(struct oamPacket_s *pOamPkt);
int eponOamExtendEvtNotifyHandler(struct oamPacket_s *pOamPkt);
int eponOamExtendInfoHandle(struct oamPacket_s *pOamPkt);
oamPacket_t *eponOamGenNotifyPkt(eponOamLlid_t *pOamLlid);
oamPacket_t *eponOamGenInfoPkt(eponOamLlid_t *pOamLlid , u_char remoteFlag);

int tcapi_get(char* node, char* attr, char *retval);
int tcapi_set(char *node,char * attr, char *value);

enum{
    CTCC,
    CUCC,

    /*add new operator id above this line!*/
    OPERATOR_NUM
};

#endif//end of __EPON_OAM
