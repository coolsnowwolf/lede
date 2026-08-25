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
	omci_me_ethernet_services.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/

#ifndef _ETHERNET_SERVICES_ME_H_
#define _ETHERNET_SERVICES_ME_H_
#include "omci_types.h"
#include "omci_me.h"

#define OMCI_ETH_UNI_DTE_DCE_AUTO	2
//9.5.1: Physical path termination point Ethernet UNI
extern alarm_id_map_entry_t alarmIdTablePPTPEthernetUNI[];
extern omciAttriDescript_t omciAttriDescriptListPPTPEthernetUNI[];
int32 setPPTPEthernetUNIExpectedType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIExpectedType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPPTPEthernetUNIAutoDetectConfig(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIAutoDetectConfig(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPPTPEthernetUNIEthernetLoopbackCfg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIEthernetLoopbackCfg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPPTPEthernetUNIAdminStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIAdminStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPPTPEthernetUNICfgIndValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPPTPEthernetUNIMaxFrameSize(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIMaxFrameSize(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPPTPEthernetUNIDTEOrDCEInd(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIDTEOrDCEInd(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getPPTPEthernetUNIPauseTime(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIPauseTime(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIARCValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIARCIntervalValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setPPTPEthernetUNIPPPoEFilter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int omciMeInitForPPTPEthernetUNI(omciManageEntity_t *omciManageEntity_p);


//9.5.2: Ethernet PM history data
extern omciAttriDescript_t omciAttriDescriptListEthernetPMHistoryData[];
int32 setEthernetPMHistoryDataThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataFCSErrors(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataExcessiveCollisionCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataLateCollisionCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataFramesTooLong(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataBufOverflowOnRecv(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataBufOverflowOnTransmit(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataSingleCollisionFrameCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataMultiCollisionFrameCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataSqeCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataDeferredTransCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataInternalMACTransErrorCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataCarrierSenseErrCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataAlignErrCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryDataInternalMACRecvErrCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForEthernetPMHistoryData(omciManageEntity_t *omciManageEntity_p);


//9.5.3: Ethernet PM history data 2
extern omciAttriDescript_t omciAttriDescriptListEthernetPMHistoryData2[];
int32 setEthernetPMHistoryData2ThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData2PPPoEFilteredFrameCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForEthernetPMHistoryData2(omciManageEntity_t *omciManageEntity_p);


//9.5.4: Ethernet PM history data 3
extern omciAttriDescript_t omciAttriDescriptListEthernetPMHistoryData3[];
int32 setEthernetPMHistoryData3ThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3DropEvents(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Packets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3BroadcastPackets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3MulticastPackets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3UndersizePackets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Fragments(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Jabbers(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Packets64Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Packets65To127Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Packets128To255Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Packets256To511Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Packets512To1023Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthernetPMHistoryData3Packets1024To1518Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForEthernetPMHistoryData3(omciManageEntity_t *omciManageEntity_p);


//9.5.5:Virtual Ethernet interface point
extern omciAttriDescript_t omciAttriDescriptListVirtualEthernetInterfacePoint[];
extern alarm_id_map_entry_t alarmIdTableVirtualEthernetInterfacePoint[];
int32 setVirtualEthernetInterfacePointAdminStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getVirtualEthernetInterfacePointOpStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getVirtualEthernetInterfacePointIANAAssignedPortValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForVirtualEthernetInterfacePoint(omciManageEntity_t *omciManageEntity_p);

// clock data set PTP
#ifdef TCSUPPORT_OMCI_ALCATEL
extern omciAttriDescript_t omciAttriDescriptListClockDataSet[];
extern alarm_id_map_entry_t alarmIdTableClockDataSet[];
int omciMEInitForClockDataSet(omciManageEntity_t *omciManageEntity_p);
int32 getclockDataSetDomainValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setclockDataSetDomainValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getclockDataSetClockClassValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setclockDataSetClockClassValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getclockDataSetClockAccuracyValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setclockDataSetClockAccuracyValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getclockDataSetClockPrior1Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setclockDataSetClockPrior1Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getclockDataSetClockPrior2Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setclockDataSetClockPrior2Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

extern omciAttriDescript_t omciAttriDescriptListPTPMasterConfigDataProfile[];
int omciMEInitForPTPMasterConfigDataProfile(omciManageEntity_t *omciManageEntity_p);

extern omciAttriDescript_t omciAttriDescriptListPTPPort[];
int omciMEInitForPTPPort(omciManageEntity_t *omciManageEntity_p);

int omciMeInitForUNISupplemental1V2(omciManageEntity_t *omciManageEntity_p);
int omciInternalCreatActionForUNISupplemental1V2(uint16 classId, uint16 instanceId);
extern omciAttriDescript_t omciAttriDescriptListUNISupplemental1V2[];
int32 getDownStreamTagHandlingModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setDownStreamTagHandlingModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getIgmpAccessControlMethodValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setIgmpAccessControlMethodValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMulticastVIDValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setMulticastVIDValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMulticastPBitValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setMulticastPBitValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getIgmpChannelBridgePortNumberValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setIgmpChannelBridgePortNumberValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getUpstreamDefaultPriorityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setUpstreamDefaultPriorityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getMaximumNumberOfHostsPerUNIValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setMaximumNumberOfHostsPerUNIValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
#endif

#endif

