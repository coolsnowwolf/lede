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
	omci_me_vlan_nodetable.c

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
#include "omci_me_vlan.h"

/*******************************************************************************************************************************
9.3.10 802.1p mapper service profile

********************************************************************************************************************************/
AttributeFunc dot1pTPPointerFunc = {
	getGeneralValue,
	setdot1pTPPointerValue
};
AttributeFunc dot1pTPPbit0Func = {
	getGeneralValue,
	setdot1pTPPbit0Value
};
AttributeFunc dot1pTPPbit1Func = {
	getGeneralValue,
	setdot1pTPPbit1Value
};
 AttributeFunc dot1pTPPbit2Func = {
	getGeneralValue,
	setdot1pTPPbit2Value
};
AttributeFunc dot1pTPPbit3Func = {
	getGeneralValue,
	setdot1pTPPbit3Value
};

AttributeFunc dot1pTPPbit4Func = {
	getGeneralValue,
	setdot1pTPPbit4Value
};
AttributeFunc dot1pTPPbit5Func = {
	getGeneralValue,
	setdot1pTPPbit5Value
};
AttributeFunc dot1pTPPbit6Func = {
	getGeneralValue,
	setdot1pTPPbit6Value
};
AttributeFunc dot1pTPPbit7Func = {
	getGeneralValue,
	setdot1pTPPbit7Value
};
AttributeFunc dot1pUnmarkOptionFunc = {
	getGeneralValue,
	setdot1pUnmarkOptionValue
};
AttributeFunc dot1pDscpPbitMappFunc = {
	getGeneralValue,
	setdot1pDscpPbitMappValue
};
AttributeFunc dot1pDefaultPbitFunc = {
	getGeneralValue,
	setdot1pDefaultPbitValue
};
AttributeFunc dot1pTPTypeFunc = {
	getGeneralValue,
	setdot1pTPTypeValue
};
		   
