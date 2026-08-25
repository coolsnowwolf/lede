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
	omci.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	andy.Yi		2012/7/5	Create
*/
#ifndef _PMMGR_STRUCT_H_
#define _PMMGR_STRUCT_H_

#include "pmmgr_define.h"
#include <pthread.h>
/************************************************************
 Command Struct Definition
************************************************************/


/******************************************************************************************
*******************************************************************************************
 The database struct
*******************************************************************************************
******************************************************************************************/
/*
***********************************************************************************************************
Gpon performance monitors pm me struct
***********************************************************************************************************
*/

typedef struct FECPmcounters_s
{
	uint32 correctedBytes;
	uint32 correctedCodeWords;
	uint32 uncorrectableCodeWords;
	uint32 totalCodeWords;
	uint16 FECSeconds;
}FECPmcounters_t;

enum FECPmMask{
	MASK_correctedBytes 				= 1<<0,
	MASK_correctedCodeWords 			= 1<<1,
	MASK_uncorrectableCodeWords 		= 1<<2,
	MASK_totalCodeWords 				= 1<<3,
	MASK_FECSeconds 					= 1<<4
};

/*9.3.3 MAC bridge performance monitoring history data ME*/
typedef struct MACBridgePmCounters_s
{
	uint32 learnEntryDiscardCnts;
}MACBridgePmCounters_t;
/*9.3.9 mac bridge port performance monitoring history data ME*/
typedef struct MACBridgePortPmCounters_s
{
	uint32 forwardFrameCnts;
	uint32 delayExceededDiscardCnts;
	uint32 mtuExceededDiscardCnts;
	uint32 receivedFrameCnts;
	uint32 receivedDiscardCnts;
}MACBridgePortPmCounters_t;

/*9.3.30 ethernet frame performance monitoring history data upstream 
   or 9.3.31 ethernet frame performance monitoring history data downstream*/
typedef struct EthFramePmStreamCounters_s
{
	uint32 dropEvents;
	uint32 octets;
	uint32 packets;
	uint32 broadcastPkts;
	uint32 multicastPkts;
	uint32 crcErroredPkts;
	uint32 undersizePkts;
	uint32 oversizePkts;
	uint32 pkts64Octets;
	uint32 pkts127Octets;
	uint32 pkts255Octets;
	uint32 pkts511Octets;
	uint32 pkts1023Octets;
	uint32 pkts1518Octets;	
}EthFramePmStreamCounters_t;

typedef struct PPTPEthernetUNIPmcounters_s
{
	uint32 FCSErrors;
	uint32 excessiveCollisionCnts;
	uint32 lateCollisionCnts;
	uint32 framesTooLong;
	uint32 bufferOverFlowOnRcv;
	uint32 singleCollisionFrameCnts;
	uint32 multiCollisionFrameCnts;
	uint32 deferredTransmissionCnts;
	uint32 alignmentErrorCnts;
}PPTPEthernetUNIPmcounters_t;

typedef struct PPTPEthernetUNI2Pmcounters_s
{
	uint32 pppoeFilterCnt;
}PPTPEthernetUNI2Pmcounters_t;

typedef struct PPTPEthernetUNI3Pmcounters_s
{
	uint32 dropEvents;
	uint32 octets;
	uint32 packets;
	uint32 broadcastPkts;
	uint32 multicastPkts;
	uint32 undersizePkts;
	uint32 fragments;
	uint32 jabbers;
	uint32 pkts64Octets;
	uint32 pkts65To127Octets;
	uint32 pkts128To255Octets;
	uint32 pkts256To511Octets;
	uint32 pkts512To1023Octets;
	uint32 pkts1024To1518Octets;
}PPTPEthernetUNI3Pmcounters_t;

/*9.3.32 Ethernet Frame Extended PM ME*/

typedef struct EthernetFrameExtendedPmcounters_S
{
	uint32 dropEvents;
	uint32 octets;
	uint32 frames;
	uint32 broadcastFrames;
	uint32 multicastFrames;
	uint32 crcerroredFrames;
	uint32 undersizeFrames;
	uint32 oversizeFrames;
	uint32 frames64Octets;
	uint32 frames65To127Octets;
	uint32 frames128To255Octets;
	uint32 frames256To511Octets;
	uint32 frames512To1023Octets;
	uint32 frames1024To1518Octets;
}EthernetFrameExtendedPmcounters_t;


/*9.3.34 Ethernet Frame Extended 64bit PM ME*/

