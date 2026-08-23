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
	omci_me_ethernet_services_nodetable.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_ethernet_services.h"

/*******************************************************************************************************************************
9.5.1 PPTP Ethernet UNI

********************************************************************************************************************************/
/*me code points define*/
static uint16 AutoDetectionCfg[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x10, 0x11, 0x12, 0x13, 0x14, 0x20, 0x30, CODE_POINT_END};
static uint16 EtherLoopbackCfg[] = {0, 3, CODE_POINT_END};
static uint16 CfgInd[] = {0x01, 0x02, 0x03, 0x11, 0x12, 0x13, CODE_POINT_END};
static uint16 DTEorDCEind[] = {0, 1, 2, CODE_POINT_END};
static uint16 bridgedOrIPind[] = {0, 1, 2, CODE_POINT_END};

alarm_id_map_entry_t alarmIdTablePPTPEthernetUNI[] = {
	{OMCI_ALARM_ID_PPTPEthernetUNI_LAN_LOS_ALARM},
	{0}
};

static AttributeFunc ethernetPPTPUNIExpectedTypeFunc = {
	getGeneralValue,
	setPPTPEthernetUNIExpectedType
};
static AttributeFunc ethernetPPTPUNISensedTypeFunc = {
	getGeneralValue,
	NULL
};
static AttributeFunc ethernetPPTPUNIAutoDetectConfigFunc = {
	getPPTPEthernetUNIAutoDetectConfig,
	setPPTPEthernetUNIAutoDetectConfig
};
static AttributeFunc ethernetPPTPUNIEthernetLoopbackCfgFunc = {
	getPPTPEthernetUNIEthernetLoopbackCfg,
	setPPTPEthernetUNIEthernetLoopbackCfg
};

static AttributeFunc ethernetPPTPUNIAdminStateFunc = {
	getPPTPEthernetUNIAdminStateValue,
	setPPTPEthernetUNIAdminStateValue
};

static AttributeFunc ethernetPPTPUNIOpStateFunc = {
	getGeneralValue,
	NULL
};

static AttributeFunc ethernetPPTPUNICfgIndFunc = {
	getPPTPEthernetUNICfgIndValue,
	NULL
};

static AttributeFunc ethernetPPTPUNIMaxFrameSizeFunc = {
	getPPTPEthernetUNIMaxFrameSize,
	setPPTPEthernetUNIMaxFrameSize
};
static AttributeFunc ethernetPPTPUNIDTEOrDCEIndFunc = {
	getGeneralValue,
	setPPTPEthernetUNIDTEOrDCEInd
};

static AttributeFunc ethernetPPTPUNIPauseTimeFunc = {
	getPPTPEthernetUNIPauseTime,
	setPPTPEthernetUNIPauseTime
};

static AttributeFunc ethernetPPTPUNIARCFunc = {
	getGeneralValue,
	setPPTPEthernetUNIARCValue
};

static AttributeFunc ethernetPPTPUNIARCIntervalFunc = {
	getGeneralValue,
	setPPTPEthernetUNIARCIntervalValue
};

static AttributeFunc ethernetPPTPUNIPPPoEFilterFunc = {
	getGeneralValue,
	setPPTPEthernetUNIPPPoEFilter
};

