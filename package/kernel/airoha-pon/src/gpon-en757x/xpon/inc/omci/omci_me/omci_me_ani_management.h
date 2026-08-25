
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
	9_12_ME.h
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/8/07	Create
	Andy.Yi		        2012/8/07	add 9.2.1,9.2.2
*/


#ifndef _9_2_ME_H_
#define _9_2_ME_H_
#include "omci_types.h"
#include "omci_me.h"

int omciMeInitForAniG(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForTcont(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForGemPortCtp(omciManageEntity_t *omciManageEntity_p);
int omciInitInstForAniG(void);
int omciInitInstForTcont(void);

int omciPhyAlarmHandler(uint32 trapValue);
int32 setGEMBlockLengthValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setSFThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setSDThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setANIGARCValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setANIGARCIntervalValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getOpticalSignalLevelValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setLowerOpticalThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setUpperOpticalThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTransmitOpticalLevelValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setLowerTransmitOpticalThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setUpperTransmitOpticalThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 setTCONTAllocIdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTCONTPolicyValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciGetTcontInstCounts(void);

#define TCONT_POLICY_NULL	0
#define TCONT_POLICY_SP 	1
#define TCONT_POLICY_WRR	2

#define DIRECTION_UNI_TO_ANI		1
#define DIRECTION_ANI_TO_UNI		2
#define DIRECTION_BIDIRECTIONAL	3

#ifdef TCSUPPORT_GPON_MAPPING
#define RECONFIG_GPONMAP_RULE		(0xFFFE)
#define EMPTY_GPONMAP_RULE 		(0xFFFF)
#define UP_GEMPORT_TO_METER_START	10
#define UP_GEMPORT_TO_METER_NUM		8
#define DOWN_GEMPORT_TO_METER_START	(UP_GEMPORT_TO_METER_START+UP_GEMPORT_TO_METER_NUM)

int omciAddQueueMappingRule(uint16 meClassId , omciMeInst_t *meInstant_ptr);
int omciDelQueueMappingRule(uint16 meClassId , omciMeInst_t *meInstant_ptr);
int omciReconfigQueueMappingRule(void);
int omciEmptyQueueMappingRule(void);
#endif
int32 getUNIcounterValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 setPQPointerDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTDPointerUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEncryptionStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTDPointerDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
extern alarm_id_map_entry_t meMapTableAniG[];
extern omciAttriDescript_t omciAttriDescriptListAniG[];
extern omciAttriDescript_t omciAttriDescriptListTcont[];
extern omciAttriDescript_t omciAttriDescriptListGemPortCtp[];
extern omciAttriDescript_t omciAttriDescriptListGemPortPm[];
extern omciAttriDescript_t omciAttriDescriptListReserved351PM[];

#ifdef TCSUPPORT_VNPTT
extern omciAttriDescript_t omciAttriDescriptListReserved250[];
extern omciAttriDescript_t omciAttriDescriptListReserved347[];
#endif

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
extern omciAttriDescript_t omciAttriDescriptListReserved350[];
extern omciAttriDescript_t omciAttriDescriptListReserved352[];
extern omciAttriDescript_t omciAttriDescriptListReserved353[];
extern omciAttriDescript_t omciAttriDescriptListReserved367[];
extern omciAttriDescript_t omciAttriDescriptListReserved373[];
#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
extern omciAttriDescript_t omciAttriDescriptListReserved370[];
extern omciAttriDescript_t omciAttriDescriptListReserved65408[];
extern omciAttriDescript_t omciAttriDescriptListReserved65414[];
extern omciAttriDescript_t omciAttriDescriptListReserved65425[];
#endif
#endif

int omciMeInitForGemPortPM(omciManageEntity_t *omciManageEntity_p);
int32 setGemPortPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getGemPortPmLostPacketsValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGemPortPmMisinsertedPacketsValue(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getGemPortPmReceivedPacketsValue(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getGemPortPmReceivedBlocksValue(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getGemPortPmTransmittedBlocksValue(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getGemPortPmImpairedBlocksValue(char * value,uint8 * meInstantPtr,omciAttriDescript_ptr omciAttribute,uint8 flag);

#ifdef TCSUPPORT_OMCI_ALCATEL
extern omciAttriDescript_t omciAttriDescriptListTotalGemPortPm[];
int omciMeInitForTotalGemPortPM(omciManageEntity_t *omciManageEntity_p);
int32 setTotalGemPortPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getTotalGemPortPmLostCntDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmLostCntUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmReceivedCntValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmReceivedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmTransmittedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmImpairedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmTransmittedCntValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmBadReceivedCntValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmReceivedBlocks64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTotalGemPortPmTransmittedBlocks64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

extern omciAttriDescript_t omciAttriDescriptListEthernetTrafficPm[];
int omciMeInitForEthernetTrafficPM(omciManageEntity_t *omciManageEntity_p);
int32 setEthernetTrafficPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getEthernetTrafficPmFramesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmFramesDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmBytesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmBytesDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmDropFramesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmDropBytesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmDropFramesDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmMulticastFramesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmMulticastFramesDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmBytesUp64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmBytesDown64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmFramesUp64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetTrafficPmFramesDown64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

extern omciAttriDescript_t omciAttriDescriptListVlanTagDownstreamSupplemental1[];
int omciMeInitForVlanTagDownstreamSupplemental1(omciManageEntity_t *omciManageEntity_p);
int32 setVlanTxDefaultTagValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 setVlanTxBehaviorTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag);

#if defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)
extern omciAttriDescript_t omciAttriDescriptListVlanTagUpstreamPolicer[];
int omciMeInitForVlanTagUpstreamPolicer(omciManageEntity_t *omciManageEntity_p);
int32 setVlanRxPolicerTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag);
#endif

extern omciAttriDescript_t omciAttriDescriptListGEMPortProtocolMonitoringHistoryDataPart2[];
int omciMeInitForGEMPortProtocolMonitoringHistoryDataPart2(omciManageEntity_t *omciManageEntity_p);
int32 setGEMPortProtocolMonitoringHistoryDataPart2ThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGEMPortProtocolMonitoringHistoryDataPart2LostGEMFragmentCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGEMPortProtocolMonitoringHistoryDataPart2TransmittedGEMFragmentCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#endif

#define MAX_TCONT_NUM (CONFIG_GPON_MAX_TCONT - 1)

#define NUM_OF_QUEUE_PER_TCONT 8
#define MAX_UNI_PORT_NUM 4
#define NUM_OF_QUEUE_PER_UIN_PORT 8

#define TXDIRECTION 0
#define RXDIRECTION 1
#define LAN_INTERFACE 0
#define WAN_INTERFADE 1

#define MIN_TONT_MEID	0x8000

#define MAX_PORT_NUM	256

#define SP_POLICY 1
#define WRR_POLICY 2
typedef struct trafficDesPortList_s{
uint16 portNum;
uint16 portId[MAX_PORT_NUM];
uint8  ifaceType[MAX_PORT_NUM]; //0:LAN interface 1:WAN interface
uint8   direction[MAX_PORT_NUM];//0:TX, 1:RX,  2:TX and RX
}trafficDesPortList_t,*trafficDesPortList_Ptr;


int omciMeInitForPriorityQueue(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForTrafficScheduler(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForTrafficDescriptor(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForGemportNetworkCtpPM(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForEnergyConsumPM(omciManageEntity_t *omciManageEntity_p);



int32 getMaxQueueSizeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getAllocQueueSizeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setAllocQueueSizeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRelatePortValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setRelatePortValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficSchedPointValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getWeightValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setWeightValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#if 1
int32 getBackPressureOperatetValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setBackPressureOperatetValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getBackPressureTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setBackPressureTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getBackPressureOccurThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setBackPressureOccurThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getBackPressureClearThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setBackPressureClearThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#endif
#if 1
int32 getPacketDropThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPacketDropThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPacketDropMaxpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPacketDropMaxpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getQueueDropWqValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setQueueDropWqValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getDropPrecedenceColorMarkValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDropPrecedenceColorMarkValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#endif

int32 setTcontPointValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTcontPolicyValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTcontPriorityWeightValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#if 0 //remove now, may be used in future

int32 getTrafficDescriptorCIRValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficDescriptorCIRValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTrafficDescriptorPIRValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficDescriptorPIRValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTrafficDescriptorCBSValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficDescriptorCBSValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTrafficDescriptorPBSValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficDescriptorPBSValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTrafficDescriptorColourModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficDescriptorColourModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTrafficDescriptoraInMarkingValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficDescriptoraInMarkingValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTrafficDescriptoraEnMarkingValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficDescriptoraEnMarkingValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getTrafficDescriptorMeterTypeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTrafficDescriptorMeterTypeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#endif

int setBoundTD(IN uint8 portId, IN uint8 ifcType, IN uint16 tdInstId, IN uint8 dir);
int cleanBoundTD(IN uint8 portId, IN uint8 ifcType, IN uint16 tdInstId, IN uint8 dir);
	
int32 setGemPortCtpPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag);
int32 getGemPortCtpPmTransmitFramesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGemPortCtpPmReceiveFramesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGemPortCtpPmReceiveBytesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGemPortCtpPmTransmitBytesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 setEnergyComsumPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag);	
int32 getEnergyComsumPmTransmitFramesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEnergyComsumPmCyclicSleepTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEnergyComsumPmEnergyConsumedValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int omciInitInstForPriorityQueue(void);
int omciInitInstForTrafficScheduler(void);


extern omciAttriDescript_t omciAttriDescriptListPriorityQueue[];
extern omciAttriDescript_t omciAttriDescriptListTrafficScheduler[];
extern omciAttriDescript_t omciAttriDescriptListTrafficDescriptor[];
extern omciAttriDescript_t omciAttriDescriptListGemportNetworkCtpPm[];
extern omciAttriDescript_t omciAttriDescriptListEnergyConsumptionPm[];



typedef struct backPressure_s{
	uint8 Enable;
	uint32 time;
	uint16 MaxQueueThreshold;
	uint16 MinQueueThreshold;
}backPressure_t, *backPressure_ptr;

typedef struct dropPolicy_s{
uint16 greenPacketDropQueueMaxThr;
uint16 greenPacketDropQueueMinThr;
uint16 yellowPacketDropQueueMaxThr;
uint16 yellowPacketDropQueueMinThr;
uint8   greenPacketDropMax_p;
uint8   yellowPacketDropMax_p;
uint8   QueueDropW_q;
uint8   DropPrecdenceColourMarking;
}dropPolicy_t, *dropPolicy_ptr;

typedef struct trafficDescriptor_s{
uint32 CIR;
uint32 PIR;
uint32 CBS;
uint32 PBS;
uint8   colourMode;
uint8   ingressColourMarking;
uint8   engressColourMarking;
uint8   meterType;
uint8 	direction;
}trafficDescriptor_t, *trafficDescriptor_ptr;

#ifdef TCSUPPORT_OMCI_ALCATEL
typedef struct vlanPolicerTxTable_s
{
	uint8 EntryNumber;
	uint8 VlanField;
	uint16 vlanID;
	uint16 Reserved;
}vlanPolicerTxTable_t,*vlanPolicerTxTable_ptr;

#define VLAN_POLICER_RX_TBL_MAX_ENTRY 8
typedef struct GNU_PACKED vlanPolicerRxTable_s
{
	uint8 entryID;
	enum { VLAN_POLICY_RM, VLAN_POLICY_ADD}actionType:1;
	uint16 dummy:3;
	uint16 vlanID:12;
	uint32 CIR;
	uint32 CBS;
	uint8 Reserved[8];
}vlanPolicerRxTable_t,*vlanPolicerRxTable_ptr;

#define VLAN_POLICER_RX_TBL_MAX_SIZE \
        (sizeof(vlanPolicerRxTable_t) * VLAN_POLICER_RX_TBL_MAX_ENTRY)
#endif

/* pon mac api */
int setQOSPolicy(uint8 channel,uint8 qosType,uint8 cfg_commit);
int getPonMacQOSParam(uint8 channel,uint8 *qosChannel, uint8 *qosType ,uint8 *ponWeight);
int setPonMacQOSParam(uint8 qosChannel, uint8 qosType, uint8 *ponWeight, uint8 cfg_commit);
int setPonMacTrafficDescriptor(uint8 portId, trafficDescriptor_t *trafficDescriptor);


#endif