omciAttriDescript_t omciAttriDescriptList8021pMapperProfile[]={
	{0,	"Managed entity id",		2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1, 	"TP pointer" , 				2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPointerFunc},
	{2, 	"TP pointer for P-bit 0",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPbit0Func},
	{3, 	"TP pointer for P-bit 1",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPbit1Func},
	{4, 	"TP pointer for P-bit 2",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPbit2Func},
	{5, 	"TP pointer for P-bit 3",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPbit3Func},
	{6, 	"TP pointer for P-bit 4",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPbit4Func},
	{7, 	"TP pointer for P-bit 5",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPbit5Func},
	{8, 	"TP pointer for P-bit 6",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPbit6Func},
	{9, 	"TP pointer for P-bit 7",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pTPPbit7Func},
	{10,	"Unmarked frame option",	1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pUnmarkOptionFunc},
	{11, "DSCP to P bit mapping",	24, 	ATTR_ACCESS_R_W,				ATTR_FORMAT_STRING,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pDscpPbitMappFunc},
	{12, "Default P bit assumption",	1 , 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &dot1pDefaultPbitFunc},
	{13, "TP type", 				1 , 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &dot1pTPTypeFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.3.11 vlan tagging filter data

********************************************************************************************************************************/
AttributeFunc vlanFilterVlanFilterListFunc = {
	getGeneralValue,
	setvlanFilterVlanFilterListValue
};
AttributeFunc vlanFilterForwardOPFunc = {
	getGeneralValue,
	setvlanFilterForwardOPValue
};
AttributeFunc vlanFilterValidNumberEntryFunc = {
	getGeneralValue,
	setvlanFilterValidNumberEntryValue
};
omciAttriDescript_t omciAttriDescriptListVlanTagFilterData[]={
	{0,	"Managed entity id",		2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1, 	"VLAN filter list",			24,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &vlanFilterVlanFilterListFunc},
	{2, 	"Forward operation",		1, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &vlanFilterForwardOPFunc},
	{3, 	"Number of entries",		1, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &vlanFilterValidNumberEntryFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.3.12 vlan tagging operation configuration data

********************************************************************************************************************************/
AttributeFunc vlanTagOpUpModeFunc = {
	getGeneralValue,
	setVlanTagOpUpModeValue
};
AttributeFunc vlanTagOpTCIFunc = {
	getGeneralValue,
	setVlanTagOpTCIValue
};
AttributeFunc vlanTagOpDownModeFunc = {
	getGeneralValue,
	setVlanTagOpDownModeValue
};
AttributeFunc vlanTagOpAssociaTypeFunc = {
	getGeneralValue,
	setVlanTagOpAssociaTypeValue
};

AttributeFunc vlanTagOpAssociaPointerFunc = {
	getGeneralValue,
	setVlanTagOpAssociaPointerValue
};
omciAttriDescript_t omciAttriDescriptListVlanTagOperation[]={
	{0,	"Managed entity id",				2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1, 	"Upsream tag operation mode",		1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &vlanTagOpUpModeFunc},
	{2, 	"Upstream TCI Value",				2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &vlanTagOpTCIFunc},
	{3, 	"Downstream tag operation mode",	1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &vlanTagOpDownModeFunc},
	{4, 	"Association type",					1, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &vlanTagOpAssociaTypeFunc},
	{5, 	"Associated ME pointer",				2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &vlanTagOpAssociaPointerFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.3.13 extended vlan tagging operation configuration data

********************************************************************************************************************************/
AttributeFunc extVlanTagOpAssociaTypeFunc = {
	getGeneralValue,
	setExtVlanTagOpAssociaTypeValue
};
AttributeFunc extVlanTagOpMaxNumFunc = {
	getExtVlanTagOpMaxNumValue,
	NULL
};
AttributeFunc extVlanTagOpInputTPIDFunc = {
	getGeneralValue,
	setExtVlanTagOpInputTPIDValue
};
AttributeFunc extVlanTagOpOutputTPIDFunc = {
	getGeneralValue,
	setExtVlanTagOpOutputTPIDValue
};
AttributeFunc extVlanTagOpDownModeFunc = {
	getGeneralValue,
	setExtVlanTagOpDownModeValue
};
AttributeFunc extVlanTagOpTblFunc = {
	getExtVlanTagOpTblValue,
	setExtVlanTagOpTblValue
};
AttributeFunc extVlanTagOpAssociaPointerFunc = {
	getGeneralValue,
	setExtVlanTagOpAssociaPointerValue
};
AttributeFunc extVlanTagOpDSCPMappingFunc = {
	getGeneralValue,
	setExtVlanTagOpDSCPMappingValue
};
omciAttriDescript_t omciAttriDescriptListVlanTagExtendOperation[]={
	{0,	"Managed entity id",		2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
	{1, 	"Association type",			1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &extVlanTagOpAssociaTypeFunc},
	{2, 	"VLAN tag table Max size",	2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &extVlanTagOpMaxNumFunc},
	{3, 	"Input TPID",				2,	ATTR_ACCESS_R_W,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_PART_SUPPORT,  NO_AVC, &extVlanTagOpInputTPIDFunc},
	{4, 	"Output TPID",			2, 	ATTR_ACCESS_R_W,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &extVlanTagOpOutputTPIDFunc},
	{5, 	"Downstream mode",		1, 	ATTR_ACCESS_R_W,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &extVlanTagOpDownModeFunc},
	{6, 	"VLAN tag operation table",	16, 	ATTR_ACCESS_R_W,				ATTR_FORMAT_TABLE,			0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &extVlanTagOpTblFunc},
	{7, 	"Associated ME pointer",		2, 	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &extVlanTagOpAssociaPointerFunc},
	{8, 	"DSCP to P bit mapping",	24, 	ATTR_ACCESS_R_W,				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL, ATTR_FULLY_SUPPORT, NO_AVC, &extVlanTagOpDSCPMappingFunc},
	{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


