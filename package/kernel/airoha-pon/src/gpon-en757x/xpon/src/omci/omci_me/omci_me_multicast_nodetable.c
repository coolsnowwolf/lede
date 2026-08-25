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
	omci_me_multicast_nodetable.c

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
#include "omci_me_multicast.h"

/*******************************************************************************************************************************
start
9.3.27 multicast operations profile

********************************************************************************************************************************/
static AttributeFunc MOPMulticastVersionGetSetFunc = {
	getGeneralValue,
	setMOPMulticastVersionValue,
};

static AttributeFunc MOPMulticastFuncGetSetFunc = {
	getGeneralValue,
	setMOPMulticastFuncValue,
};

static AttributeFunc MOPMulticastFastLeaveGetSetFunc = {
	getGeneralValue,
	setMOPMulticastFastLeaveValue,
};

static AttributeFunc MOPMulticastUpstreamTCIGetSetFunc = {
	getGeneralValue,
	setMOPMulticastUpstreamTCIValue,
};

static AttributeFunc MOPMulticastTagCtrlGetSetFunc = {
	getGeneralValue,
	setMOPMulticastTagCtrlValue,
};

static AttributeFunc MOPMulticastUpstreamRateGetSetFunc = {
	getGeneralValue,
	setMOPMulticastUpstreamRateValue,
};

static AttributeFunc MOPMulticastDynamicAclTableGetSetFunc = {
	getMOPMulticastDynamicAclTableValue,
	setMOPMulticastDynamicAclTableValue,
};

static AttributeFunc MOPMulticastStaticAclTableGetSetFunc = {
	getMOPMulticastStaticAclTableValue,
	setMOPMulticastStaticAclTableValue,
}; 

static AttributeFunc MOPMulticastRobustnessGetSetFunc = {
	getGeneralValue,
	setMOPMulticastRobustnessValue,
}; 

static AttributeFunc MOPMulticastQuerierIPAddressGetSetFunc = {
	getGeneralValue,
	setMOPMulticastQuerierIPAddressValue,
}; 

static AttributeFunc MOPMulticastQueryIntervalGetSetFunc = {
	getGeneralValue,
	setMOPMulticastQueryIntervalValue,
}; 

static AttributeFunc MOPMulticastQueryMaxResponseTimeGetSetFunc = {
	getGeneralValue,
	setMOPMulticastQueryMaxResponseTimeValue,
}; 

static AttributeFunc MOPMulticastLastMemberQueryIntervalGetSetFunc = {
	getGeneralValue,
	setMOPMulticastLastMemberQueryIntervalValue,
}; 

static AttributeFunc MOPMulticastUnauthorizedJoinRequestGetSetFunc = {
	getGeneralValue,
	setMOPMulticastUnauthorizedJoinRequestValue,
}; 

static AttributeFunc MOPMulticastDownstreamIGMPAndTCIGetSetFunc = {
	getGeneralValue,
	setMOPMulticastDownstreamIGMPAndTCIValue,
};

omciAttriDescript_t omciAttriDescriptListMulticastProfile[]={
	{0,	"Managed entity id" , 			2, ATTR_ACCESS_R_SET_CREATE ,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},	
	{1,	"IGMP version" , 				1, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &MOPMulticastVersionGetSetFunc},
	{2,	"IGMP function" , 				1, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &MOPMulticastFuncGetSetFunc},	
	{3, 	"immediate leave" , 			1, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastFastLeaveGetSetFunc},	
	{4,	"Upstream IGMP TCI" , 			2, ATTR_ACCESS_R_W_SET_CREATE ,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastUpstreamTCIGetSetFunc},	
	{5,	"Upsteam IGMP tag control" , 	1, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &MOPMulticastTagCtrlGetSetFunc},	
	{6,	"Upstream IGMP rate" , 			4, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastUpstreamRateGetSetFunc},	
	{7,	"Dynamic access control table" ,	24, ATTR_ACCESS_R_W, 				ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &MOPMulticastDynamicAclTableGetSetFunc},	
	{8,	"static access control table" , 	24, ATTR_ACCESS_R_W,				ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &MOPMulticastStaticAclTableGetSetFunc},	
	{9,	"Lost groups list table" , 		0, ATTR_ACCESS_R,					ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},	
	{10,	"Robustness" , 				1, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastRobustnessGetSetFunc},	
	{11,	"Querier IP address" , 			4, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastQuerierIPAddressGetSetFunc},	
	{12,	"Query interval" , 				4, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastQueryIntervalGetSetFunc},	
	{13,	"Query max response time" , 	4, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastQueryMaxResponseTimeGetSetFunc},	
	{14,	"Last member query interval" , 	4, ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastLastMemberQueryIntervalGetSetFunc},	
	{15,	"Unauthorized join request" , 	1, ATTR_ACCESS_R_W , 				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastUnauthorizedJoinRequestGetSetFunc},	
	{16,	"Downstream IGMP and TCI" , 	3, ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &MOPMulticastDownstreamIGMPAndTCIGetSetFunc},	
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

AttributeFunc mscMeTypeFunc = {
	getGeneralValue,
	setMSCmeTypeValue
};
AttributeFunc mscOperationProfileFunc = {
	getGeneralValue,
	setMSCOperationProfileValue
};
AttributeFunc mscMaxSimultaneousGroupFunc = {
	getGeneralValue,
	setMSCMaxSimultaneousGroupValue
};
AttributeFunc mscMaxBandwidthFunc = {
	getGeneralValue,
	setMSCMaxBandwidthValue
};
AttributeFunc mscBWEnforcementFunc = {
	getGeneralValue,
	setMSCBWEnforcementValue
};

/*******************************************************************************************************************************
start
9.3.28 multicast subscriber config info

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListMulticasSubscribertCfgInfo[]={
	{0,	"Managed entity id", 		2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},	
	{1,	"ME type", 				1, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mscMeTypeFunc},
	{2,	"operations profile pointer",	2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mscOperationProfileFunc},
	{3,	"Max simultaneous groups", 	2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT, NO_AVC, &mscMaxSimultaneousGroupFunc},
	{4,	"Max multicast bandwidth", 	4, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT, NO_AVC, &mscMaxBandwidthFunc},
	{5,	"Bandwidth enforcement", 	1, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &mscBWEnforcementFunc},	
	{6,	"service package table", 	0, 	ATTR_ACCESS_R_W,				ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},	
	{7,	"preview groups table", 		0, 	ATTR_ACCESS_R_W,				ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},	
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
start
9.3.29 multicast subscriber monitor

********************************************************************************************************************************/
AttributeFunc mscIpv4ActiveGroupTblFunc = {
	getMSMIpv4ActiveGroupTblValue,
	NULL
};

omciAttriDescript_t omciAttriDescriptListMulticastSubscriberMonitor[]={
	{0,	"Managed entity id", 			2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},	
	{1,	"ME type", 					1, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,	NO_AVC, &generalGetSetFunc},
	{2,	"Current multicast bandwidth",	4, 	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},	
	{3,	"Join messages counter", 		4, 	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},	
	{4,	"Bandwidth exceeded counter", 	4, 	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},	
	{5,	"IPv4 active group table", 		0, 	ATTR_ACCESS_R,					ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &mscIpv4ActiveGroupTblFunc},
	{6,	"IPv6 active group table", 		0, 	ATTR_ACCESS_R,					ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},	
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};