typedef struct EthernetFrameExtendedPm64bitcounters_s
{
	uint64 dropEvents;
	uint64 octets;
	uint64 frames;
	uint64 broadcastFrames;
	uint64 multicastFrames;
	uint64 crcerroredFrames;
	uint64 undersizeFrames;
	uint64 oversizeFrames;
	uint64 frames64Octets;
	uint64 frames65To127Octets;
	uint64 frames128To255Octets;
	uint64 frames256To511Octets;
	uint64 frames512To1023Octets;
	uint64 frames1024To1518Octets;
}EthernetFrameExtendedPm64bitcounters_t;



#ifdef TCSUPPORT_VOIP
typedef struct RTPDataPmcounters_s
{
	uint32 RtpErrors;
	uint32 PacketLoss;
	uint32 MaxJitter;
	uint32 MaxTimeBetRTCP;
	uint32 BufferUnderflows;
	uint32 BufferOverflows;
}RTPDataPmcounters_t;

typedef struct SipAgentPmcounters_s
{
    uint32 Transactions;
    uint32 RxInviteReqs;
    uint32 RxInviteRetrans;
    uint32 RxNoninviteReqs;
    uint32 RxNoninviteRetrans;
    uint32 RxResponse;
    uint32 RxResponseRetransmissions;
    uint32 TxInviteReqs;
    uint32 TxInviteRetrans;
    uint32 TxNonInviteReqs;
    uint32 TxNonInviteRetrans;
    uint32 TxResponse;
    uint32 TxResponseRetransmissions;
    
}SipAgentPmcounters_s_t;


typedef struct SipCallInitPmcounters_s
{
    uint32 FailedToConnect;
    uint32 FailedToValidate;
    uint32 Timeout;
    uint32 FailureReceived;
    uint32 FailedToAuthenticate;
    
}SipCallInitPmcounters_t;

typedef struct CallCtrlPmcounters_s
{
	uint32 SetupFailure;
	uint32 SetupTimer;
	uint32 TerminateFailure;
	uint32 PortRelease;
	uint32 PortOffhookTimer;
}CallCtrlPmcounters_t;
#endif

typedef struct IPHostCfgDataPmcounters_s
{
	uint32 icmpErrors;
	uint32 dnsErrors;
	uint16 dhcpTimeout;
	uint16 ipConflict;
	uint16 outOfMemory;
	uint16 internalError;
}IPHostCfgDataPmcounters_t;

typedef struct TcpUdpCfgDataPmcounters_s
{
	uint32 socketFailed;
	uint32 listenFailed;
	uint32 bindFailed;
	uint32 acceptFailed;
	uint32 selectFailed;
}TcpUdpCfgDataPmcounters_t;

typedef struct GALEthernetPmcounters_s
{
	uint32 discardedFrames;
}GALEthernetPmcounters_t;

typedef struct GemPortPmcounters_s
{
	uint32 lostPackets;
	uint32 misinsertedPackets;
	uint64 receivedPackets;
	uint64 receivedBlocks;
	uint64 transmittedBlocks;
	uint32 impairedBlocks;
}GemPortPmcounters_t;

#ifdef TCSUPPORT_OMCI_ALCATEL
typedef struct TotalGemPortPmcounters_s
{
	uint32 lostFragmentDown;
	uint32 lostFragmentUp;
	uint64 receivedFragment;
	uint64 receivedBlocks;
	uint64 transmittedBlocks;
	uint32 impairedBlocks;
	uint64 transmittedFragment;
	uint32 badReceivedPackets;
	uint64 receivedBlocks64;
	uint64 transmittedBlocks64;
}TotalGemPortPmcounters_t;

typedef struct EthernetTrafficPmcounters_s
{
	uint32 UpstreamFrames;
	uint32 DownstreamFrames;
	uint32 UpstreamBytes;
	uint32 DownstreamBytes;
	uint32 DropUpstreamFrames;
	uint32 DropUpstreamBytes;
	uint32 DropDownstreamFrames;
	uint32 MulticastUpstreamFrames;
	uint32 MulticastDownstreamFrames;
	uint64 UpstreamBytes64;
	uint64 DownstreamBytes64;
	uint64 UpstreamFrames64;
	uint64 DownstreamFrames64;
}EthernetTrafficPmcounters_t;

typedef struct GEMPortProtocolPmcounters_s
{
	uint32 lostGEMFragmentCounter;
	uint32 transmittedGEMFragmentCounter;
}GEMPortProtocolPmcounters_t;
#endif


/*
***********************************************************************************************************
Gpon performance monitors database struct
***********************************************************************************************************
*/

typedef struct pmmgrPmAttriDescript_s
{
	char 	name[32];                                                                /*pm me attribute name*/
	uint8 	length;										 /*pm me attribute length, for example, 1(uint8), 2(uint16),4(uint32)...*/
	uint8 	offset;										 /*pm me attribute in struct Pmcounters_s offset*/
	uint32 	tcaId;										/*pm me attribute need send the tca, the TCA ID*/
	int(*getDataFromApi)(uint8 *data, uint16 portId);
}pmmgrPmAttriDescript_t, pmmgrPmAttriDescript_ptr;

