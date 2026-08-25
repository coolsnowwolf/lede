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
	omci_me_equipment_management_nodetable.c
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/7/16	Create
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_equipment_management.h"

/*******************************************************************************************************************************
9.1.1 ONU-G

********************************************************************************************************************************/
alarm_id_map_entry_t meMapTableOnuG[] = {
{OMCI_ALARM_ID_EQUIPMENT},
//{OMCI_ALARM_ID_POWERING},
//{OMCI_ALARM_ID_BATTERY_MISS},
//{OMCI_ALARM_ID_BATTERY_FAIL},
//{OMCI_ALARM_ID_BATTERY_LOW},	
//{OMCI_ALARM_ID_PHYSICAL_INSTU},
{OMCI_ALARM_ID_ONU_SELFTEST_FAIL},
{OMCI_ALARM_ID_ONU_DYING_GASP},
{OMCI_ALARM_ID_ONU_TEMPER_YELLOW},
{OMCI_ALARM_ID_ONU_TEMPER_RED},	
{OMCI_ALARM_ID_ONU_VOLTAGE_YELLOW},
{OMCI_ALARM_ID_ONU_VOLTAGE_RED},
{OMCI_ALARM_ID_ONU_ONU_MANANA_POWER_OFF},
{OMCI_ALARM_ID_ONU_INV_IMAGE},
//{OMCI_ALARM_ID_ONU_PSE_OVERLOAD_YELLOW},	
//{OMCI_ALARM_ID_ONU_PSE_OVERLOAD_RED},
{0}};
static uint16 TMOption[] = {0, 1, 2, CODE_POINT_END};
static uint16 CredentialsStatus[] = {0, 1, 2, 3, 4, CODE_POINT_END};
static AttributeFunc ONUGAdminStateFunc = {
	getGeneralValue,
	setONUGAdminStateValue,
};

static AttributeFunc ONUGOpStateFunc = {
	getONUGOpStateValue,
	NULL,
};

#ifdef TCSUPPORT_CUC

static AttributeFunc ONUGGetLOID = {
	getONUGLOID,
	NULL,
};

static AttributeFunc ONUGGetLOIDPWD = {
	getONUGLOIDPWD,
	NULL,
};

#else

#define ONUGGetLOID     generalGetFunc
#define ONUGGetLOIDPWD  generalGetFunc

#endif

omciAttriDescript_t omciAttriDescriptListOnuG[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Vendor id" , 			4 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Version" , 			14 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Serial number" , 		8 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "TM option" , 			1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_ENUM,			0,	0,			0,			TMOption, 		ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{5, "Deprecated" , 			1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{6, "Battery backup" ,		1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,			NULL, 			ATTR_PART_SUPPORT,	NO_AVC, &generalGetSetFunc},
{7, "Administrative state" ,	1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &ONUGAdminStateFunc},
{8, "Operational state" ,		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,			NULL, 			ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_ONU_G_OP_STATE, &ONUGOpStateFunc},
{9, "ONU survival time" ,	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,			NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{10, "Logical ONU ID" ,		24 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,		OMCI_AVC_ID_ONU_G_LOID, &ONUGGetLOID},
{11, "Logical password" ,	12 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, 			ATTR_FULLY_SUPPORT,		OMCI_AVC_ID_ONU_G_LPW, &ONUGGetLOIDPWD},
{12, "Credentials status" ,	1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_ENUM,			0,	0,			0,			CredentialsStatus,ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},	
{13, "Extended TC-layer" ,	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_BIT_FIELD,		0,	0x1,			0x03,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}	
};



/*******************************************************************************************************************************
9.1.2 ONU2-G

********************************************************************************************************************************/

static uint16 OMCCVersion[] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x96, 0xA0, 0xA1, 0xB0, 0xB1, CODE_POINT_END};
static uint16 SecurityCapability[] = {1, CODE_POINT_END};
static AttributeFunc OMCCVersionFunc = {
	getOMCCVersionValue,
	NULL,
};
static AttributeFunc SysUpTimeFunc = {
	getSysUpTimeValue,
	NULL,
};

