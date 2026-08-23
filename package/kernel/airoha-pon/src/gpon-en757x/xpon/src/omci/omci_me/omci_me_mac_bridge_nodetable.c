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
	omci_me_mac_bridge_nodetable.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2012/7/28	Create
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_mac_bridge.h"

/*******************************************************************************************************************************
start
9.3.1 MAC Bridge Service Profile ME

********************************************************************************************************************************/
AttributeFunc mbspSpanningTreeIndFunc = {
	getGeneralValue,
	setGeneralValue
};

AttributeFunc mbspLlearningIndFunc = {
	getGeneralValue,
	setMBSPLearningIndValue
};

AttributeFunc mbspPortBridingIndFunc = {
	getMBSPPortBridingIndValue,
	setMBSPPortBridingIndValue
};

AttributeFunc mbspPriorityFunc = {
	getGeneralValue,
	setGeneralValue
};

 AttributeFunc mbspMaxAgeFunc = {
	getGeneralValue,
	setGeneralValue
};

 AttributeFunc mbspHelloTimeFunc = {
	getGeneralValue,
	setGeneralValue
};

 AttributeFunc mbspForwardDelayFunc = {
	getGeneralValue,
	setGeneralValue
};

 AttributeFunc mbspUnKnownMacAddrDiscardFunc = {
	getMBSPUnKnownMacAddrDiscardValue,
	setMBSPUnKnownMacAddrDiscardValue
};

AttributeFunc mbspMacAddrLimitFunc = {
	getGeneralValue,
	setMBSPMacAddrLimitValue 
};

