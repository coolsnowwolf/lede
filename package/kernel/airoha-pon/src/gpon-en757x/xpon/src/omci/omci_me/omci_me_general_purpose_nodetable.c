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
	omci_me_general_purpose_nodetable.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/7/16	Create
        Andy.Yi                 2012/7/27       add the 9.12.1-11 me node
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_general_purpose.h"

/*******************************************************************************************************************************
9.12.1 UNI-G

********************************************************************************************************************************/
static uint16 ManagementCapability[] = {ONLY_OMCI, ONLY_NOOMCI, OMCI_AND_NOOMCI, CODE_POINT_END};
omciAttriDescript_t omciAttriDescriptListUniG[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 					ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Deprecated" , 			2 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Administrative state", 	1 , 	ATTR_ACCESS_R_W , 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0x1,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Management capability",1,	ATTR_ACCESS_R, 					ATTR_FORMAT_ENUM, 			0, 	0, 			0, 			ManagementCapability, 	ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{4, "Non-OMCI manageid",	2,	ATTR_ACCESS_R_W, 				ATTR_FORMAT_POINTER, 		0, 	0xFFFFFFFF, 	0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetSetFunc},
{5, "Relay Agent Options",	2,	ATTR_ACCESS_R_W, 				ATTR_FORMAT_POINTER, 		0, 	0xFFFFFFFF, 	0, 			NULL, 					ATTR_UNSUPPORT, 	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};



/*******************************************************************************************************************************
9.12.2 OLT-G

********************************************************************************************************************************/

static AttributeFunc OLTVendorIdFunc = {
	getGeneralValue,
	setOLTVendorIdValue,
};
static AttributeFunc OLTEquipmentIdFunc = {
	getGeneralValue,
	setOLTEquipmentIdValue,
};
static AttributeFunc OLTVersionFunc = {
	getGeneralValue,
	setOLTVersionValue,
};
static AttributeFunc TimeOfDayFunc = {
	getTimeOfDayValue,
	setTimeOfDayValue,
};

omciAttriDescript_t omciAttriDescriptListOltG[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 					ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "OLT vendor id" , 		4 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT, 	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &OLTVendorIdFunc},
{2, "Equipment id", 		20 , 	ATTR_ACCESS_R_W , 				ATTR_FORMAT_STRING, 		0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &OLTEquipmentIdFunc},
{3, "Version",				14,	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING, 		0, 	0, 			0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &OLTVersionFunc},
{4, "Time of day",			14,	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING, 		0, 	0, 			0, 			NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &TimeOfDayFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

	
/*******************************************************************************************************************************
9.12.3 Network address 

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListNetworkAddress[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,		0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Security pointer" , 	2 , 	ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_POINTER, 			0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Address pointer" , 	2 , 	ATTR_ACCESS_R_W_SET_CREATE , 	ATTR_FORMAT_POINTER, 			0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


/*******************************************************************************************************************************
9.12.4 Authentication security method 

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListAuthenticationMethod[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Validation scheme" ,1 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Username 1" , 	25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Password" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "Realm" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{5, "Username 2" , 	25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.12.5 Large string 

********************************************************************************************************************************/
static AttributeFunc largeStringGetSetFunc = {
	getGeneralValue,
#ifdef TCSUPPORT_CWMP
	setLargeStringValue,
#else
    setGeneralValue,
#endif
};


omciAttriDescript_t omciAttriDescriptListLargeString[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Number of parts" , 	1 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Part 1" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{3, "Part 2" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{4, "Part 3" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{5, "Part 4" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{6, "Part 5" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{7, "Part 6" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{8, "Part 7" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{9, "Part 8" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{10, "Part 9" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{11, "Part 10" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{12, "Part 11" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{13, "Part 12" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{14, "Part 13" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{15, "Part 14" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{16, "Part 15" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &largeStringGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


	
/*******************************************************************************************************************************
9.12.6 Threshold data 1

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListThresholdData1[]={
{0, "meId" , 			2, 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Threshold value 1",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Threshold value 2",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Threshold value 3",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "Threshold value 4",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{5, "Threshold value 5",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{6, "Threshold value 6",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{7, "Threshold value 7",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.12.7 Threshold data 2

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListThresholdData2[]={
{0, "meId" , 				2, 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Threshold value 8",		4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Threshold value 9",		4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Threshold value 10",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "Threshold value 11",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{5, "Threshold value 12",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{6, "Threshold value 13",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{7, "Threshold value 14",	4, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


/*******************************************************************************************************************************
9.12.8 OMCI

********************************************************************************************************************************/
static AttributeFunc METypeTableFunc = {
	getMETypeTableValue,
	NULL,
};	

static AttributeFunc MessageTypeTableFunc = {
	getMessageTypeTableValue,
	NULL,
};

omciAttriDescript_t omciAttriDescriptListOmci[]={
{0, "meId" , 				2, 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0xFFFF,		0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "ME type table",		0, 	ATTR_ACCESS_R, 	ATTR_FORMAT_TABLE, 				0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &METypeTableFunc},
{2, "Message type table",	0, 	ATTR_ACCESS_R, 	ATTR_FORMAT_TABLE, 				0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &MessageTypeTableFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};



/*******************************************************************************************************************************
9.12.9 Managed entity

********************************************************************************************************************************/
/*me code points define*/
static uint16 managedEntityAccess[] = {OMCI_ME_ACCESS_ONU, OMCI_ME_ACCESS_OLT, OMCI_ME_ACCESS_OLT_ONU,CODE_POINT_END};
static uint16 managedEntitySupport[] = {OMCI_ME_SUPPORT, OMCI_ME_UNSUPPORT, OMCI_ME_PARTIAL_SUPPORT, OMCI_ME_IGNORED, CODE_POINT_END};


static AttributeFunc AttributeTableFunc = {
	getAttributeTableValue,
	NULL,
};
static AttributeFunc AlarmsTableFunc = {
	getAlarmsTableValue,
	NULL,
};
static AttributeFunc AVCsTableFunc = {
	getAVCsTableValue,
	NULL,
};

static AttributeFunc InstancesTableFunc = {
	getInstancesTableValue,
	NULL,
};


omciAttriDescript_t omciAttriDescriptListManagedEntity[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Name" , 			25 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Attributes table" , 	0 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_TABLE, 			0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &AttributeTableFunc},
{3, "Access", 			1,	ATTR_ACCESS_R, 	ATTR_FORMAT_ENUM, 			0, 	0, 			0, 			managedEntityAccess, 	ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{4, "Alarms table", 	0, 	ATTR_ACCESS_R, 	ATTR_FORMAT_TABLE, 			0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &AlarmsTableFunc},
{5, "AVCs table", 		0, 	ATTR_ACCESS_R, 	ATTR_FORMAT_TABLE, 			0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &AVCsTableFunc},
{6, "Actions", 			4,	ATTR_ACCESS_R, 	ATTR_FORMAT_BIT_FIELD, 		0, 	0, 			0xFFFFFFFF, 	NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{7, "Instances table", 	0, 	ATTR_ACCESS_R, 	ATTR_FORMAT_TABLE, 			0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &InstancesTableFunc},
{8, "Support", 		1,	ATTR_ACCESS_R, 	ATTR_FORMAT_ENUM, 			0, 	0, 			0, 			managedEntitySupport, 	ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};





/*******************************************************************************************************************************
9.12.10 Attribute
This managed entity describes a particular attribute type that is supported by the ONU. This ME is 
not included in MIB upload. 
Relationships 
	One or more attribute entities are related to each ME entity. More than one ME entity can 
	refer to a given attribute entity. 
********************************************************************************************************************************/

/*me code points define*/
static uint16 Attribute_Access[] = {ATTR_ACCESS_R, ATTR_ACCESS_W, ATTR_ACCESS_R_W, ATTR_ACCESS_R_SET_CREATE, ATTR_ACCESS_W_SET_CREATE, ATTR_ACCESS_R_W_SET_CREATE, CODE_POINT_END};
static uint16 Attribute_Format[] = {ATTR_FORMAT_POINTER, ATTR_FORMAT_BIT_FIELD, ATTR_FORMAT_SIGNED_INT, ATTR_FORMAT_UNSIGNED_INT, ATTR_FORMAT_STRING, ATTR_FORMAT_ENUM, ATTR_FORMAT_TABLE,CODE_POINT_END};
static AttributeFunc CodePointsTableFunc = {
	getCodePointsTableValue,
	NULL,
};
omciAttriDescript_t omciAttriDescriptListAttribute[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFF,		0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Name" , 			25 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "Size" , 			4, 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "Access", 			1,	ATTR_ACCESS_R, 	ATTR_FORMAT_ENUM, 			0, 	0, 			0, 			Attribute_Access, 		ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{4, "Format", 			1,	ATTR_ACCESS_R, 	ATTR_FORMAT_ENUM, 			0, 	0, 			0, 			Attribute_Format, 		ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{5, "Lower limit" , 		4, 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{6, "Upper limit" , 		4, 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{7, "Bit field", 			4,	ATTR_ACCESS_R, 	ATTR_FORMAT_BIT_FIELD, 		0, 	0, 			0xFFFFFFFF, 	NULL, 					ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{8, "Code points table", 0, 	ATTR_ACCESS_R, 	ATTR_FORMAT_TABLE, 			0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &CodePointsTableFunc},
{9, "Support", 		1,	ATTR_ACCESS_R, 	ATTR_FORMAT_ENUM, 			0, 	0, 			0, 			managedEntitySupport, 	ATTR_FULLY_SUPPORT, 	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


/*******************************************************************************************************************************
9.12.11 Octet string 

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListOctetString[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	1,	0xFFFE,		0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Length" , 			2 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,	0,	375,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Part 1" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Part 2" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "Part 3" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{5, "Part 4" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{6, "Part 5" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{7, "Part 6" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{8, "Part 7" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{9, "Part 8" , 			25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{10, "Part 9" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{11, "Part 10" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{12, "Part 11" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{13, "Part 12" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{14, "Part 13" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{15, "Part 14" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{16, "Part 15" , 		25 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,		0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};
/*******************************************************************************************************************************
9.12.12 General purpose buffer 

********************************************************************************************************************************/
static AttributeFunc MaximumSizeFunc = {
	getGeneralValue,
	setMaximumSizeValue,
};
static AttributeFunc GeneralBufferTableFunc = {
	getGeneralBufferTableValue,
	NULL,
};



omciAttriDescript_t omciAttriDescriptListGeneralBuffer[]={
{0, "meId" , 			2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Maximum size" , 	4 , 	ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &MaximumSizeFunc},
{2, "Buffer Table" , 	0 , 	ATTR_ACCESS_R, 					ATTR_FORMAT_STRING,				0,	0,			0,			NULL,					ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_BUFFER_TABLE, &GeneralBufferTableFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.12.13 File transfer controller 

********************************************************************************************************************************/
/*me code points define*/
static uint16 fileTransferTrigger[] = {1, 2, 3, 4, CODE_POINT_END};
static uint16 fileTransferStatus[] = {0, 1, 2, 3, 5, 6, CODE_POINT_END};
static AttributeFunc FileTypeFunc = {
	getGeneralValue,
	setFileTypeValue,
};
static AttributeFunc FileInstanceFunc = {
	getGeneralValue,
	setFileInstanceValue,
};
static AttributeFunc LocalFileNameFunc = {
	getGeneralValue,
	setLocalFileNameValue,
};
static AttributeFunc NetworkAddressFunc = {
	getGeneralValue,
	setNetworkAddressValue,
};
static AttributeFunc FileTransferTriggerFunc = {
	getGeneralValue,
	setFileTransferTriggerValue,
};


omciAttriDescript_t omciAttriDescriptListFileTransController[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 			ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,			NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Transfer protocols",	2 , 	ATTR_ACCESS_R, 			ATTR_FORMAT_BIT_FIELD,		0,	0,			0xFFFFFFFF,	NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "File type" , 			2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &FileTypeFunc},
{3, "File instance" , 		2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &FileInstanceFunc},
{4, "Local file name" , 		2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_POINTER,		0,	0x0000FFFF,	0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &LocalFileNameFunc},
{5, "Network address" , 	2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_POINTER,		0,	0x0000FFFF,	0,			NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &NetworkAddressFunc},
{6, "File transfer trigger" , 	1 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_ENUM,			0,	0,			0,			fileTransferTrigger,	ATTR_FULLY_SUPPORT,	NO_AVC, &FileTransferTriggerFunc},
{7, "File transfer status" , 	1 , 	ATTR_ACCESS_R, 			ATTR_FORMAT_ENUM,			0,	0,			0,			fileTransferStatus,		ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_FILE_TRANSFER_STATUS, &generalGetFunc},
{8, "GEM IWTP pointer" , 	2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL,					ATTR_UNSUPPORT,		NO_AVC, &generalGetSetFunc},
{9, "VLAN" , 				2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,			NULL,					ATTR_UNSUPPORT,		NO_AVC, &generalGetSetFunc},
{10, "File size" , 			4 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,			NULL,					ATTR_UNSUPPORT,		NO_AVC, &generalGetSetFunc},
{11, "Directory listing" , 	0 , 	ATTR_ACCESS_R, 			ATTR_FORMAT_TABLE,			0,	0,			0,			NULL,					ATTR_UNSUPPORT,		NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
9.12.14 Generic status portal 

********************************************************************************************************************************/
static AttributeFunc StatusDocumentFunc = {
	getStatusDocumentValue,
	NULL,
};
static AttributeFunc ConfugurationdocumentFunc = {
	getConfugurationdocumentValue,
	NULL,
};
static AttributeFunc AVCReportRateFunc = {
	getGeneralValue,
	setAVCReportRateValue,
};

omciAttriDescript_t omciAttriDescriptListGenericStatus[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,			0,		NULL, 	ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Status document" , 	0, 	ATTR_ACCESS_R, 					ATTR_FORMAT_TABLE,			0,	0,			0,		NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_STATUS_DOCUMENT, &StatusDocumentFunc},
{2, "Configuration document",0 , 	ATTR_ACCESS_R, 					ATTR_FORMAT_TABLE,			0,	0,			0,		NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_CONFIGURATION_DOCUMENT, &ConfugurationdocumentFunc},
{3, "AVC report rate" , 		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,		NULL,	ATTR_FULLY_SUPPORT,	NO_AVC, &AVCReportRateFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


#ifdef TCSUPPORT_SNMP
/*******************************************************************************************************************************
9.12.15 SNMP config data 

********************************************************************************************************************************/
static AttributeFunc snmpVersionGetSetFunc = {
	getGeneralValue,
	setSnmpVersionValue,
};
static AttributeFunc snmpAgentIPGetSetFunc = {
	getGeneralValue,
	setSnmpAgentIpValue,
};
static AttributeFunc snmpServerIPGetSetFunc = {
	getGeneralValue,
	setSnmpServerIpValue,
};
static AttributeFunc snmpServerPortGetSetFunc = {
	getGeneralValue,
	setSnmpServerPortValue,
};
static AttributeFunc snmpReadCommunityGetSetFunc = {
	getGeneralValue,
	setSnmpReadCommunityValue,
};
static AttributeFunc snmpWriteCommunityGetSetFunc = {
	getGeneralValue,
	setSnmpWriteCommunityValue,
};
static AttributeFunc snmpSysNameGetSetFunc = {
	getGeneralValue,
	setSnmpSysNameValue,
};

omciAttriDescript_t omciAttriDescriptListSnmpConfigData[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "SNMP version" , 		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &snmpVersionGetSetFunc},
{2, "SNMP agent address",	2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,		0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &snmpAgentIPGetSetFunc},
{3, "SNMP server address",	4 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,		0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &snmpServerIPGetSetFunc},
{4, "SNMP server port",		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &snmpServerPortGetSetFunc},
{5, "Security name pointer",		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,	0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{6, "Community for read",		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,	0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &snmpReadCommunityGetSetFunc},
{7, "Community for write",		2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,	0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &snmpWriteCommunityGetSetFunc},
{8, "Sys name pointer",			2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,	0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &snmpSysNameGetSetFunc},
{255, NULL , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};
#endif

#ifdef TCSUPPORT_CWMP
/*******************************************************************************************************************************
9.12.16 TR-069 management server 

********************************************************************************************************************************/
static AttributeFunc tr069ManageAdminGetSetFunc = {
	getTr069ManageAdmin,
	setTr069ManageAdmin,
};

static AttributeFunc tr069ACSAddrGetSetFunc = {
	getGeneralValue,
	setTr069AcsAddr,
};

static AttributeFunc tr069AssociatedTagGetSetFunc = {
	getGeneralValue,
#if  defined(TCSUPPORT_PON_IP_HOST) && defined(TCSUPPORT_CT)
	setTr069AssociateTag,
#else
	NULL,
#endif
};

omciAttriDescript_t omciAttriDescriptListTr069ManageServer[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 			ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Administrative" , 		1 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &tr069ManageAdminGetSetFunc},
{2, "ACS network address",	2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_POINTER,		0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &tr069ACSAddrGetSetFunc},
{3, "Associated tag",		2 , 	ATTR_ACCESS_R_W, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL,					ATTR_PART_SUPPORT,	NO_AVC, &tr069AssociatedTagGetSetFunc},
{255, NULL , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};
#endif

#ifdef TCSUPPORT_CUC
static AttributeFunc CucOnuCapabilityOperatorIDGet = {
	getCucOnuCapOperatorID,
	NULL
};

static AttributeFunc CucOnuCapabilityCucSpecVerGet = {
	getCucOnuCapSpecVer,
	NULL
};

static AttributeFunc CucOnuCapabilityOnuTypeGet = {
	getCucOnuCapOnuType,
	NULL
};

static AttributeFunc CucOnuCapabilityTxPowerCtrlGet = {
	getCucOnuCapOnuTxPowerCtrl,
	NULL
};

omciAttriDescript_t omciAttriDescriptListCucOnuCapability[]={
{0, "meId",				    2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0x1,	0xFFFE,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,								&generalGetFunc},
{1, "Operator ID",			4,	ATTR_ACCESS_R,	    ATTR_FORMAT_STRING,			0,	0,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,									&CucOnuCapabilityOperatorIDGet},
{2, "CUC Spec Version",		1,	ATTR_ACCESS_R,		ATTR_FORMAT_ENUM,			0,	1,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,		&CucOnuCapabilityCucSpecVerGet},
{3, "ONU Type",		        1,	ATTR_ACCESS_R,	    ATTR_FORMAT_ENUM,		    0,	5,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,									&CucOnuCapabilityOnuTypeGet},
{4, "ONU Tx power supply control",		1,	ATTR_ACCESS_R,	ATTR_FORMAT_ENUM,   0,	2,		0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,									&CucOnuCapabilityTxPowerCtrlGet},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};
#endif

/*******************************************************************************************************************************
9.12.17 Threshold data 64 bit

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListThresholdData64Bit[]={
{0, "meId" , 			2, 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,		NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "Threshold value 1",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{2, "Threshold value 2",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{3, "Threshold value 3",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{4, "Threshold value 4",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{5, "Threshold value 5",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{6, "Threshold value 6",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{7, "Threshold value 7",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{8, "Threshold value 8",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{9, "Threshold value 9",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{10,"Threshold value 10",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{11,"Threshold value 11",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{12,"Threshold value 12",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{13,"Threshold value 13",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{14,"Threshold value 14",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{15,"Threshold value 15",	8, 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFFFFFFFF,	0,	NULL, ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