static AttributeFunc SecurityModeFunc = {
	getGeneralValue,
	setSecurityModeValue,
};
omciAttriDescript_t omciAttriDescriptListOnu2G[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Equipment id" , 		20 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "OMCC version" , 		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_ENUM,			0,	0,			0,		OMCCVersion, 	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_ONU2_G_OMCC_VERSION, &OMCCVersionFunc},
{3, "Vendor product code" , 	2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "Security capability" , 	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_ENUM,			0,	0,			0,		SecurityCapability,ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{5, "Security mode" , 		1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_ENUM,			0,	0,			0,		SecurityCapability,ATTR_FULLY_SUPPORT,	NO_AVC, &SecurityModeFunc},
{6, "Total priority queue" ,	2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{7, "Total traffic scheduler" ,	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{8, "Deprecated" ,			1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{9, "Total GEM port-ID" ,	2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{10, "SysUpTime" ,			4 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &SysUpTimeFunc},
{11, "Connectivity capability" ,2,	ATTR_ACCESS_R, 		ATTR_FORMAT_BIT_FIELD,		0,	0,			0xFF,	NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{12, "Connectivity mode" ,	1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_BIT_FIELD,		0,	0,			0xFF,	NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{13, "QoS configuration" ,	2,	ATTR_ACCESS_R, 		ATTR_FORMAT_BIT_FIELD,		0,	0,			0xFF,	NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{14, "PQ scale factor" ,		2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,		NULL, 			ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};



/*******************************************************************************************************************************
9.1.3 ONU data

********************************************************************************************************************************/
static AttributeFunc MIBDataSyncFunc = {
	getMIBDataSyncValue,
	setMIBDataSyncValue,
};
omciAttriDescript_t omciAttriDescriptListOnuData[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "MIB data sync" , 	1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &MIBDataSyncFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.4 Software image

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListSoftwareImage[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Version" , 			14 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,				0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Is committed" , 		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Is active" , 			1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "Is valid" , 			1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{5, "Product code" , 		25 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,				0,	0,			0,			NULL, 					ATTR_UNSUPPORT,	NO_AVC, &generalGetFunc},
{6, "Image hash" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 					ATTR_UNSUPPORT,	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


/*******************************************************************************************************************************
9.1.5 Cardholder

********************************************************************************************************************************/
static uint16 ProtectionSwitch[] = {0, 1, 2, 3, CODE_POINT_END};
omciAttriDescript_t omciAttriDescriptListCardholder[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, 				ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Actual unit type" , 		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,NULL, 				ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Expected unit type" ,	1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,NULL, 				ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Expected port count" ,	1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,NULL, 				ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "Expected equipment id" ,20 , ATTR_ACCESS_R_W, 	ATTR_FORMAT_STRING,		0,	0x000000FF,	0,NULL, 				ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{5, "Actual equipment id" ,	20 , ATTR_ACCESS_R_W, 	ATTR_FORMAT_STRING,		0,	0x000000FF,	0,NULL, 				ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetSetFunc},
{6, "Protection pointer" ,	2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_POINTER,		0,	0x0000FFFF,	0,NULL, 				ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetFunc},
#ifdef TCSUPPORT_PON_ROSTELECOM
{7, "Protection switch" ,		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_ENUM,			0,	0,			0,ProtectionSwitch, 	ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetFunc},
{8, "ARC" ,				1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,NULL, 				ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetSetFunc},
{9, "ARC interval" ,		1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1	,		0,NULL, 				ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetSetFunc},
#else
{7, "Protection switch" ,		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_ENUM,			0,	0,			0,ProtectionSwitch, 	ATTR_UNSUPPORT,		NO_AVC, &generalGetFunc},
{8, "ARC" ,				1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,NULL, 				ATTR_UNSUPPORT,		NO_AVC, &generalGetSetFunc},
{9, "ARC interval" ,		1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1	,		0,NULL, 				ATTR_UNSUPPORT,		NO_AVC, &generalGetSetFunc},
#endif
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.6 Circuit pack

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListCircuitPack[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Type" , 				1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Number of ports" ,		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Serial number" ,		8 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "Version" ,				14 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{5, "Vendor id" ,			4 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetFunc},
{6, "Administrative state" ,	1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{7, "Operational state" ,		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,		NULL, ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetFunc},
{8, "Bridged or IP ind" ,		1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,			0,		NULL, ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetSetFunc},
{9, "Equipment id" ,		20 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0x1	,		0,		NULL, ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetFunc},
{10, "Card configuration" ,	1 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT,		0,	0x000000FF	,		0,		NULL, ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetSetFunc},
{11, "Total T-CONT buffer" ,	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{12, "Total priority queue" ,	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{13, "Total traffic scheduler",	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{14, "Power shed override" ,	4 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_BIT_FIELD,		0,	0x000000FF,	0,		NULL, ATTR_FULLY_SUPPORT,		NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.7 ONU power shedding

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListOnuPowerShedding[]={
{0, "meId" , 					2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Restore power timer reset" , 	2, 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Data class shedding" , 		2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "Voice class shedding" , 		2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{5, "Video overlay class shedding", 2 , ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{6, "Video return class shedding", 2, 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{7, "DSL class shedding" , 		2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{8, "ATM class shedding " , 		2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{9, "CES class shedding" , 		2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{10, "Frame class shedding" , 	2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{11, "Sdh-sonet class shedding",	2,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{12, "Shedding status",			1,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.8 Port mapping package 

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListPortMapping[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Max ports" , 		1, 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Port list 1" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Port list 2" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "Port list 3" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{5, "Port list 4" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{6, "Port list 5" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{7, "Port list 6" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{8, "Port list 7" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{9, "Port list 8" , 		16 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{10, "Combined port table",0,ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_UNSUPPORT,		NO_AVC, NULL},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.9 Equipment extension package 

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListExtensionPackage[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Environmental sense" , 	2, 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Contact closure" , 		2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.10 Protection data 

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListProtectionData[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Working pointer" , 		2, 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Protection pointer" , 	2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Protection type" , 		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, ATTR_UNSUPPORT,		NO_AVC, NULL},
{4, "Revertive ind" , 		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, ATTR_UNSUPPORT,		NO_AVC, NULL},
{5, "Wait to restore time" , 	2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, ATTR_UNSUPPORT,		NO_AVC, NULL},
{6, "Switching guard time" , 	2 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, ATTR_UNSUPPORT,		NO_AVC, NULL},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.11 Equipment protection profile 

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListProtectionProfile[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R_SET_CREATE, 			ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Protect slot 1" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Protect slot 2" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Working slot 1" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "Working slot 2" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{5, "Working slot 3" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{6, "Working slot 4" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{7, "Working slot 5" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{8, "Working slot 6" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{9, "Working slot 7" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{10, "Working slot 8" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{11, "Protect status 1" , 	1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{12, "Protect status 2" , 	1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{13, "Revertive ind" , 		1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{14, "Wait to restore time" , 	1, 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.12 ONU remote debug

********************************************************************************************************************************/
static AttributeFunc remoteDebugCmdFunc = {
	NULL,
	setRemoteDebugCmd,
};
static AttributeFunc remoteDebugReplyTabFunc = {
	getRemoteDebugReplyTab,
	NULL,
};
omciAttriDescript_t omciAttriDescriptListOunRemoteDebug[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Command format" , 	1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Command" , 			25 , 	ATTR_ACCESS_W, 	ATTR_FORMAT_STRING,				0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &remoteDebugCmdFunc},
{3, "Reply table" , 			0 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_TABLE,				0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &remoteDebugReplyTabFunc},
{255, NULL , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.13 ONU-E 

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListOnuE[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Vendor id" , 		4 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Version" , 		14 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Serial number" , 	8 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,			NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.1.14 ONU dynamic power management control

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListOnuDynamicPower[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Power reduction" , 		1 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_BIT_FIELD,		0,	0,			0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Power reduction mode" ,1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_BIT_FIELD,		0,	0,			0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Itransinit" , 			2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "Itxinit" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,			0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{5, "Max sleep interval" , 	4 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{6, "Min aware interval" , 	4 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{7, "Min active held interval",4 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

#ifdef TCSUPPORT_OMCI_ALCATEL
static AttributeFunc powerFeedVoltageFunc = {
	getPowerFeedVoltageValue,
	NULL,
};

static AttributeFunc receiveOpticalPowerFunc = {
	getReceiveOpticalPowerValue,
	NULL,
};

static AttributeFunc transmitOpticalPowerFunc = {
	getTransmitOpticalPowerValue,
	NULL,
};

static AttributeFunc laserBiasCurrentFunc = {
	getLaserBiasCurrentValue,
	NULL,
};

static AttributeFunc temperatureFunc = {
	getTemperatureValue,
	NULL,
};

omciAttriDescript_t omciAttriDescriptListONTOpticalSupervisionStatus[]={
{0, "meId",									2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "Power feed voltage measurement indicator",	1,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{2, "Power Feed Voltage",						2,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &powerFeedVoltageFunc},
{3, "Receive optical power measurement indicator",	1,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{4, "Receive Optical Power",						2,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &receiveOpticalPowerFunc},
{5, "Transmit optical power measurement indicator",	1,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{6, "Transmit Optical Power",					2,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &transmitOpticalPowerFunc},
{7, "Laser Bias Current measurement indicator", 	1,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{8, "Laser Bias Current", 						2, 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &laserBiasCurrentFunc},
{9, "Temperature measurement indicator", 			1, 	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{10, "Temperature", 							2, 	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &temperatureFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


static AttributeFunc ethernetPortToPortTrafficEnableIndicatorFunc = {
	getEthernetPortToPortTrafficEnableIndicatorValue,
	setEthernetPortToPortTrafficEnableIndicatorValue,
};

omciAttriDescript_t omciAttriDescriptListONTGenericV2[]={
{0, "meId",									2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "IGMPSnoopEnable",						1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{2, "ForceDataStorageCommand",				1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{3, "TimeOfLastDataStorageOperation",			4,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{4, "CraftEnableDisableIndicator",				1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "EthernetPortToPortTrafficEnableIndicator",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &ethernetPortToPortTrafficEnableIndicatorFunc},
{6, "StaticMulticastVLANTranslationIndicator",		1,	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{7, "ALUProprietaryMECapabilitiesBitmap",		 	4,	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{8, "FlexiblePQAssignmentCapability", 				1, 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{9, "SLIDCapability", 							1, 	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{10, "YPSerialNumber", 							12, 	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{11, "ALUSpecialBehaviorsField",					4,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{12, "ALUProprietaryMECapabilitiesBitmapPart2",	25,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};
#endif