typedef struct pmmgrPmInst_s
{
	uint16 portId;                                                                            /*pm in this portId(ethernet port, gem port....)*/
	uint8 enable;                                                                           /*this pm function is open(1) or close(0)*/
	uint32 tcaSendMask;										/* the inst have send a tca in the cycle time*/
	uint8 *pmStartData;             	   		 /*15 min timer start statistics data*/
	uint8 *pmCurrentData;               		/*current statistics data*/
	uint8 *pmHistoryData;                		/*history statistics data*/
	uint8 *pmThreshold;                   		/*threshold setting*/
	struct pmmgrPmInst_s *next;                         /*next pm*/
}pmmgrPmInst_t, *pmmgrPmInst_ptr;

typedef struct pmmgrPmMe_s
{
	uint16 	classId;
	char 	name[32];
	int (*pmmgrPmInitFunc)(struct pmmgrPmMe_s* me_p);
	pthread_mutex_t  *pmmgrMutex;					/*pmgr instance list mutex, when process inst list, will lock this mutes*/
	struct 	pmmgrPmInst_s *pmmgrInstList; 
	uint8 	attriNum;							
	uint16 	attriTotalSize;						/*the sizeof(struct xxPmcounters_s)*/
	struct 	pmmgrPmAttriDescript_s *pmmgrAttriDescriptList;
	int(*getAllDataFromApi)(uint8 *data, uint16 portId);
}pmmgrPmMe_t, *pmmgrPmMe_ptr; 

typedef struct tmpMeInstance{
	pmmgrPmMe_ptr tmpme;
	pmmgrPmInst_ptr tmpins;
}tmpMeInstance_t;

enum gponAlarmMask{
	MASK_sfThreshold 				= 1<<0,
	MASK_sdThreshold 				= 1<<1,
	MASK_lowerOpticalThreshold 		= 1<<2,
	MASK_upperOpticalThreshold		= 1<<3,
	MASK_lowerTxPowerThreshold 	= 1<<4,
	MASK_upperTxPowerThreshold 	= 1<<5
};

typedef struct gponAlarmConfig_s
{
	uint8 sfThreshold;
	uint8 sdThreshold;
	uint8 lowerOpticalThreshold;
	uint8 upperOpticalThreshold;
	uint8 lowerTxPowerThreshold;
	uint8 upperTxPowerThreshold;
}gponAlarmConfig_t, *gponAlarmConfig_ptr;

typedef struct gponAlarmStatus_s
{
	int AlarmID_RXPower;
	int AlarmID_TXPower;
}gponAlarmStatus_t;

typedef struct pmmgrGponDatabase_s{
	uint8 enable;
	uint8 alarmEnable;
	int timerId;
	uint16 times;									/*running how many seconds in a cycle time*/
	struct pmmgrPmMe_s *pmMeTable;
	struct gponAlarmConfig_s gponAlarmCfg;
}pmmgrGponDatabase_t;


/*
***********************************************************************************************************
Epon performance monitors struct
***********************************************************************************************************
*/

typedef struct PMEponControlType_s
{
    uint8   enable;                 /*PM 1:open or 0:close*/
    uint16  cycleTime;              /*PM cycle time*/
    uint16  times;                  /*seconds*/
    int     timerId;                /*timer id*/
    uint32  AlarmEnableMask;        /*alarm enable mask, bit is 1: enable, bit is 0: disable*/
    uint32  WarningEnableMask;
    uint32  AlarmSendMask;          /*send a alarm in a cycle?*/
    uint32  WarningSendMask;
}PMEponControl_t, *PMEponControl_ptr;


typedef struct PMEponCounters_s
{
    long long downstreamDropEvents;
    long long upstreamDropEvents;
    long long downstreamOctets;
    long long upstreamOctets;
    long long downstreamFrames;
    long long upstreamFrames;
    long long downstreamBroadcastFrames;
    long long upstreamBroadcastFrames;
    long long downstreamMulticastFrames;
    long long upstreamMulticastFrames;
    long long downstreamCrcErroredFrames;
    long long upstreamCrcErroredFrames;
    long long downstreamUndersizeFrames;
    long long upstreamUndersizeFrames;
    long long downstreamOversizeFrames;
    long long upstreamOversizeFrames;
    long long downstreamFragments;
    long long upstreamFragments;
    long long downstreamJabbers;
    long long upstreamJabbers;
    long long downstream64Frames;
    long long upstream64Frames;
    long long downstream127Frames;
    long long upstream127Frames;
    long long downstream255Frames;
    long long upstream255Frames;
    long long downstream511Frames;
    long long upstream511Frames;
    long long downstream1023Frames;
    long long upstream1023Frames;
    long long downstream1518Frames;
    long long upstream1518Frames;
    long long downstreamDiscards;
    long long upstreamDiscards;
    long long downstreamErrors;
    long long upstreamErrors;
    long long statusChangeTimes;
    unsigned int highRxPower;
    unsigned int lowRxPower;
    unsigned int highTxPower;
    unsigned int lowTxPower;
    unsigned int highTxCurrent;
    unsigned int lowTxCurrent;
    unsigned int highSupplyVoltage;
    unsigned int lowSupplyVoltage;
    int highTemprature;
    int lowTemprature;
}PMEponCounters_t, *PMEponCounters_ptr;


