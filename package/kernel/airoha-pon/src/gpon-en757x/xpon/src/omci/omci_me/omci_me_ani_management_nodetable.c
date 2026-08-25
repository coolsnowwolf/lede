
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
	9_12_ME_nodetable.c
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/8/07	Create
	Andy.Yi		        2012/8/07	add 9.2.1,9.2.2
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_ani_management.h"

/*******************************************************************************************************************************
9.2.1 ANI-G

********************************************************************************************************************************/
alarm_id_map_entry_t meMapTableAniG[] = {
{OMCI_ALARM_ID_LOW_RX_OPTICAL},
{OMCI_ALARM_ID_HIGH_RX_OPTICAL},
{OMCI_ALARM_ID_SF},
{OMCI_ALARM_ID_SD},
{OMCI_ALARM_ID_LOW_TX_OPTICAL},	
{OMCI_ALARM_ID_HIGH_TX_OPTICAL},
{OMCI_ALARM_ID_LASER_BIAS_CURRENT},
{0}};

static uint16 piggybackDBAReport[] = {0, 1, 2, 3, 4, CODE_POINT_END};

static AttributeFunc gemBlockLengthFunc = {
	getGeneralValue,
	setGEMBlockLengthValue,
};
static AttributeFunc sfThresholdFunc = {
	getGeneralValue,
	setSFThresholdValue,
};

static AttributeFunc sdThresholdFunc = {
	getGeneralValue,
	setSDThresholdValue,
};

static AttributeFunc aniGARCFunc = {
	getGeneralValue,
	setANIGARCValue,
};

static AttributeFunc opticalSignalLevelFunc = {
	getOpticalSignalLevelValue,
	NULL,
};

static AttributeFunc lowerOpticalThresholdFunc = {
	getGeneralValue,
	setLowerOpticalThresholdValue,
};

static AttributeFunc upperOpticalThresholdFunc = {
	getGeneralValue,
	setUpperOpticalThresholdValue,
};

static AttributeFunc transmitOpticalLevelFunc = {
	getTransmitOpticalLevelValue,
	NULL,
};

static AttributeFunc lowerTransmitOpticalThresholdFunc = {
	getGeneralValue,
	setLowerTransmitOpticalThresholdValue,
};

static AttributeFunc upperTransmitOpticalThresholdFunc = {
	getGeneralValue,
	setUpperTransmitOpticalThresholdValue,
};



