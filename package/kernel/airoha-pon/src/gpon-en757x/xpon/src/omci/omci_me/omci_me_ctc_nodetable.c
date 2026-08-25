#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_ctc.h"
/*******************************************************************************************************************************
CTC ONU capability

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListOnuCapability[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL, 					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Operator ID" , 		4 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_STRING,			0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{2, "CTC Spec Version",	1 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{3, "ONU type",			1 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,					ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{4, "ONU Tx power supply control",		1 , 	ATTR_ACCESS_R, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};

/*******************************************************************************************************************************
CTC LOID authentication

********************************************************************************************************************************/
static AttributeFunc operatorIDGetFunc = {
	getOperatorIDValue,
	NULL,
};

static AttributeFunc ctcLOIDGetFunc = {
	getCtcLOIDValue,
	NULL,
};

static AttributeFunc ctcPasswordGetFunc = {
	getCtcPasswordValue,
	NULL,
};


static AttributeFunc authStatusGetSetFunc = {
	getGeneralValue,
	setAuthStatusValue,
};


omciAttriDescript_t omciAttriDescriptListLoidAuthentication[]={
{0, "meId" , 				2 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL, 		ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetFunc},
{1, "Operator ID" , 		4 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &operatorIDGetFunc},
{2, "LOID",				24 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &ctcLOIDGetFunc},
{3, "Password",			12 , 	ATTR_ACCESS_R, 		ATTR_FORMAT_STRING,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &ctcPasswordGetFunc},
{4, "Authentication status",	1 , 	ATTR_ACCESS_R_W, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &authStatusGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};


/*******************************************************************************************************************************
CTC Extended multicast operations profiles

********************************************************************************************************************************/
static AttributeFunc EMOPMulticastVersionGetSetFunc = {
	getGeneralValue,
	setMOPMulticastVersionValue,
};

static AttributeFunc EMOPMulticastFuncGetSetFunc = {
	getGeneralValue,
	setMOPMulticastFuncValue,
};

static AttributeFunc EMOPMulticastFastLeaveGetSetFunc = {
	getGeneralValue,
	setMOPMulticastFastLeaveValue,
};

static AttributeFunc EMOPMulticastUpstreamTCIGetSetFunc = {
	getGeneralValue,
	setMOPMulticastUpstreamTCIValue,
};

static AttributeFunc EMOPMulticastTagCtrlGetSetFunc = {
	getGeneralValue,
	setMOPMulticastTagCtrlValue,
};

static AttributeFunc EMOPMulticastUpstreamRateGetSetFunc = {
	getGeneralValue,
	setMOPMulticastUpstreamRateValue,
};

static AttributeFunc EMOPMulticastDynamicAclTableGetSetFunc = {
	getMOPMulticastDynamicAclTableValue,
	setEMOPMulticastDynamicAclTableValue,
};

static AttributeFunc EMOPMulticastStaticAclTableGetSetFunc = {
	getMOPMulticastStaticAclTableValue,
	setEMOPMulticastStaticAclTableValue,
}; 

static AttributeFunc EMOPMulticastRobustnessGetSetFunc = {
	getGeneralValue,
	setMOPMulticastRobustnessValue,
}; 

static AttributeFunc EMOPMulticastQuerierIPAddressGetSetFunc = {
	getGeneralValue,
	setMOPMulticastQuerierIPAddressValue,
}; 

static AttributeFunc EMOPMulticastQueryIntervalGetSetFunc = {
	getGeneralValue,
	setMOPMulticastQueryIntervalValue,
}; 

static AttributeFunc EMOPMulticastQueryMaxResponseTimeGetSetFunc = {
	getGeneralValue,
	setMOPMulticastQueryMaxResponseTimeValue,
}; 

static AttributeFunc EMOPMulticastLastMemberQueryIntervalGetSetFunc = {
	getGeneralValue,
	setMOPMulticastLastMemberQueryIntervalValue,
}; 

static AttributeFunc EMOPMulticastUnauthorizedJoinRequestGetSetFunc = {
	getGeneralValue,
	setMOPMulticastUnauthorizedJoinRequestValue,
}; 

static AttributeFunc EMOPMulticastDownstreamIGMPAndTCIGetSetFunc = {
	getGeneralValue,
	setMOPMulticastDownstreamIGMPAndTCIValue,
};

omciAttriDescript_t omciAttriDescriptListExtMulticastProfile[]={
{0, "meId" , 						2 , 	ATTR_ACCESS_R_SET_CREATE, 		ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL, 		ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{1, "IGMP Version" , 				1 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	 	0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastVersionGetSetFunc},
{2, "IGMP function",				1 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastFuncGetSetFunc},
{3, "Immediate leave",				1 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastFastLeaveGetSetFunc},
{4, "Upstream IGMP TCI",			2 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastUpstreamTCIGetSetFunc},
{5, "Upstream IGMP tag control",		1 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastTagCtrlGetSetFunc},
{6, "Upstream IGMP rate",			4 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastUpstreamRateGetSetFunc},
{7, "Dynamic ACL table",			30 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_TABLE,				0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastDynamicAclTableGetSetFunc},
{8, "Static ACL table",				30 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_TABLE,				0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastStaticAclTableGetSetFunc},
{9, "Lost groups list table",			16 , 	ATTR_ACCESS_R, 					ATTR_FORMAT_TABLE,				0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &generalGetSetFunc},
{10, "Robustness",					1 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastRobustnessGetSetFunc},
{11, "Querier IP address",			16 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,			0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastQuerierIPAddressGetSetFunc},
{12, "Querier interval",				4 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastQueryIntervalGetSetFunc},
{13, "Querier max response time",	4 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastQueryMaxResponseTimeGetSetFunc},
{14, "Last member query interval",	4 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastLastMemberQueryIntervalGetSetFunc},
{15, "Unauthorized join request",		1 , 	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastUnauthorizedJoinRequestGetSetFunc},
{16, "Downstream IGMP TCI",		3 , 	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,		0,	0,	0,		NULL,		ATTR_FULLY_SUPPORT,	NO_AVC, &EMOPMulticastDownstreamIGMPAndTCIGetSetFunc},
{255, "" , 0 , 0,  0, 0,0, 0, NULL , 0, 0 ,NULL}
};