// the pm data is increased, need no clear threshold
typedef struct pmmgrEponDatabase_s{
	PMEponCounters_t PMStartData[MAX_PORT_NUM];   		/*cycle timer start statistics data*/
	PMEponCounters_t PMHistoryData[MAX_PORT_NUM];		/*history statistics data*/
	PMEponCounters_t PMCurrentData[MAX_PORT_NUM];		/*current statistics data*/
	PMEponCounters_t PMAlarmThreshold[MAX_PORT_NUM];	/*alarm threshold*/
	PMEponCounters_t PMAlarmClearThreshold[MAX_PORT_NUM];	/*alarm clear threshold*/
	PMEponCounters_t PMWarningThreshold[MAX_PORT_NUM];	/*warning threshold*/
	PMEponCounters_t PMWarningClearThreshold[MAX_PORT_NUM];	/*warning clear threshold*/
	PMEponControl_t   PMEponControl[MAX_PORT_NUM];
}pmmgrEponDatabase_t;


typedef struct alarmIdIndex_s
{
	uint16 index;
	uint16 alarmId;
}alarmIdIndex_t, *alarmIdIndex_ptr;

/*typedef struct oamAlarmMsg_s
{
	long msg_type;
	unsigned short trapIndex; //Alarm ID
	unsigned short devIndex_1; //port ID
	unsigned short devIndex_2; //LLID
	unsigned short devIndex_3; //PON IF
	unsigned char trapOtherInfo[100]; //Alarm Info
}OamAlarmMsg_t, *OamAlarmMsg_Ptr;
*/
/***********************************************************************************************************
GEM Port network performance monitors pm me struct
***********************************************************************************************************
*/
typedef struct GEMPort_CTP_Pmcounters_s
{
	uint32 transmitGEMFrames;
	uint32 receiveGEMFrames;
	uint64 receivePayloadBytes;
	uint64 transmitPayloadBytes;
}GEMPort_CTP_Pmcounters_t;

enum EMPort_CTP_PmMask{
	MASK_transmitGEMFrames 			= 1<<0,
	MASK_receiveGEMFrames 			= 1<<1,
	MASK_receivePayloadBytes			= 1<<2,
	MASK_transmitPayloadBytes			= 1<<3
};

/***********************************************************************************************************
Gpon performance monitors pm me struct
***********************************************************************************************************
*/
typedef struct EnergyConsumed_Pmcounters_s
{
	uint32 dozeTime;
	uint32 cyclicSleepTime;
	uint32 energyConsumed;
}EnergyConsumed_Pmcounters_t;


/***********************************************************************************************************
EPON MSG API Struct
***********************************************************************************************************
*/

#define PMMGR_MQ_PATH "/tmp/pmq"
#define PMMGR_PROJID	30

#define PMMGR_MSG_SND   1
#define PMMGR_MSG_RCV   2

#define PMMGR_CT_GET_ENABLE		1
#define PMMGR_CT_SET_ENABLE		2
#define PMMGR_CT_GET_STATE			3
#define PMMGR_CT_SET_STATE			4
#define PMMGR_CT_GET_THRSHLD		5
#define PMMGR_CT_SET_THRSHLD		6
#define PMMGR_CT_GET_CYCLE_TIME  	7
#define PMMGR_CT_SET_CYCLE_TIME  	8
#define PMMGR_CT_GET_PMDATA		9
#define PMMGR_CT_SET_PMDATA		10
#define PMMGR_CT_GET_PMHSTYDATA	11
#define PMMGR_CT_INIT_EPON 		12

typedef struct pmmgr_msg{
	long 	msg_type;
	uint8      cmd_type;
	char   	cmd_ret;
	uint8 	port_id;
	char  	port_state;
	uint8 	alarm_state;
	uint32  	alarm_id;
	uint32 	alarm_set_threshold;
	uint32	alarm_clear_threshold;

	uint32 	cycle_time;
	PMEponCounters_t pm;
}Pmmgr_msg_t, *Pmmgr_msg_ptr;


#endif /* _PMGR_STRUCT_H_ */