omciAttriDescript_t omciAttriDescriptListPPTPEthernetUNI[]={
{0, "meId",					2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0x01,	0xFF,	0,	NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,									&generalGetFunc},
{1, "expected type",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,		0xFE,	0,	NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,									&generalGetSetFunc},
{2, "sensed type",				1,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,		0xFE,	0,	NULL,				ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_PPTPEthernetUNI_SENSED_TYPE,&generalGetFunc},
{3, "auto detection config",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	AutoDetectionCfg,	ATTR_PART_SUPPORT,	NO_AVC,									&ethernetPPTPUNIAutoDetectConfigFunc},
{4, "ethernet loopback config",	1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	EtherLoopbackCfg,	ATTR_FULLY_SUPPORT,	NO_AVC,									&ethernetPPTPUNIEthernetLoopbackCfgFunc},
{5, "admin state",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,									&ethernetPPTPUNIAdminStateFunc},
{6, "operational state",			1,	ATTR_ACCESS_R,		ATTR_FORMAT_ENUM,			0,		0,		0,	NULL,				ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_PPTPEthernetUNI_OPERATIONAL_STATE,	&ethernetPPTPUNIOpStateFunc},
{7, "configuration ind",			1,	ATTR_ACCESS_R,		ATTR_FORMAT_ENUM,			0,		0,		0,	CfgInd,				ATTR_PART_SUPPORT,	NO_AVC,									&ethernetPPTPUNICfgIndFunc},
{8, "max frame size",			2,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,									&ethernetPPTPUNIMaxFrameSizeFunc},
{9, "dte or dce ind",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	DTEorDCEind,		ATTR_FULLY_SUPPORT,	NO_AVC,									&generalGetSetFunc},
{10, "pause time",				2,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,		0xFFFF,	0,	NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,									&ethernetPPTPUNIPauseTimeFunc},
{11, "Bridged or IP ind",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	bridgedOrIPind,		ATTR_FULLY_SUPPORT,	NO_AVC,									&generalGetSetFunc},
{12, "ARC",					1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,		0x1,		0,	NULL,				ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_PPTPEthernetUNI_ARC,			&ethernetPPTPUNIARCFunc},
{13, "ARC interval",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,		0xFE,	0,	NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,									&ethernetPPTPUNIARCIntervalFunc},
{14, "pppoe filter",			1 , ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,									&ethernetPPTPUNIPPPoEFilterFunc},
{15, "power control",			1 , ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	NULL,				ATTR_FULLY_SUPPORT,		NO_AVC,									&generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.5.2 Ethernet performance monitoring history data

********************************************************************************************************************************/

static AttributeFunc ethernetPMHistoryDataThresholdDataFunc = {
	getGeneralValue,
	setEthernetPMHistoryDataThresholdData
};

static AttributeFunc ethernetPMHistoryDataFCSErrorsFunc = {
	getEthernetPMHistoryDataFCSErrors,
	NULL
};

static AttributeFunc ethernetPMHistoryDataExcessiveCollisionCntFunc = {
	getEthernetPMHistoryDataExcessiveCollisionCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataLateCollisionCntFunc = {
	getEthernetPMHistoryDataLateCollisionCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataFramesTooLongFunc = {
	getEthernetPMHistoryDataFramesTooLong,
	NULL
};

static AttributeFunc ethernetPMHistoryDataBufOverflowOnRecvFunc = {
	getEthernetPMHistoryDataBufOverflowOnRecv,
	NULL
};

static AttributeFunc ethernetPMHistoryDataBufOverflowOnTransmitFunc = {
	getEthernetPMHistoryDataBufOverflowOnTransmit,
	NULL
};

static AttributeFunc ethernetPMHistoryDataSingleCollisionFrameCntFunc = {
	getEthernetPMHistoryDataSingleCollisionFrameCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataMultiCollisionFrameCntFunc = {
	getEthernetPMHistoryDataMultiCollisionFrameCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataSqeCntFunc = {
	getEthernetPMHistoryDataSqeCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataDeferredTransCntFunc = {
	getEthernetPMHistoryDataDeferredTransCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataInternalMACTransErrorCntFunc = {
	getEthernetPMHistoryDataInternalMACTransErrorCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataCarrierSenseErrCntFunc = {
	getEthernetPMHistoryDataCarrierSenseErrCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataAlignErrCntFunc = {
	getEthernetPMHistoryDataAlignErrCnt,
	NULL
};

static AttributeFunc ethernetPMHistoryDataInternalMACRecvErrCntFunc = {
	getEthernetPMHistoryDataInternalMACRecvErrCnt,
	NULL
};

omciAttriDescript_t omciAttriDescriptListEthernetPMHistoryData[]={
{0, "meId",						2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0x01,	0xFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "interval end time",				1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",			2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataThresholdDataFunc},
{3, "fcs errors",					4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataFCSErrorsFunc},
{4, "excessive collision cnt",			4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataExcessiveCollisionCntFunc},
{5, "late collision cnt",				4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataLateCollisionCntFunc},
{6, "frames too long",				4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataFramesTooLongFunc},
{7, "Buffer overflows on receive",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataBufOverflowOnRecvFunc},
{8, "Buffer overflows on transmit",	4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataBufOverflowOnTransmitFunc},
{9, "single collision frame cnt",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataSingleCollisionFrameCntFunc},
{10, "multi collisions frame cnt",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataMultiCollisionFrameCntFunc},
{11, "sqe counter",				4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataSqeCntFunc},
{12, "deferred transmission cnt",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataDeferredTransCntFunc},
{13, "internal mac trans error cnt",	4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataInternalMACTransErrorCntFunc},
{14, "carrier sense error cnt",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataCarrierSenseErrCntFunc},
{15, "alignment error cnt",			4, 	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataAlignErrCntFunc},
{16, "internal mac recv error cnt",	4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryDataInternalMACRecvErrCntFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.5.3 Ethernet performance monitoring history data 2

********************************************************************************************************************************/

static AttributeFunc ethernetPMHistoryData2ThresholdDataFunc = {
	getGeneralValue,
	setEthernetPMHistoryData2ThresholdData
};

static AttributeFunc ethernetPMHistoryData2PPPoEFilteredFrameCntFunc = {
	getEthernetPMHistoryData2PPPoEFilteredFrameCnt,
	NULL
};

omciAttriDescript_t omciAttriDescriptListEthernetPMHistoryData2[]={
{0, "meId",						2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0x01,	0xFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "interval end time",				1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",			2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData2ThresholdDataFunc},
{3, "pppoe filtered frame cnt",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData2PPPoEFilteredFrameCntFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.5.4 Ethernet performance monitoring history data 3

********************************************************************************************************************************/

static AttributeFunc ethernetPMHistoryData3ThresholdDataFunc = {
	getGeneralValue,
	setEthernetPMHistoryData3ThresholdData
};

static AttributeFunc ethernetPMHistoryData3DropEventsFunc = {
	getEthernetPMHistoryData3DropEvents,
	NULL
};

static AttributeFunc ethernetPMHistoryData3OctetsFunc = {
	getEthernetPMHistoryData3Octets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3PacketsFunc = {
	getEthernetPMHistoryData3Packets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3BroadcastPacketsFunc = {
	getEthernetPMHistoryData3BroadcastPackets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3MulticastPacketsFunc = {
	getEthernetPMHistoryData3MulticastPackets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3UndersizePacketsFunc = {
	getEthernetPMHistoryData3UndersizePackets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3FragmentsFunc = {
	getEthernetPMHistoryData3Fragments,
	NULL
};

static AttributeFunc ethernetPMHistoryData3JabbersFunc = {
	getEthernetPMHistoryData3Jabbers,
	NULL
};

static AttributeFunc ethernetPMHistoryData3Packets64OctetsFunc = {
	getEthernetPMHistoryData3Packets64Octets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3Packets65To127OctetsFunc = {
	getEthernetPMHistoryData3Packets65To127Octets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3Packets128To255OctetsFunc = {
	getEthernetPMHistoryData3Packets128To255Octets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3Packets256To511OctetsFunc = {
	getEthernetPMHistoryData3Packets256To511Octets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3Packets512To1023OctetsFunc = {
	getEthernetPMHistoryData3Packets512To1023Octets,
	NULL
};

static AttributeFunc ethernetPMHistoryData3Packets1024To1518OctetsFunc = {
	getEthernetPMHistoryData3Packets1024To1518Octets,
	NULL
};

omciAttriDescript_t omciAttriDescriptListEthernetPMHistoryData3[]={
{0, "meId",						2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0x01,	0xFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "interval end time",				1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",			2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3ThresholdDataFunc},
{3, "drop events",					4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3DropEventsFunc},
{4, "octets",						4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3OctetsFunc},
{5, "packets",						4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3PacketsFunc},
{6, "broadcast packets",			4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3BroadcastPacketsFunc},
{7, "multicast packets",				4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3MulticastPacketsFunc},
{8, "undersize packets",			4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3UndersizePacketsFunc},
{9, "fragments",					4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3FragmentsFunc},
{10, "jabbers",					4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3JabbersFunc},
{11, "packets 64 octets",			4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3Packets64OctetsFunc},
{12, "packets 65 to 127 octets",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3Packets65To127OctetsFunc},
{13, "packets 128 to 255 octets",	4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3Packets128To255OctetsFunc},
{14, "packets 256 to 511 octets",	4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3Packets256To511OctetsFunc},
{15, "packets 512 to 1023 octets",	4, 	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3Packets512To1023OctetsFunc},
{16, "packets 1024 to 1518 octets",	4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&ethernetPMHistoryData3Packets1024To1518OctetsFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.5.5 Virtual Ethernet interface point

********************************************************************************************************************************/

static AttributeFunc virtualEthernetInterfacePointAdminStateFunc = {
	getGeneralValue,
	setVirtualEthernetInterfacePointAdminStateValue
};

static AttributeFunc virtualEthernetInterfacePointOpStateFunc = {
	getVirtualEthernetInterfacePointOpStateValue,
	NULL
};

static AttributeFunc virtualEthernetInterfacePointIANAAssignedPortFunc = {
	getVirtualEthernetInterfacePointIANAAssignedPortValue,
	NULL
};

alarm_id_map_entry_t alarmIdTableVirtualEthernetInterfacePoint[] = {
	{OMCI_ALARM_ID_VEIP_CONNECTING_FUNCTION_FAIL},
	{0}
};

omciAttriDescript_t omciAttriDescriptListVirtualEthernetInterfacePoint[]={
{0, "meId",				2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0x1,	0xFFFE,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,									&generalGetFunc},
{1, "admin state",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,	0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,									&virtualEthernetInterfacePointAdminStateFunc},
{2, "operational state",		1,	ATTR_ACCESS_R,		ATTR_FORMAT_ENUM,			0,	0,		0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_VEIP_OPERATIONAL_STATE,		&virtualEthernetInterfacePointOpStateFunc},
{3, "interdomain name",		25,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,		0,	0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,									&generalGetSetFunc},
{4, "tcp/udp pointer",		2,	ATTR_ACCESS_R_W,	ATTR_FORMAT_POINTER,		0,	0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,									&generalGetSetFunc},
{5, "iana assigned port",	2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,									&virtualEthernetInterfacePointIANAAssignedPortFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

#ifdef TCSUPPORT_OMCI_ALCATEL
/********************************
Clock Data Set
*********************************/
alarm_id_map_entry_t alarmIdTableClockDataSet[] = {
    {OMCI_ALARM_ID_CLOCK_DATA_SET_OUN_LOST_ToD_SYNC_ALARM},
    {0}
};

static AttributeFunc clockDataSetDomainFunc = {
    getclockDataSetDomainValue,
    setclockDataSetDomainValue    
};

static AttributeFunc clockDataSetClockClassFunc = {
    getclockDataSetClockClassValue,
    setclockDataSetClockClassValue    
};    

static AttributeFunc clockDataSetClockAccuracyFunc = {
    getclockDataSetClockAccuracyValue,
    setclockDataSetClockAccuracyValue    
};     

static AttributeFunc clockDataSetClockPrior1Func = {
    getclockDataSetClockPrior1Value,
    setclockDataSetClockPrior1Value    
};  

static AttributeFunc clockDataSetClockPrior2Func = {
    getclockDataSetClockPrior2Value,
    setclockDataSetClockPrior2Value    
};  

omciAttriDescript_t omciAttriDescriptListClockDataSet[]={
{0, "meId",             2,  ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,       0x01,   0xFFFE, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},   
{1, "Domain",           1,  ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_ENUM,               0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{2, "Clock class",      1,  ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,       0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{3, "Clock accuracy",   1,  ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_ENUM,               0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "Clock priority1",  1,  ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,       0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "Clock priority2",  1,  ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,       0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

omciAttriDescript_t omciAttriDescriptListPTPMasterConfigDataProfile[]={
{0, "meId",                     2,ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_UNSIGNED_INT,  0x01,   0xFFFE, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{1, "Clock Data Set Pointer",   2,ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_POINTER,       0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{2, "Step mode",                1,ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_UNSIGNED_INT,  0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{3, "Delay mechanism",          1,ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_UNSIGNED_INT,  0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "Interval of sync message", 1,ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_SIGNED_INT,    0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "Interval of delay message",1,ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_SIGNED_INT,    0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{6, "Interval of pdelay message",1,ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_SIGNED_INT,   0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{7, "Interval of announce message",1,ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_SIGNED_INT, 0,      0xFF,   0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

omciAttriDescript_t omciAttriDescriptListPTPPort[]={
{0, "meId",                                2, ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,  0x01,   0xFFFE, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{1, "PTP Master Configure Data pointer",   2, ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_POINTER,       0,      0xFFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{2, "PPTP UNI pointer",                    2, ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_POINTER,       0,      0xFFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{3, "Admin state",                         1, ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_ENUM,          0,      1,      0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "TCI",                                 2, ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,  0,      0xFFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

static AttributeFunc downStreamTagHandlingModeFunc = {
	getGeneralValue,
	setDownStreamTagHandlingModeValue
};

static AttributeFunc igmpAccessControlMethodFunc = {
	getIgmpAccessControlMethodValue,
	setIgmpAccessControlMethodValue
};

static AttributeFunc multicastVIDFunc = {
	getMulticastVIDValue,
	setMulticastVIDValue
};

static AttributeFunc multicastPBitFunc = {
	getMulticastPBitValue,
	setMulticastPBitValue
};

static AttributeFunc igmpChannelBridgePortNumberFunc = {
	getIgmpChannelBridgePortNumberValue,
	setIgmpChannelBridgePortNumberValue
};

static AttributeFunc upstreamDefaultPriorityFunc = {
	getUpstreamDefaultPriorityValue,
	setUpstreamDefaultPriorityValue
};

static AttributeFunc maximumNumberOfHostsPerUNIFunc = {
	getMaximumNumberOfHostsPerUNIValue,
	setMaximumNumberOfHostsPerUNIValue
};

omciAttriDescript_t omciAttriDescriptListUNISupplemental1V2[]={
{0, "meId",							2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "Downstream Tag Handling Mode",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&downStreamTagHandlingModeFunc},
{2, "IGMP Access Control Method",		1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{3, "Multicast VID Value",				2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{4, "Multicast P-Bit Value",				1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{5, "IGMPChannelBridgePortNumber",		1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{6, "Upstream Default Priority",			1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{7, "Maximum Number of Hosts per UNI", 	1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

#endif