omciAttriDescript_t omciAttriDescriptListMBServiceProfile[]={
	{0,	"Managed entity id" , 			2, ATTR_ACCESS_R_SET_CREATE ,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},	
	{1,	"Spanning tree ind" , 			1, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
	{2,	"Learning ind" , 				1, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT, 	NO_AVC, &generalGetSetFunc},	
	{3, 	"Port bridging ind" , 			1, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &mbspPortBridingIndFunc},	
	{4,	"Priority" , 					2, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
	{5,	"Max age" , 					2, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
	{6,	"Hello time" , 					2, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
	{7,	"Forward delay" , 				2, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
	{8,	"Unknown MAC addr discard" , 	1, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},	/*it is always 0*/
	{9,	"MAC learning depth" , 			1, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &mbspMacAddrLimitFunc},
	{10,	"Dynamic filter age time", 		4, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.2 MAC Bridge Configuration Data ME

********************************************************************************************************************************/
 AttributeFunc mbcdBridgeMACAddrFunc = {
	getMBCDBridgeMACAddrValue,
	NULL
};
 AttributeFunc mbcdBridgePriorityFunc = {
	getMBCDBridgePriorityValue,
	NULL
};
 AttributeFunc mbcdDesignatedRootFunc = {
	getMBCDDesignatedRootValue,
	NULL
};	
 AttributeFunc mbcdRootPathCostFunc = {
	getMBCDRootPathCostValue,
	NULL
};
 AttributeFunc mbcdBridgePortCountFunc = {
	getMBCDBridgePortCountValue,
	NULL
};

 AttributeFunc mbcdRootPortNumFunc = {
	getMBCDRootPortNumValue,
	NULL
};

omciAttriDescript_t omciAttriDescriptListMBConnData[]={
	{0,	"Managed entity id" , 			2 , ATTR_ACCESS_R ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1,	"Bridge MAC address" , 			6 , ATTR_ACCESS_R ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbcdBridgeMACAddrFunc},
	{2,	"Bridge priority" , 				2 , ATTR_ACCESS_R ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbcdBridgePriorityFunc},
	{3, 	"Designated root" , 			8 , ATTR_ACCESS_R ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbcdDesignatedRootFunc},
	{4,	"Root path cost" , 				4 , ATTR_ACCESS_R ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbcdRootPathCostFunc},
	{5,	"Bridge port count" , 			1 , ATTR_ACCESS_R , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbcdBridgePortCountFunc},
	{6,	"Root port num" , 				2 , ATTR_ACCESS_R , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbcdRootPortNumFunc}, 
	{7,	"Hello time" , 					2 , ATTR_ACCESS_R , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{8,	"Forward delay" , 				2 , ATTR_ACCESS_R , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.3 MAC bridge performance monitoring history data ME

********************************************************************************************************************************/
 AttributeFunc mbPMHDThresholdDataIdFunc = {
	getGeneralValue,
	setMBPMHDThresholdDataIdValue
};

omciAttriDescript_t omciAttriDescriptListMBPMHD[]={
	{0,	"Managed entity id" , 			2 , ATTR_ACCESS_R_SET_CREATE ,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1,	"Interval end time" , 			1 , ATTR_ACCESS_R ,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetIntervalEndTimeFunc},
	{2,	"Threshold data 1/2 id" , 		2 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbPMHDThresholdDataIdFunc},
	{3, 	"entry discard count" , 			4 , ATTR_ACCESS_R ,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT, NO_AVC, &performanceCntFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.4 MAC bridge port configuration data

********************************************************************************************************************************/
 AttributeFunc mbPortCfgDataTPTypeFunc = {
	getGeneralValue,
	setMBPCDTPTypeValue
};

 AttributeFunc mbPortCfgDataTPPointerFunc = {
	getGeneralValue,
	setMBPCDTPPointerValue
};
 
 AttributeFunc mbPortCfgDataPortMacAddressFunc = {
	getMBPCDportMacAddressValue,
	NULL
};

 AttributeFunc mbPortCfgDataOutTDPointerFunc = {
	getGeneralValue,
	setMBPCDoutTDPointerValue
};
 AttributeFunc mbPortCfgDataInTDPointerFunc = {
	getGeneralValue,
	setMBPCDinTDPointerValue
};
  AttributeFunc mbPortCfgDataMacAddrLimitFunc = {
	 getGeneralValue,
	 setMBPCDMacAddrLimitValue
 };

omciAttriDescript_t omciAttriDescriptListMBPortConData[]={
	{0,	"Managed entity id" , 			2 , ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},	
	{1,	"Bridge id pointer" , 			2 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},	
	{2,	"Port num" , 					1 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},	
	{3, 	"TP type" , 					1 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &mbPortCfgDataTPTypeFunc},
	{4, 	"TP pointer" , 					2 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &mbPortCfgDataTPPointerFunc},	
	{5, 	"Port priority" , 				2 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},	
	{6, 	"Port path cost" , 				2 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},	
	{7, 	"Port ST ind" , 				1 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},	
	{8, 	"Deprecated 1" , 				1 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},	
	{9, 	"Deprecated 2" , 				1 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},	
	{10, "Port MAC Address" , 			6 , ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &mbPortCfgDataPortMacAddressFunc},	
	{11, "Outbound TD pointer" , 		2 , ATTR_ACCESS_R_W,				ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &mbPortCfgDataOutTDPointerFunc},	
	{12, "Inbound TD pointer" , 			2 , ATTR_ACCESS_R_W,				ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &mbPortCfgDataInTDPointerFunc},	
	{13, "MAC learning depth" , 			1 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &mbPortCfgDataMacAddrLimitFunc},	
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.5 MAC bridge port designation data

********************************************************************************************************************************/

 AttributeFunc mbPortDesDataPortStateFunc = {
	getMBPDDPortStateValue,
	NULL
};

omciAttriDescript_t omciAttriDescriptListMBPortDesignData[]={
	{0,	"Managed entity id" , 			2,	ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	/* include port identifier, root identifier, designated port number, port path cost*/
	{1,	"bridge root cost port" , 			24,	ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{2,	"Port state" , 					1, 	ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbPortDesDataPortStateFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.6 mac bridge port filter table 

********************************************************************************************************************************/
 AttributeFunc mbPortFilterTableFunc = {
	getMBPFTmacFilterTableValue,
	setMBPFTmacFilterTableValue
};
omciAttriDescript_t omciAttriDescriptListMBPortFilterData[]={
	{0,	"Managed entity id",	2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1,	"Mac filter table",		8,	ATTR_ACCESS_R_W,	ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbPortFilterTableFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.7 mac bridge port filter preassign table

********************************************************************************************************************************/
AttributeFunc mbpPreFilterIpv4MulticastFilterFunc = {
	getGeneralValue,
	setMBPFPTipv4MulticastFilterValue
};
AttributeFunc mbpPreFilterIpv6MulticastFilterFunc = {
	getGeneralValue,
	setMBPFPTipv6MulticastFilterValue
};
AttributeFunc mbpPreFilterIpv4BroadcastFilterFunc = {
	getGeneralValue,
	setMBPFPTipv4BroadcastFilterValue
};
AttributeFunc mbpPreFilterRARPFilterFunc = {
	getGeneralValue,
	setMBPFPTRARPFilterValue
};
AttributeFunc mbpPreFilterIPXFilterFunc = {
	getGeneralValue,
	setMBPFPTIPXFilterValue
};
AttributeFunc mbpPreFilterNetBEULFilterFunc = {
	getGeneralValue,
	setMBPFPTNetBEULFilterValue
};

AttributeFunc mbpPreFilterappleTalkFilterFunc = {
	getGeneralValue,
	setMBPFPTappleTalkFilterValue
};
AttributeFunc mbpPreFilterManageInfoFilterFunc = {
	getGeneralValue,
	setMBPFPTmanageInfoFilterValue
};
AttributeFunc mbpPreFilterARPFilterFunc = {
	getGeneralValue,
	setMBPFPTARPFilterValue
};
AttributeFunc mbpPreFilterPPPoEBroadcastFilterFunc = {
	getGeneralValue,
	setMBPFPTPPPoEBroadcastFilterValue
};

omciAttriDescript_t omciAttriDescriptListMBPortFilterPreData[]={
	{0,	"Managed entity id",		2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1,	"ipv4 multicast filter",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterIpv4MulticastFilterFunc},
	{2,	"ipv6 multicast filter",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterIpv6MulticastFilterFunc},
	{3,	"ipv4 broadcast filter",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterIpv4BroadcastFilterFunc},
	{4,	"RARP filter",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT, NO_AVC, &mbpPreFilterRARPFilterFunc},
	{5,	"IPX filter",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterIPXFilterFunc},
	{6,	"NetBEUI filter",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterNetBEULFilterFunc},
	{7,	"AppleTalk filter",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterappleTalkFilterFunc},
	{8,	"Bridge manage info  filter",	1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterManageInfoFilterFunc},
	{9,	"ARP filter",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterARPFilterFunc},
	{10,	"PPPoE broadcast filter",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbpPreFilterPPPoEBroadcastFilterFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.8 mac bridge port bridge table data

********************************************************************************************************************************/
 AttributeFunc mbPortTableFunc = {
	getBPBTDBridgeTableValue,
	NULL
};
omciAttriDescript_t omciAttriDescriptListMBPortData[]={
	{0,	"Managed entity id",	2,	ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1,	"Bridge table",			0,	ATTR_ACCESS_R,	ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbPortTableFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.9 mac bridge port performance monitoring history data

********************************************************************************************************************************/
AttributeFunc mbPortPMHDThresholdDataFunc = {
	getGeneralValue,
	setMBPPMHDThresholdDataValue
};

omciAttriDescript_t omciAttriDescriptListMBPortPMHD[]={
	{0,	"Managed entity id" , 			2 , ATTR_ACCESS_R_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1,	"Interval end time" , 			1 , ATTR_ACCESS_R ,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetIntervalEndTimeFunc},
	{2,	"Threshold data 1/2 id" , 		2 , ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mbPortPMHDThresholdDataFunc},
	{3,	"Forwarded frame cnt" , 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{4,	"Delay exceeded discard cnt" , 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{5,	"MTU exceeded discard cnt" , 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{6,	"Received frame cnt" , 			4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{7,	"Received and discarded cnt" , 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.30 ethernet frame performance monitoring history data upstream

********************************************************************************************************************************/
AttributeFunc ethPMHDThresholdDataFunc = {
	getGeneralValue,
	setEFPMHDThresholdDataValue
};

omciAttriDescript_t omciAttriDescriptList_EthFrameUpPMHD[]={
	{0,	"Managed entity id", 		2 , ATTR_ACCESS_R_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1,	"Interval end time", 		1 , ATTR_ACCESS_R ,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetIntervalEndTimeFunc},
	{2,	"Threshold data 1/2 id", 	2 , ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &ethPMHDThresholdDataFunc},
	{3,	"Drop events", 			4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{4,	"Octets", 					4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{5,	"Packets", 				4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{6,	"Broadcast packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{7,	"Multicast packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{8,	"CRC errored packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},	
	{9,	"Undersize packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{10,	"Oversize packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{11,	"Packets 64 octets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{12,	"Packets 65~127 octets", 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{13,	"Packets 128~255 octets", 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{14,	"Packets 256~511 octets", 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{15,	"Packets 512~1023 octets",	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{16,	"Packets 1024~1518 octets",	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};



/*******************************************************************************************************************************
start
9.3.31 ethernet frame performance monitoring history data downstream

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListEthFrameDownPMHD[]={
	{0,	"Managed entity id", 		2 , ATTR_ACCESS_R_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1,	"Interval end time", 		1 , ATTR_ACCESS_R ,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetIntervalEndTimeFunc},
	{2,	"Threshold data 1/2 id", 	2 , ATTR_ACCESS_R_W_SET_CREATE,ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &ethPMHDThresholdDataFunc},
	{3,	"Drop events", 			4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{4,	"Octets", 					4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{5,	"Packets", 				4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{6,	"Broadcast packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{7,	"Multicast packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{8,	"CRC errored packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},	
	{9,	"Undersize packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{10,	"Oversize packets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{11,	"Packets 64 octets", 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{12,	"Packets 65~127 octets", 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{13,	"Packets 128~255 octets", 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{14,	"Packets 256~511 octets", 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{15,	"Packets 512~1023 octets",	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{16,	"Packets 1024~1518 octets",	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &performanceCntFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

