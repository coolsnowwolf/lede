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
	omci_me_traffic_management_notetable.c

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
#include "omci_me_traffic_management.h"


/*******************************************************************************************************************************
9.2.7: GAL Ethernet profile

********************************************************************************************************************************/

static AttributeFunc GALEthernetProfileMaxGemPayloadSizeFunc = {
	getGALEthernetProfileMaxGemPayloadSize,
	setGALEthernetProfileMaxGemPayloadSize
};

omciAttriDescript_t omciAttriDescriptListGALEthernetProfile[]={
{0, "meId",				2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,ATTR_FULLY_SUPPORT,NO_AVC,&generalGetFunc},
{1, "max gem payload size",	2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,ATTR_FULLY_SUPPORT,NO_AVC,&GALEthernetProfileMaxGemPayloadSizeFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.2.8: GAL Ethernet performance monitoring history data

********************************************************************************************************************************/

static AttributeFunc GALEthernetPMThresholdDataFunc = {
	getGeneralValue,
	setGALEthernetPMThresholdData
};

static AttributeFunc GALEthernetPMDiscardedFramesFunc = {
	getGALEthernetPMDiscardedFrames,
	NULL
};

omciAttriDescript_t omciAttriDescriptListGALEthernetPM[]={
{0, "meId",				2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "interval end time",		1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",	2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&GALEthernetPMThresholdDataFunc},
{3, "discarded frames",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&GALEthernetPMDiscardedFramesFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.2.9: FEC performance monitoring history data

********************************************************************************************************************************/

static AttributeFunc FecPMThresholdDataFunc = {
	getGeneralValue,
	setFecPMThresholdData
};

static AttributeFunc FecPMCorrectedBytesFunc = {
	getFecPMCorrectedBytes,
	NULL
};

static AttributeFunc FecPMCorrectedCodeWordsFunc = {
	getFecPMCorrectedCodeWords,
	NULL
};

static AttributeFunc FecPMUnCorrectedCodeWordsFunc = {
	getFecPMUnCorrectedCodeWords,
	NULL
};

static AttributeFunc FecPMTotalCorrectedWordsFunc = {
	getFecPMTotalCorrectedWords,
	NULL
};


static AttributeFunc FecPMFecSecondsFunc = {
	getFecPMFecSeconds,
	NULL
};

omciAttriDescript_t omciAttriDescriptListFecPM[]={
{0, "meId",					2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "interval end time",			1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",		2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&FecPMThresholdDataFunc},
{3, "corrected bytes",			4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&FecPMCorrectedBytesFunc},
{4, "corrected code words",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&FecPMCorrectedCodeWordsFunc},
{5, "uncorrected code words",	4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&FecPMUnCorrectedCodeWordsFunc},
{6, "total code words",			4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&FecPMTotalCorrectedWordsFunc},
{7, "fec seconds",				2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&FecPMFecSecondsFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.2.4: GEM interworking termination point

********************************************************************************************************************************/

static uint16 IWOption[] = {0, 1, 2, 3, 4, 5, 6, 7, CODE_POINT_END};
static uint16 GalLoopbackCfg[] = {0, 1, CODE_POINT_END};

static AttributeFunc GemIWTPGalLoopbackCfgFunc = {
	getGemIWTPGalLoopbackCfg,
	setGemIWTPGalLoopbackCfg
};

static AttributeFunc GemIWTPPPTPCounterFunc = {
	getGemIWTPPPTPCounter,
	NULL
};

static AttributeFunc GemIWTPOperStateFunc = {
	getGemIWTPOperState,
	NULL
};

omciAttriDescript_t omciAttriDescriptListGemIWTP[]={
{0, "meId",						2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,			ATTR_FULLY_SUPPORT,NO_AVC,									&generalGetFunc},
{1, "gem port network ctp conn ptr",	2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,0,0,NULL,			ATTR_FULLY_SUPPORT,NO_AVC,									&generalGetSetFunc},
{2, "iw option",					1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,0,0,IWOption,		ATTR_FULLY_SUPPORT,NO_AVC,									&generalGetSetFunc},
{3, "service profile ptr",				2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,0,0,NULL,			ATTR_FULLY_SUPPORT,NO_AVC,									&generalGetSetFunc},
{4, "iw termination point ptr",		2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,0,0,NULL,			ATTR_FULLY_SUPPORT,NO_AVC,									&generalGetSetFunc},
{5, "pptp counter",					1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,			ATTR_FULLY_SUPPORT,NO_AVC,									&GemIWTPPPTPCounterFunc},
{6, "operational state",				1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,			ATTR_FULLY_SUPPORT,OMCI_AVC_ID_GEM_IWTP_OPERATIONAL_STATE,&GemIWTPOperStateFunc},
{7, "gal profile ptr",				2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,0,0,NULL,			ATTR_FULLY_SUPPORT,NO_AVC,									&generalGetSetFunc},
{8, "gal loopback cfg",				1,	ATTR_ACCESS_R_W,				ATTR_FORMAT_ENUM,			0,0,0,GalLoopbackCfg,ATTR_FULLY_SUPPORT,NO_AVC,									&GemIWTPGalLoopbackCfgFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.2.5: Multicast GEM interworking termination point

********************************************************************************************************************************/

static uint16 MulticastIWOption[] = {0, 1, 3, 5, CODE_POINT_END};

static AttributeFunc MulticastGemIWTPNotUsed1Func = {
	getGeneralValue,
	setMulticastGemIWTPNotUsed1
};

static AttributeFunc MulticastGemIWTPPPTPCounterFunc = {
	getMulticastGemIWTPPPTPCounter,
	NULL
};

static AttributeFunc MulticastGemIWTPOperStateFunc = {
	getMulticastGemIWTPOperState,
	NULL
};

static AttributeFunc MulticastGemIWTPNotUsed2Func = {
	getGeneralValue,
	setMulticastGemIWTPNotUsed2
};

static AttributeFunc MulticastGemIWTPIPv4MultiAddrTBFunc = {
	getMulticastGemIWTPIPv4MultiAddrTB,
	setMulticastGemIWTPIPv4MultiAddrTB
};

omciAttriDescript_t omciAttriDescriptListMulticastGemIWTP[]={
{0, "meId",						2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,												&generalGetFunc},
{1, "gem port network ctp conn ptr",	2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,0,0,NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,												&generalGetSetFunc},
{2, "iw option",					1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,0,0,MulticastIWOption,	ATTR_FULLY_SUPPORT,	NO_AVC,												&generalGetSetFunc},
{3, "service profile ptr",				2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,0,0,NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,												&generalGetSetFunc},
{4, "not used 1",					2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,												&MulticastGemIWTPNotUsed1Func},
{5, "pptp counter",					1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,												&MulticastGemIWTPPPTPCounterFunc},
{6, "operational state",				1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,				ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_MULTICAST_GEM_IWTP_OPERATIONAL_STATE,	&MulticastGemIWTPOperStateFunc},
{7, "gal profile ptr",				2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,0,0,NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,												&generalGetFunc},
{8, "not used 2",					1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,0,0,NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,												&MulticastGemIWTPNotUsed2Func},
{9, "ipv4 multicast addr table",		12,	ATTR_ACCESS_R_W,				ATTR_FORMAT_TABLE,			0,0,0,NULL,				ATTR_FULLY_SUPPORT,	NO_AVC,												&MulticastGemIWTPIPv4MultiAddrTBFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


