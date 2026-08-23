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
	omci_me_layer2_data_services_nodetable.c

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
#include "omci_me_layer2_data_services.h"


/*******************************************************************************************************************************
								9.3.18 Dot1 rate limiter

********************************************************************************************************************************/
static uint16 tpType[] = {1, 2, CODE_POINT_END};

static AttributeFunc Dot1RateLimiterTPTypeFunc = {
	getGeneralValue,
	setDot1RateLimiterTPType
};

omciAttriDescript_t omciAttriDescriptListDot1RateLimiter[]={
{0, "meId",						2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "parent me pointer",				2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{2, "tp type",						1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,		0,		0,	tpType,	ATTR_FULLY_SUPPORT,	NO_AVC,	&Dot1RateLimiterTPTypeFunc},
{3, "us unicast flood rate ptr",		2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{4, "us broadcast rate ptr",			2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{5, "us multicast payload rate ptr",	2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};
#ifdef TCSUPPORT_OMCI_DOT1AG
/*******************************************************************************************************************************
						9.3.19 Dot1ag maintenance domain

********************************************************************************************************************************/
static uint16 MDNameFormat[] = {1, 2, 3, 4, 32, CODE_POINT_END};
static uint16 MDMHFCreation[] = {1, 2, 3, CODE_POINT_END};
static uint16 MDSenderIDPermission[] = {1, 2, 3, 4, CODE_POINT_END};

static AttributeFunc dot1agMaintenanceDomainMDLevelFunc = {
	getGeneralValue,
	setDot1agMaintenanceDomainMDLevel
};

static AttributeFunc dot1agMaintenanceDomainMDNameFormatFunc = {
	getGeneralValue,
	setDot1agMaintenanceDomainMDNameFormat
};

static AttributeFunc dot1agMaintenanceDomainMDName1Func = {
	getGeneralValue,
	setDot1agMaintenanceDomainMDName1
};

static AttributeFunc dot1agMaintenanceDomainMDName2Func = {
	getGeneralValue,
	setDot1agMaintenanceDomainMDName2
};

static AttributeFunc dot1agMaintenanceDomainMHFCreationFunc = {
	getGeneralValue,
	setDot1agMaintenanceDomainMHFCreation
};

static AttributeFunc dot1agMaintenanceDomainSenderIDPermissionFunc = {
	getGeneralValue,
	setDot1agMaintenanceDomainSenderIDPermission
};

omciAttriDescript_t omciAttriDescriptListDot1agMaintenanceDomain[]={
{0, "meId",					2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0x1,		0xFFFE,	0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "MD level",				1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0x0,		0x7,		0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceDomainMDLevelFunc},
{2, "MD name format",			1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,		0,		0,	MDNameFormat,			ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceDomainMDNameFormatFunc},
{3, "MD name 1",				25,	ATTR_ACCESS_R_W,				ATTR_FORMAT_STRING,		0,		0,		0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceDomainMDName1Func},
{4, "MD name 2",				25,	ATTR_ACCESS_R_W,				ATTR_FORMAT_STRING,		0,		0,		0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceDomainMDName2Func},
{5, "MHF creation",			1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,		0,		0,	MDMHFCreation,			ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceDomainMHFCreationFunc},
{6, "sender id permission",		1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,		0,		0,	MDSenderIDPermission,	ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceDomainSenderIDPermissionFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
						9.3.20 Dot1ag maintenance association

********************************************************************************************************************************/
static uint16 shortMANameFormat[] = {1, 2, 3, 4, CODE_POINT_END};
static uint16 CCMInterval[] = {0, 1, 2, 3, 4, 5, 6, 7, CODE_POINT_END};
static uint16 MAMHFCreation[] = {1, 2, 3, 4, CODE_POINT_END};
static uint16 MASenderIDPermission[] = {1, 2, 3, 4, 5, CODE_POINT_END};

static AttributeFunc dot1agMaintenanceDomainMANameFormatFunc = {
	getGeneralValue,
	setDot1agMaintenanceDomainMANameFormat
};

static AttributeFunc dot1agMaintenanceAssociationShortMAName1Func = {
	getGeneralValue,
	setDot1agMaintenanceAssociationShortMAName1
};

static AttributeFunc dot1agMaintenanceAssociationShortMAName2Func = {
	getGeneralValue,
	setDot1agMaintenanceAssociationShortMAName2
};

static AttributeFunc dot1agMaintenanceAssociationCCMIntervalFunc = {
	getGeneralValue,
	setDot1agMaintenanceAssociationCCMInterval
};

static AttributeFunc dot1agMaintenanceAssociationAssociatedVlansFunc = {
	getGeneralValue,
	setDot1agMaintenanceAssociationAssociatedVlans
};


static AttributeFunc dot1agMaintenanceAssociationMHFCreationFunc = {
	getGeneralValue,
	setDot1agMaintenanceAssociationMHFCreation
};

static AttributeFunc dot1agMaintenanceAssociationSenderIDPermissionFunc = {
	getGeneralValue,
	setDot1agMaintenanceAssociationSenderIDPermission
};

omciAttriDescript_t omciAttriDescriptListDot1agMaintenanceAssociation[]={
{0, "meId",					2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0x1,		0xFFFE,	0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "MD pointer",				2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,		0,		0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{2, "short MA name format",		1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,		0,		0,	shortMANameFormat,		ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceDomainMANameFormatFunc},
{3, "short MA name 1",			25,	ATTR_ACCESS_R_W,				ATTR_FORMAT_STRING,		0,		0,		0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceAssociationShortMAName1Func},
{4, "short MA name 2",			25,	ATTR_ACCESS_R_W,				ATTR_FORMAT_STRING,		0,		0,		0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceAssociationShortMAName2Func},
{5, "CCM interval",				1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,		0,		0,	CCMInterval,				ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceAssociationCCMIntervalFunc},
{6, "associated vlans",			24,	ATTR_ACCESS_R_W,				ATTR_FORMAT_STRING,		0,		0,		0,	NULL,					ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceAssociationAssociatedVlansFunc},
{7, "MHF creation",			1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,		0,		0,	MAMHFCreation,			ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceAssociationMHFCreationFunc},
{8, "sender id permission",		1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,		0,		0,	MASenderIDPermission,	ATTR_FULLY_SUPPORT,	NO_AVC,	&dot1agMaintenanceAssociationSenderIDPermissionFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};
#endif


/*******************************************************************************************************************************
					9.3.32 Ethernet frame extended PM

********************************************************************************************************************************/


static AttributeFunc EthnetFrameExtendedPMGetFunc = {
	getEthnetFrameExtendedPMFunc,
	NULL,
};
static AttributeFunc EthnetFrameExtendedPMGetcontrolblockFunc = {
	getEthnetFrameExtendedPMcontrolblockFunc,
	NULL,
};



omciAttriDescript_t omciAttriDescriptListMBEthnetFrameExtendedPM[]={
{0,   "meId",					     2,	ATTR_ACCESS_R_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0x01,	0xFFFF,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&generalGetFunc},
{1,   "interval end time",	         1,	ATTR_ACCESS_R,	            ATTR_FORMAT_UNSIGNED_INT,   0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&generalGetIntervalEndTimeFunc},
{2,   "control block",    	         16,ATTR_ACCESS_R,	            ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetcontrolblockFunc},
{3,   "drop events",		         4, ATTR_ACCESS_R,              ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{4,   "octets",    		             4, ATTR_ACCESS_R,              ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{5,   "frames",				         4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{6,   "broadcast frames",		     4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{7,   "multicast frames", 	         4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{8,   "crc errored frames", 	     4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},	
{9,   "undersize frames", 	         4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{10,  "oversize frames", 	         4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{11,  "frames 64 octets",	         4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{12,  "frames 65 to 127 octets",	 4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{13,  "frames 128 to 255 octets",	 4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{14,  "frames 256 to 511 octets",	 4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{15,  "frames 512 to 1023 octets",   4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{16,  "frames 1024 to 1518 octets",  4,	ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, 	NO_AVC, &EthnetFrameExtendedPMGetFunc},
{255, "",                            0, 0,                          0,                          0,      0,      0,  NULL,   0,                      0,      NULL}
};




/*******************************************************************************************************************************
					9.3.33 MAC bridge port ICMPv6 process preassign table

********************************************************************************************************************************/
static AttributeFunc MACBridgePortICMPv6ProcessFunc = {
	getGeneralValue,
	setMACBridgePortICMPv6ProcessMsg
};

omciAttriDescript_t omciAttriDescriptListMBPortICMPv6Process[]={
{0, "meId",					2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&generalGetFunc},
{1, "icmpv6 error msg",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{2, "icmpv6 informational msg",	1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{3, "router solicitation",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{4, "router advertisement",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{5, "neighbor solicitation",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{6, "neighbor advertisement",	1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{7, "redirect",					1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{8, "multicast listener query",	1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{9, "unknown icmpv6",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,	&MACBridgePortICMPv6ProcessFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};




/*******************************************************************************************************************************
					9.3.34 Ethernet frame extended pm 64-bit

********************************************************************************************************************************/

static AttributeFunc EthnetFrameExtendedPM64BitGetFunc = {
	getEthnetFrameExtendedPM64BitFunc,
	NULL,
};
static AttributeFunc EthnetFrameExtendedPM64BitcontrolblockGetFunc = {
	getEthnetFrameExtendedPM64BitcontrolblockFunc,
	setEthnetFrameExtendedPM64BitctlblkFunc,
};



omciAttriDescript_t omciAttriDescriptListEthernetFrameExtendedPM64bit[]={
{0, "meId", 					    2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0x01,	0xFFFF,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{1, "interval end time",		    1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetIntervalEndTimeFunc},
{2, "control block",			    16,	ATTR_ACCESS_R_W_SET_CREATE,     ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitcontrolblockGetFunc},
{3, "drop events",					8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{4, "octets",						8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{5, "frames",						8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{6, "broadcast frames",				8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{7, "multicast frames",				8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{8, "CRC errored frames",			8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{9, "undersize frames",				8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{10, "oversize frames", 			8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{11, "frames 64 octets",			8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{12, "frames 65 to 127 octets",		8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{13, "frames 128 to 255 octets",	8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{14, "frames 256 to 511 octets",	8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{15, "frames 512 to 1023 octets",	8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{16, "frames 1024 to 1518 octets",  8,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &EthnetFrameExtendedPM64BitGetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