omciAttriDescript_t omciAttriDescriptListAniG[] = {
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "SR indication" , 		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x1,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Total T-CONT number", 2 , 	ATTR_ACCESS_R , 	ATTR_FORMAT_UNSIGNED_INT, 	0,	0x000000FF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "GEM block length",		2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &gemBlockLengthFunc},
{4, "Piggyback DBA report",	1,	ATTR_ACCESS_R, 		ATTR_FORMAT_ENUM, 			0, 	0, 			0, 			piggybackDBAReport,		ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{5, "Deprecated",			1,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0, 	0, 			0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{6, "SF threshold",			1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	3, 	8, 			0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &sfThresholdFunc},
{7, "SD threshold",			1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	4, 	10, 			0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &sdThresholdFunc},
{8, "ARC",				1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x1, 		0, 			NULL, 					ATTR_FULLY_SUPPORT, 	OMCI_AVC_ID_ANI_G_ARC, &aniGARCFunc},
{9, "ARC interval",			1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0xFF, 		0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{10, "Optical signal level",	2,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF, 	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &opticalSignalLevelFunc},
{11, "Lower optical threshold",1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x000000FF, 	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &lowerOpticalThresholdFunc},
{12, "Upper optical threshold",1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x000000FF, 	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &upperOpticalThresholdFunc},
{13, "ONU response time",	2,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF, 	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{14, "Transmit optical level",	2,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF, 	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &transmitOpticalLevelFunc},
{15, "Lower transmit optical",1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x000000FF, 	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &lowerTransmitOpticalThresholdFunc},
{16, "Upper transmit optical",1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x000000FF, 	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &upperTransmitOpticalThresholdFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.2.2 T-CONT

********************************************************************************************************************************/
static AttributeFunc tcontAllocIdFunc = {
	getGeneralValue,
	setTCONTAllocIdValue,
};


static AttributeFunc tcontPolicyFunc = {
	getGeneralValue,
	setTCONTPolicyValue,
};

omciAttriDescript_t omciAttriDescriptListTcont[] = {
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 					ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 	ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Alloc-ID" , 			2 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0x00000FFF,	0,			NULL, 	ATTR_FULLY_SUPPORT,	NO_AVC, &tcontAllocIdFunc},
{2, "Deprecated", 			1 , 	ATTR_ACCESS_R , 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0x1,			0,			NULL, 	ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Policy",				1,	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 	ATTR_FULLY_SUPPORT, 	NO_AVC, &tcontPolicyFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.2.3 GEM Port

********************************************************************************************************************************/
static AttributeFunc uniCounterFunc = {
	getUNIcounterValue,
	NULL,
};

static AttributeFunc encryptionStateFunc = {
	getEncryptionStateValue,
	NULL,
};

omciAttriDescript_t omciAttriDescriptListGemPortCtp[] = {
{0, "meId" , 				2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Port-ID" , 			2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "T-CONT pointer", 		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Direction", 			1 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0,	0x00000003,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "TMPointerUp",			2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{5, "TDPointerUp",			2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{6, "UNIcounter",			1,	ATTR_ACCESS_R, 					ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &uniCounterFunc},
{7, "PQPointerDown",		2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{8, "Encryption state",		1,	ATTR_ACCESS_R, 					ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &encryptionStateFunc},
{9, "TDPointerDown",		2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{10, "Encryption key ring",	2,	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, ATTR_UNSUPPORT, 		NO_AVC, NULL},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.2.6 GEM Port PM in G.984.4

********************************************************************************************************************************/

static AttributeFunc gemPortPmThrdDataFunc = {
	getGeneralValue,
	setGemPortPmThrdDataValue,
};

static AttributeFunc gemPortPmLostPacketsFunc = {
	getGemPortPmLostPacketsValue,
	NULL,
};

static AttributeFunc gemPortPmMisinsertedPacketsFunc = {
	getGemPortPmMisinsertedPacketsValue,
	NULL,
};

static AttributeFunc gemPortPmReceivedPacketsFunc = {
	getGemPortPmReceivedPacketsValue,
	NULL,
};

static AttributeFunc gemPortPmReceivedBlocksFunc = {
	getGemPortPmReceivedBlocksValue,
	NULL,
};

static AttributeFunc gemPortPmTransmittedBlocksFunc = {
	getGemPortPmTransmittedBlocksValue,
	NULL,
};

static AttributeFunc gemPortPmImpairedBlocksFunc = {
	getGemPortPmImpairedBlocksValue,
	NULL,
};


omciAttriDescript_t omciAttriDescriptListGemPortPm[] = {
{0, "meId" , 					2 , ATTR_ACCESS_R_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Interval end time" , 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetIntervalEndTimeFunc},
{2, "Threshold data 1/2 id", 	2 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &gemPortPmThrdDataFunc},
{3, "Lost packets", 			4 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	OMCI_TCA_ID_GEM_PORT_PM_LOST_PACKETS, &gemPortPmLostPacketsFunc},
{4, "Misinserted packets",		4 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0, 			NULL, 			ATTR_FULLY_SUPPORT, OMCI_TCA_ID_GEM_PORT_PM_MISINSERTED_PACKETS,  &gemPortPmMisinsertedPacketsFunc},
{5, "Received packets",			5 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0, 			NULL, 			ATTR_FULLY_SUPPORT, NO_AVC,  &gemPortPmReceivedPacketsFunc},
{6, "Received blocks",			5 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0, 			NULL, 			ATTR_FULLY_SUPPORT, NO_AVC,  &gemPortPmReceivedBlocksFunc},
{7, "Transmitted blocks" ,		5 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &gemPortPmTransmittedBlocksFunc},
{8, "Impaired blocks" ,			4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, OMCI_TCA_ID_GEM_PORT_PM_IMPAIRED_PACKETS, &gemPortPmImpairedBlocksFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.2.x reseved for future standardization 351: for HuaWei OLT IOT. get ethernet port statistic

********************************************************************************************************************************/

omciAttriDescript_t omciAttriDescriptListReserved351PM[] = {
{0, "meId" , 					2 , ATTR_ACCESS_R_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Interval end time" , 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetIntervalEndTimeFunc},
{2, "Threshold data 1/2 id", 		2 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "unknown attribute 1", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "unknown attribute 2", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{5, "unknown attribute 3", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{6, "unknown attribute 4", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{7, "unknown attribute 5", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{8, "unknown attribute 6", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{9, "unknown attribute 7", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{10, "unknown attribute 8", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{11, "unknown attribute 9", 		1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{12, "unknown attribute 10", 	1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{13, "unknown attribute 11", 	1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{14, "unknown attribute 12", 	1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{15, "unknown attribute 13", 	1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{16, "unknown attribute 14", 	1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

#ifdef TCSUPPORT_VNPTT
omciAttriDescript_t omciAttriDescriptListReserved250[] = {
{0, "meId" , 					    2 , ATTR_ACCESS_R, 	 ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "unknown attribute 1" ,  		1 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "unknown attribute 2" , 		2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	    	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "unknown attribute 3" ,			2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "unknown attribute 4" ,			2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "unknown attribute 5" ,			4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{6, "unknown attribute 6" , 		4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{7, "unknown attribute 7" , 		4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{8, "unknown attribute 8" ,			4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{9, "unknown attribute 9" ,			4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},


{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

omciAttriDescript_t omciAttriDescriptListReserved347[]={
{0, "meId",				    2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0x1,	0xFFFE,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&generalGetFunc},
{1, "IP operation",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	    0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&generalGetSetFunc},
{2, "MAC address",	        6,	ATTR_ACCESS_R_W,    ATTR_FORMAT_STRING,	        0,	    0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&generalGetSetFunc},
{3, "Onu identifier",		25,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,	        0,	    0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&generalGetSetFunc},
{4, "test 4 name",          16,  ATTR_ACCESS_R_W,   ATTR_FORMAT_STRING,         0,      0,      0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC,     &generalGetSetFunc},
{5, "test 5 name",          16,  ATTR_ACCESS_R_W,   ATTR_FORMAT_STRING,         0,      0,      0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC,     &generalGetSetFunc},
{6, "test 6 name",		    16,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,	        0,	    0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&generalGetSetFunc},
{7, "test 7 name",		    16,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,	        0,	    0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&generalGetSetFunc},
{8, "test 8 name",		    16,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,	        0,	    0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&generalGetSetFunc},
{9, "test 9 name",		    16,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,	        0,	    0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&generalGetSetFunc},
{10, "test 10 name",			16, ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING, 		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, 	&generalGetSetFunc},
{11, "test 11 name",			16, ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING, 		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, 	&generalGetSetFunc},
{12, "test 12 name", 		16, ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING, 		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, 	&generalGetSetFunc},
{13, "test 13 name",		25, ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING, 		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, 	&generalGetSetFunc},

{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};
#endif

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
omciAttriDescript_t omciAttriDescriptListReserved350[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "flow Mapping Mode" , 		        1 , ATTR_ACCESS_R_W,     		ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	        0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "traffic management option" , 		1 , ATTR_ACCESS_R,       		ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	        0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "flow CAR" ,						1 , ATTR_ACCESS_R_W,		    ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "ont transparent" ,					1 , ATTR_ACCESS_R_W,			ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "current mac learning number" ,		2 , ATTR_ACCESS_R,  			ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{6, "mac age time" , 					4 , ATTR_ACCESS_R_W,			ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{7, "ti" ,					            16 ,ATTR_ACCESS_R,			    ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{8, "802.1p_to_queue capability" ,   	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{9, "802.1p_to_queue map" ,				6 , ATTR_ACCESS_R_W,			ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};
omciAttriDescript_t omciAttriDescriptListReserved352[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "gem port" , 		        		2 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "physical port" , 					2 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT, 	1,	255,	    0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "vlan id" ,							2 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "priority" ,					    1 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "SIR" ,								4 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{6, "PIR" , 							4 , ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

omciAttriDescript_t omciAttriDescriptListReserved353[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "version" , 		        		14 ,ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "is committed" , 					1 , ATTR_ACCESS_R, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	    	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "is active" ,						1 , ATTR_ACCESS_R, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{4, "is valid" ,					    1 , ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{5, "crc32" ,							4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

omciAttriDescript_t omciAttriDescriptListReserved367[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R, 	 ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "multicast transfer mode" ,  		1 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "user vlanid" , 					2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	    	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "IGMP packets operation mode" ,		2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "IGMP vlan tag tci value" ,			2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "crc32" ,							4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

omciAttriDescript_t omciAttriDescriptListReserved373[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R, 	 ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "multicast transfer mode" ,  		1 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "user vlanid" , 					2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	    	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "IGMP packets operation mode" ,		2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "IGMP vlan tag tci value" ,			2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "crc32" ,							4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
omciAttriDescript_t omciAttriDescriptListReserved370[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R, 	 ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "multicast transfer mode" ,  		1 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "user vlanid" , 					2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	    	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "IGMP packets operation mode" ,		2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "IGMP vlan tag tci value" ,			2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "crc32" ,							4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

omciAttriDescript_t omciAttriDescriptListReserved65408[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R, 	 ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "multicast transfer mode" ,  		1 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "user vlanid" , 					2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	    	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "IGMP packets operation mode" ,		2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "IGMP vlan tag tci value" ,			2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "crc32" ,							4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

omciAttriDescript_t omciAttriDescriptListReserved65414[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R, 	 ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "multicast transfer mode" ,  		1 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "user vlanid" , 					2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	    	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "IGMP packets operation mode" ,		2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "IGMP vlan tag tci value" ,			2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "crc32" ,							4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

omciAttriDescript_t omciAttriDescriptListReserved65425[] = {
{0, "meId" , 					        2 , ATTR_ACCESS_R, 	 ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "multicast transfer mode" ,  		1 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "user vlanid" , 					2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	    	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "IGMP packets operation mode" ,		2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "IGMP vlan tag tci value" ,			2 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "crc32" ,							4 , ATTR_ACCESS_R_W, ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},

{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};
#endif
#endif

/*******************************************************************************************************************************
9.2.10 Piority queue

********************************************************************************************************************************/
static AttributeFunc maxQueueSizeFunc = {
	getMaxQueueSizeValue,
	NULL,
};

static AttributeFunc allocQueueSizeFunc = {
	getAllocQueueSizeValue,
	setAllocQueueSizeValue,
};

static AttributeFunc relatePortFunc = {
	getRelatePortValue,
	setRelatePortValue,
};

static AttributeFunc trafficSchedPointFunc = {
	getGeneralValue,
	setTrafficSchedPointValue,
};

static AttributeFunc weightGetSetFunc = {
	getWeightValue,
	setWeightValue,
};

#if 1
static AttributeFunc backPressureOperateFunc = {
	getBackPressureOperatetValue,
	setBackPressureOperatetValue,
};

static AttributeFunc backPressureTimeFunc = {
	getBackPressureTimeValue,
	setBackPressureTimeValue,
};

static AttributeFunc backPressureOccurThredFunc = {
	getBackPressureOccurThredValue,
	setBackPressureOccurThredValue,
};

static AttributeFunc backPressureClearThredFunc = {
	getBackPressureClearThredValue,
	setBackPressureClearThredValue,
};
#endif


#if 1
static AttributeFunc packetDropThredFunc = {
	getPacketDropThredValue,
	setPacketDropThredValue,
};

static AttributeFunc packetDropMax_pFunc = {
	getPacketDropMaxpValue,
	setPacketDropMaxpValue,
};

static AttributeFunc queueDropW_qFunc = {
	getQueueDropWqValue,
	setQueueDropWqValue,
};

static AttributeFunc dropPrecedenceColorMarkFunc = {
	getDropPrecedenceColorMarkValue,
	setDropPrecedenceColorMarkValue,
};
#endif



omciAttriDescript_t omciAttriDescriptListPriorityQueue[] = {
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Queue cofig option" , 	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Max queue size", 		2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &maxQueueSizeFunc},
{3, "Allocated queue size", 	2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &allocQueueSizeFunc},
#ifdef TCSUPPORT_PON_ROSTELECOM
{4, "Discard counter reset interval",	2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{5, "Threshold for discarded blocks",	2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
#else
{4, "Discard counter reset interval",	2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_UNSUPPORT, 	NO_AVC, NULL},
{5, "Threshold for discarded blocks",	2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_UNSUPPORT, 	NO_AVC, NULL},
#endif
{6, "Related port",					4,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &relatePortFunc},
{7, "Traffic scheduler pointer",		2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &trafficSchedPointFunc},
{8, "Weight",						1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &weightGetSetFunc},
{9, "Back pressure operation",		2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &backPressureOperateFunc},
{10, "Back pressure time",			4,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &backPressureTimeFunc},
{11, "Back pressure occur threshold",			2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 			ATTR_FULLY_SUPPORT, 	NO_AVC, &backPressureOccurThredFunc},
{12, "Back pressure clear threshold",			2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 			ATTR_FULLY_SUPPORT, 	NO_AVC, &backPressureClearThredFunc},
{13, "Packet drop thresholds",				8,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 			ATTR_PART_SUPPORT, 	NO_AVC, &packetDropThredFunc},
{14, "Packet drop max_p",					2,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 			ATTR_PART_SUPPORT, 	NO_AVC, &packetDropMax_pFunc},
{15, "Queue drop w_q",						1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 			ATTR_PART_SUPPORT, 	NO_AVC, &queueDropW_qFunc},
{16, "Drop precedence colour marking",		1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 			ATTR_PART_SUPPORT, 	NO_AVC, &dropPrecedenceColorMarkFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.2.11 Traffic Scheduler

********************************************************************************************************************************/
static AttributeFunc tcontPointFunc = {
	getGeneralValue,
	setTcontPointValue,
};

static AttributeFunc trafficSchedulerPolicyFunc = {
	getGeneralValue,
	setTcontPolicyValue,
};

static AttributeFunc tcontPriorityWeightFunc = {
	getGeneralValue,
	setTcontPriorityWeightValue,
};


omciAttriDescript_t omciAttriDescriptListTrafficScheduler[] = {
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "T-CONT pointer" , 		2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &tcontPointFunc},
{2, "Traffic Scheduler pointer", 		2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Policy", 				1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &trafficSchedulerPolicyFunc},
{4, "Priority_Weight",	1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &tcontPriorityWeightFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.2.12 Traffic Descriptor

********************************************************************************************************************************/
#if 0//remove now, may be used in future
static AttributeFunc TrafficDescriptorCIRFunc = {
	getTrafficDescriptorCIRValue,
	setTrafficDescriptorCIRValue,
};

static AttributeFunc TrafficDescriptorPIRFunc = {
	getTrafficDescriptorPIRValue,
	setTrafficDescriptorPIRValue,
};

static AttributeFunc TrafficDescriptorCBSFunc = {
	getTrafficDescriptorCBSValue,
	setTrafficDescriptorCBSValue,
};

static AttributeFunc TrafficDescriptorPBSFunc = {
	getTrafficDescriptorPBSValue,
	setTrafficDescriptorPBSValue,
};

static AttributeFunc TrafficDescriptorColourModeFunc = {
	getTrafficDescriptorColourModeValue,
	setTrafficDescriptorColourModeValue,
};

static AttributeFunc TrafficDescriptorInMarkFunc = {
	getTrafficDescriptoraInMarkingValue,
	setTrafficDescriptoraInMarkingValue,
};

static AttributeFunc TrafficDescriptorEnMarkFunc = {
	getTrafficDescriptoraEnMarkingValue,
	setTrafficDescriptoraEnMarkingValue,
};

static AttributeFunc TrafficDescriptorMeterTypeFunc = {
	getTrafficDescriptorMeterTypeValue,
	setTrafficDescriptorMeterTypeValue,
};
#endif


omciAttriDescript_t omciAttriDescriptListTrafficDescriptor[] = {
{0, "meId" , 		2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "CIR" , 		4 , 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "PIR", 		4 , 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "CBS", 		4 , 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "PBS",		4 ,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &generalGetSetFunc},
{5, "Colour mode",				1 ,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &generalGetSetFunc},
{6, "Ingress colour marking",	1 ,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &generalGetSetFunc},
{7, "Egress colour marking" ,	1 ,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &generalGetSetFunc},
{8, "Meter type" ,	1 ,	ATTR_ACCESS_R_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.2.13 GEM port network CTP performance monitoring history data

********************************************************************************************************************************/
static AttributeFunc gemPortCtpPmThrdDataFunc = {
	getGeneralValue,
	setGemPortCtpPmThrdDataValue,
};

static AttributeFunc gemPortCtpPmTransmitFramesFunc = {
	getGemPortCtpPmTransmitFramesValue,
	NULL,
};

static AttributeFunc gemPortCtpPmReceiveFramesFunc = {
	getGemPortCtpPmReceiveFramesValue,
	NULL,
};

static AttributeFunc gemPortCtpPmReceiveBytesFunc = {
	getGemPortCtpPmReceiveBytesValue,
	NULL,
};

static AttributeFunc gemPortCtpPmTransmitBytesFunc = {
	getGemPortCtpPmTransmitBytesValue,
	NULL,
};

omciAttriDescript_t omciAttriDescriptListGemportNetworkCtpPm[] = {
{0, "meId" , 		2 , 	ATTR_ACCESS_R_SET_CREATE, 						ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Interval end time" , 		1 , 	ATTR_ACCESS_R, 						ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetIntervalEndTimeFunc},
{2, "Threshold data 1/2 id", 		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &gemPortCtpPmThrdDataFunc},
{3, "Transmitted GEM frames", 	4 , 	ATTR_ACCESS_R, 						ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &gemPortCtpPmTransmitFramesFunc},
{4, "Received GEM frames",		4 ,	ATTR_ACCESS_R, 						ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &gemPortCtpPmReceiveFramesFunc},
{5, "Received payload bytes",	8 ,	ATTR_ACCESS_R, 						ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &gemPortCtpPmReceiveBytesFunc},
{6, "Transmitted payload bytes",	8 ,	ATTR_ACCESS_R, 						ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &gemPortCtpPmTransmitBytesFunc},
{7, "Encryption key errors" ,		8 ,	ATTR_ACCESS_R, 						ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0,			NULL, 					ATTR_UNSUPPORT,	OMCI_TCA_ID_GEM_PORT_NETWORK_CTP_PM, NULL},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


/*******************************************************************************************************************************
9.2.14 Energy consumption performance monitoring history data

********************************************************************************************************************************/
static AttributeFunc energyComsumPmThrdDataFunc = {
	getGeneralValue,
	setEnergyComsumPmThrdDataValue,
};

static AttributeFunc energyComsumPmDozeTimeFunc = {
	getEnergyComsumPmTransmitFramesValue,
	NULL,
};

static AttributeFunc energyComsumPmCyclicSleepTimeFunc = {
	getEnergyComsumPmCyclicSleepTimeValue,
	NULL,
};

static AttributeFunc energyComsumPmEnergyConsumedFunc = {
	getEnergyComsumPmEnergyConsumedValue,
	NULL,
};

omciAttriDescript_t omciAttriDescriptListEnergyConsumptionPm[] = {
{0, "meId" , 		2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Interval end time" , 		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetIntervalEndTimeFunc},
{2, "Threshold data 1/2 id", 		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &energyComsumPmThrdDataFunc},
{3, "Doze time", 			4 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x0000FFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &energyComsumPmDozeTimeFunc},
{4, "Cyclic sleep time",		4 ,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &energyComsumPmCyclicSleepTimeFunc},
{5, "Energy consumed",		4 ,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT, 	0, 	0x0000FFFF,	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC,  &energyComsumPmEnergyConsumedFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


#ifdef TCSUPPORT_OMCI_ALCATEL
/*******************************************************************************************************************************
ALCATEL Vendor Specific ME:   Total GEM Port PM

********************************************************************************************************************************/

static AttributeFunc totalGemPortPmThrdDataFunc = {
	getGeneralValue,
	setTotalGemPortPmThrdDataValue,
};

static AttributeFunc totalGemPortPmLostCntDownFunc = {
	getTotalGemPortPmLostCntDownValue,
	NULL,
};

static AttributeFunc totalGemPortPmLostCntUpFunc = {
	getTotalGemPortPmLostCntUpValue,
	NULL,
};

static AttributeFunc totalGemPortPmReceivedCntFunc = {
	getTotalGemPortPmReceivedCntValue,
	NULL,
};

static AttributeFunc totalGemPortPmReceivedBlocksFunc = {
	getTotalGemPortPmReceivedBlocksValue,
	NULL,
};

static AttributeFunc totalGemPortPmTransmittedBlocksFunc = {
	getTotalGemPortPmTransmittedBlocksValue,
	NULL,
};

static AttributeFunc totalGemPortPmImpairedBlocksFunc = {
	getTotalGemPortPmImpairedBlocksValue,
	NULL,
};

static AttributeFunc totalGemPortPmTransmittedCntFunc = {
	getTotalGemPortPmTransmittedCntValue,
	NULL,
};

static AttributeFunc totalGemPortPmBadReceivedCntFunc = {
	getTotalGemPortPmBadReceivedCntValue,
	NULL,
};

static AttributeFunc totalGemPortPmReceivedBlocks64Func = {
	getTotalGemPortPmReceivedBlocks64Value,
	NULL,
};

static AttributeFunc totalGemPortPmTransmittedBlocks64Func = {
	getTotalGemPortPmTransmittedBlocks64Value,
	NULL,
};


omciAttriDescript_t omciAttriDescriptListTotalGemPortPm[] = {
{0, "meId" , 								2 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Interval end time" , 					1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetIntervalEndTimeFunc},
{2, "Threshold data 1/2 id", 				2 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &totalGemPortPmThrdDataFunc},
{3, "Lost GEM Fragment Counter Downstream", 4 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	OMCI_TCA_ID_TOTAL_GEM_PORT_PM_LOST_COUNTER_DOWNSTREAM, &totalGemPortPmLostCntDownFunc},
{4, "Lost GEM Fragment Counter Upstream", 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, OMCI_TCA_ID_TOTAL_GEM_PORT_PM_LOST_COUNTER_UPSTREAM, &totalGemPortPmLostCntUpFunc},
{5, "Received GEM Fragment Counter",		5 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0, 			NULL, 			ATTR_FULLY_SUPPORT, NO_AVC,  &totalGemPortPmReceivedCntFunc},
{6, "Received GEM Blocks",					5 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0, 			NULL, 			ATTR_FULLY_SUPPORT, NO_AVC,  &totalGemPortPmReceivedBlocksFunc},
{7, "Transmitted GEM Blocks" ,				5 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &totalGemPortPmTransmittedBlocksFunc},
{8, "Impaired blocks" ,						4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, OMCI_TCA_ID_TOTAL_GEM_PORT_PM_IMPAIRED_BLOCKS, &totalGemPortPmImpairedBlocksFunc},
{9, "Transmitted GEM Fragment Counter" , 	5 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &totalGemPortPmTransmittedCntFunc},
{10, "Bad GEM Headers Received Counter" , 	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, OMCI_TCA_ID_TOTAL_GEM_PORT_PM_BAD_RECEIVED_CONTERS, &totalGemPortPmBadReceivedCntFunc},
{11, "Received GEM Blocks 64Bit" , 			8 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &totalGemPortPmReceivedBlocks64Func},
{12, "Transmitted GEM Blocks 64Bit" , 		8 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &totalGemPortPmTransmittedBlocks64Func},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


/*******************************************************************************************************************************
ALCATEL Vendor Specific ME:   Ethernet Traffic PM

********************************************************************************************************************************/

static AttributeFunc EthernetTrafficPmThrdDataFunc = {
	getGeneralValue,
	setEthernetTrafficPmThrdDataValue,
};

static AttributeFunc EthernetTrafficPmFramesUpFunc = {
	getEthernetTrafficPmFramesUpValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmFramesDownFunc = {
	getEthernetTrafficPmFramesDownValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmBytesUpFunc = {
	getEthernetTrafficPmBytesUpValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmBytesDownFunc = {
	getEthernetTrafficPmBytesDownValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmDropFramesUpFunc = {
	getEthernetTrafficPmDropFramesUpValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmDropBytesUpFunc = {
	getEthernetTrafficPmDropBytesUpValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmDropFramesDownFunc = {
	getEthernetTrafficPmDropFramesDownValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmMulticastFramesUpFunc = {
	getEthernetTrafficPmMulticastFramesUpValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmMulticastFramesDownFunc = {
	getEthernetTrafficPmMulticastFramesDownValue,
	NULL,
};

static AttributeFunc EthernetTrafficPmBytesUp64Func = {
	getEthernetTrafficPmBytesUp64Value,
	NULL,
};

static AttributeFunc EthernetTrafficPmBytesDown64Func = {
	getEthernetTrafficPmBytesDown64Value,
	NULL,
};

static AttributeFunc EthernetTrafficPmFramesUp64Func = {
	getEthernetTrafficPmFramesUp64Value,
	NULL,
};

static AttributeFunc EthernetTrafficPmFramesDown64Func = {
	getEthernetTrafficPmFramesDown64Value,
	NULL,
};


omciAttriDescript_t omciAttriDescriptListEthernetTrafficPm[] = {
{0, "meId" , 									2 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Interval end time" , 						1 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetIntervalEndTimeFunc},
{2, "Threshold data 1/2 id", 					2 , ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &EthernetTrafficPmThrdDataFunc},
{3, "Upstream Frames Counter",					4 , ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &EthernetTrafficPmFramesUpFunc},
{4, "Downstream Frames Counter", 				4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmFramesDownFunc},
{5, "Upstream Bytes Counter",					4 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0, 			NULL, 			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmBytesUpFunc},
{6, "Downstream Bytes Counter",					4 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0, 			NULL, 			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmBytesDownFunc},
{7, "Dropped Upstream Frames Counter" ,			4 ,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT, 	0, 	0,	0,			NULL, 			ATTR_FULLY_SUPPORT,	OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_FRAMES_UPSTREAM_COUNTER, &EthernetTrafficPmDropFramesUpFunc},
{8, "Dropped Upstream Bytes Counter" ,			4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_BYTES_UPSTREAM_COUNTER, &EthernetTrafficPmDropBytesUpFunc},
{9, "Dropped Downstream Frames Counter" , 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_FRAMES_DOWNSTREAM_COUNTER, &EthernetTrafficPmDropFramesDownFunc},
{10, "Multicast Upstream Frames Counter" , 		4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmMulticastFramesUpFunc},
{11, "Multicast Downstream Frames Counter" ,	4 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmMulticastFramesDownFunc},
{12, "Upstream Bytes Counter 64Bit" , 			8 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmBytesUp64Func},
{13, "Downstream Bytes Counter 64Bit" , 		8 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmBytesDown64Func},
{14, "Upstream Frames Counter 64Bit" , 			8 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmFramesUp64Func},
{15, "Downstream Frames Counter 64Bit" , 		8 , ATTR_ACCESS_R,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,			NULL,			ATTR_FULLY_SUPPORT, NO_AVC, &EthernetTrafficPmFramesDown64Func},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


static AttributeFunc VlanTxDefaultTagFunc = {
	NULL,
	setVlanTxDefaultTagValue,
};

static AttributeFunc VlanTxBehaviorTableFunc = {
	NULL,
	setVlanTxBehaviorTableValue,
};

omciAttriDescript_t omciAttriDescriptListVlanTagDownstreamSupplemental1[] = {
{0, "meId" , 					2 , ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,	NULL, 	ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Vlan Tx Default Tag" ,		1 , ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &VlanTxDefaultTagFunc},
{2, "Vlan Tx Behavior Table" ,	6 , ATTR_ACCESS_R_W,	ATTR_FORMAT_TABLE,		 	0,	0,			0,	NULL, 	ATTR_FULLY_SUPPORT,	NO_AVC, &VlanTxBehaviorTableFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

#if defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)
static AttributeFunc VlanRxPolicerTableFunc = {
	NULL,
	setVlanRxPolicerTableValue,
};

omciAttriDescript_t omciAttriDescriptListVlanTagUpstreamPolicer[] = {
{0, "meId" ,                    2 , ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT, 0, 0xFFFF,    0,      NULL,   ATTR_FULLY_SUPPORT,     NO_AVC, &generalGetFunc},
{1, "Vlan Rx Policer Table" ,   19, ATTR_ACCESS_W,      ATTR_FORMAT_TABLE,        0, 0,         0,      NULL,   ATTR_FULLY_SUPPORT,     NO_AVC, &VlanRxPolicerTableFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};
#endif // #if defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)

static AttributeFunc GEMPortProtocolMonitoringHistoryDataPart2ThresholdDataFunc = {
	getGeneralValue,
	setGEMPortProtocolMonitoringHistoryDataPart2ThresholdData
};

static AttributeFunc GEMPortProtocolMonitoringHistoryDataPart2LostGEMFragmentCounterFunc = {
	getGEMPortProtocolMonitoringHistoryDataPart2LostGEMFragmentCounter,
	NULL
};

static AttributeFunc GEMPortProtocolMonitoringHistoryDataPart2TransmittedGEMFragmentCounterFunc = {
	getGEMPortProtocolMonitoringHistoryDataPart2TransmittedGEMFragmentCounter,
	NULL
};

omciAttriDescript_t omciAttriDescriptListGEMPortProtocolMonitoringHistoryDataPart2[]={
{0, "meId",									2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0x01,	0xFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{1, "interval end time",							1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",						2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&GEMPortProtocolMonitoringHistoryDataPart2ThresholdDataFunc},
{3, "Lost GEM Fragment Counter(Upstream)",		4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_TCA_ID_LOST_GEM_FRAGMENT_COUNTER,	&GEMPortProtocolMonitoringHistoryDataPart2LostGEMFragmentCounterFunc},
{4, "Transmitted GEM Fragment Counter",			5,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &GEMPortProtocolMonitoringHistoryDataPart2TransmittedGEMFragmentCounterFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};
#endif

