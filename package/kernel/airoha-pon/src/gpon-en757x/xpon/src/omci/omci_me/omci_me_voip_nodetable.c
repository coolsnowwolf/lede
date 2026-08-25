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
	omci_me_voip_nodetable.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2013/9/26	Create
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_voip.h"


/*******************************************************************************************************************************
9.9.2 sip user data

********************************************************************************************************************************/
alarm_id_map_entry_t alarmIdTableSipUserData[] = {
	{OMCI_ALARM_ID_SIP_USER_DATA_SIPUA_REG_AUTH},
	{OMCI_ALARM_ID_SIP_USER_DATA_SIPUA_REG_TIMEOUT},
	{OMCI_ALARM_ID_SIP_USER_DATA_SIPUA_REG_FAIL},
	{0}
};

static AttributeFunc sipUserDatauserpartaorFunc = {
	getGeneralValue,
	setSipUserDataUserPartAOR
};

static AttributeFunc sipUserDataDispNameFunc = {
	getSipUserDataDispName,
	setSipUserDataDispName
};

static AttributeFunc sipUserDataUserPassFunc = {
	getGeneralValue,
	setSipUserDataUserPass
};
static AttributeFunc sipUserDataSipURIFunc = {
	getGeneralValue,
	setSipUserDataSipURI
};

static AttributeFunc sipUserDataExpTimeFunc = {
	getSipUserDataExpTime,
	setSipUserDataExpTime
};

static AttributeFunc sipUserDataPPTPPointerFunc = {
	getGeneralValue,
	setSipUserDataPPTPPointer
};

omciAttriDescript_t omciAttriDescriptListSipUserData[]={
{0, "meId",							2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "SIP agent pointer",			2,	ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{2, "User part AOR",				2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &sipUserDatauserpartaorFunc},
{3, "SIP display name",				25,	ATTR_ACCESS_R_W, 				ATTR_FORMAT_STRING,			0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &sipUserDataDispNameFunc},
{4, "Username,password",			2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &sipUserDataUserPassFunc},
{5, "voicemail server SIP URI",		2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &sipUserDataSipURIFunc},
{6, "voicemail subscri exp time",	4,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &sipUserDataExpTimeFunc},
{7, "network dial plan pointer", 	2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{8, "App serv profile pointer", 	2, ATTR_ACCESS_R_W_SET_CREATE, 		ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{9, "feature code pointer", 		2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{10, "PPTP pointer", 				2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &sipUserDataPPTPPointerFunc},
{11, "Release timer", 				1, ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{12, "ROH timer", 					1, ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.3 sip config data

********************************************************************************************************************************/
alarm_id_map_entry_t alarmIdTableSIPAgentConfigData[] = {
    {OMCI_ALARM_ID_SIP_AGENT_CFG_DATA_SIPUA_REG_NAME},
    {OMCI_ALARM_ID_SIP_AGENT_CFG_DATA_SIPUA_REG_REACH},
    {OMCI_ALARM_ID_SIP_AGENT_CFG_DATA_SIPUA_REG_CONNECT},
    {OMCI_ALARM_ID_SIP_AGENT_CFG_DATA_SIPUA_REG_VALID},    
    {0}
};

static AttributeFunc sipAgentCfgDataProxyAddrFunc = {
	getGeneralValue,
	Set_SipAgentCfgData_ProxyAddr
};

static AttributeFunc sipAgentCfgDataOutBoundProxyAddrFunc = {
	getGeneralValue,
	Set_SipAgentCfgData_OutBoundProxyAddr
};

static AttributeFunc sipAgentCfgDataSipRegExpTimeFunc = {
	Get_SipAgentCfgData_SipRegExpTime,
	Set_SipAgentCfgData_SipRegExpTime
};

static AttributeFunc sipAgentCfgDataSIPReregTimeFunc = {
    Get_SipAgentCfgData_SIPReregTime,
	Set_SipAgentCfgData_SIPReregTime
};

static AttributeFunc sipAgentCfgDataSIPStatusFunc = {
	Get_SipAgentCfgData_SIPStatus,
	NULL
};

static AttributeFunc sipAgentCfgDataSIPRegistrarFunc = {
	getGeneralValue,
	Set_SipAgentCfgData_SIPRegistrar
};

static AttributeFunc sipAgentCfgDataTcpUdpFunc = {
	getGeneralValue,
	Set_SipAgentCfgData_TcpUdp,
};

omciAttriDescript_t    omciAttriDescriptListSIPAgentConfigData[]={
{0, "meID",                             2, ATTR_ACCESS_R_SET_CREATE,    ATTR_FORMAT_UNSIGNED_INT,   0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc },
{1, "Proxy server address pointer",     2, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_POINTER,        0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &sipAgentCfgDataProxyAddrFunc },
{2, "Outbound proxy address pointer",   2, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_POINTER,        0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &sipAgentCfgDataOutBoundProxyAddrFunc },
{3, "Primary SIP DNS",                  4, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_UNSIGNED_INT,   0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc },
{4, "Secondary SIP DNS",                4, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_UNSIGNED_INT,   0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc },
{5, "TCP/UDP pointer",                  2, ATTR_ACCESS_R_W,             ATTR_FORMAT_POINTER,        0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &sipAgentCfgDataTcpUdpFunc },
{6, "SIP reg exp time",                 4, ATTR_ACCESS_R_W,             ATTR_FORMAT_UNSIGNED_INT,   0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &sipAgentCfgDataSipRegExpTimeFunc },
{7, "SIP rereg head start time",        4, ATTR_ACCESS_R_W,             ATTR_FORMAT_UNSIGNED_INT,   0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &sipAgentCfgDataSIPReregTimeFunc },
{8, "Host part URI",                    2, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_POINTER,        0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc },
{9, "SIP status",                       1, ATTR_ACCESS_R,               ATTR_FORMAT_ENUM,           0, 0, 0, NULL, ATTR_FULLY_SUPPORT, OMCI_AVC_ID_SIP_AGENT_CFG_DATA_SIP_STATUS, &sipAgentCfgDataSIPStatusFunc },
{10, "SIP registrar",                   2, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_POINTER,        0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &sipAgentCfgDataSIPRegistrarFunc },
{11, "Softswitch",                      4, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_STRING,         0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc },
{12, "SIP response table",				5 ,ATTR_ACCESS_R_W,				ATTR_FORMAT_TABLE,		 	0, 0, 0, NULL, ATTR_UNSUPPORT, 	   NO_AVC, &generalGetSetFunc },
{13, "SIP option transmit control",     1, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_ENUM,           0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc },
{14, "SIP URI format",                  1, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_ENUM,           0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc },
{15, "Redundant SIP agent pointer",     2, ATTR_ACCESS_R_W_SET_CREATE,  ATTR_FORMAT_POINTER,        0, 0, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc },
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.4  VoIP voice CTP

********************************************************************************************************************************/

omciAttriDescript_t omciAttriDescriptListVoipVoiceCTP[]={
{0, "meId", 						2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{1, "User protocol pointer", 		2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{2, "PPTP pointer",		2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{3, "VoIP media profile pointer",		2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "Signalling code",	1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.9.5  VoIP media profile

********************************************************************************************************************************/
static AttributeFunc voipMediaProFaxModeFunc = {
	getVOIPMediaProFaxMode,
	setVOIPMediaProFaxMode
};
static AttributeFunc voipMediaProCodecSele1stFunc = {
	getVOIPMediaProCodecSele1st,
	setVOIPMediaProCodecSele1st
};
static AttributeFunc voipMediaProPktPeriodSele1stFunc = {
	getVOIPMediaProPktPeriodSele1st,
	setVOIPMediaProPktPeriodSele1st
};

static AttributeFunc voipMediaProSlience1stFunc = {
	getVOIPMediaProSlience1st,
	setVOIPMediaProSlience1st
};
static AttributeFunc voipMediaProCodecSele2ndFunc = {
	getVOIPMediaProCodecSele2nd,
	setVOIPMediaProCodecSele2nd
};
static AttributeFunc voipMediaProPktPeriodSele2ndFunc = {
	getVOIPMediaProPktPeriodSele2nd,
	setVOIPMediaProPktPeriodSele2nd
};
static AttributeFunc voipMediaProSlience2ndFunc = {
	getVOIPMediaProSlience2nd,
	setVOIPMediaProSlience2nd
};

static AttributeFunc voipMediaProCodecSele3rdFunc = {
	getVOIPMediaProCodecSele3rd,
	setVOIPMediaProCodecSele3rd
};
static AttributeFunc voipMediaProPktPeriodSele3rdFunc = {
	getVOIPMediaProPktPeriodSele3rd,
	setVOIPMediaProPktPeriodSele3rd
};
static AttributeFunc voipMediaProSlience3rdFunc = {
	getVOIPMediaProSlience3rd,
	setVOIPMediaProSlience3rd
};

static AttributeFunc voipMediaProCodecSele4thFunc = {
	getVOIPMediaProCodecSele4th,
	setVOIPMediaProCodecSele4th
};
static AttributeFunc voipMediaProPktPeriodSele4thFunc = {
	getVOIPMediaProPktPeriodSele4th,
	setVOIPMediaProPktPeriodSele4th
};
static AttributeFunc voipMediaProSlience4thFunc = {
	getVOIPMediaProSlience4th,
	setVOIPMediaProSlience4th
};

static AttributeFunc voipMediaProOOBDtmfFunc = {
	getVOIPMediaProOOBDtmf,
	setVOIPMediaProOOBDtmf
};

omciAttriDescript_t omciAttriDescriptListVoipMediaPro[]={
{0, "meId", 						2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{1, "Fax mode",						1,	ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProFaxModeFunc},
{2, "Voice serv profile pointer", 	2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{3, "Codec select-1st order",		1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProCodecSele1stFunc},
{4, "Pkt period select-1st order",	1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProPktPeriodSele1stFunc},
{5, "Silence -1st order",			1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProSlience1stFunc},
{6, "Codec select-2nd order",		1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProCodecSele2ndFunc},
{7, "Pkt period select-2nd order",	1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProPktPeriodSele2ndFunc},
{8, "Silence -2nd order",			1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProSlience2ndFunc},
{9, "Codec select-3rd order",		1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProCodecSele3rdFunc},
{10, "Pkt period select-3rd order",	1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProPktPeriodSele3rdFunc},
{11, "Silence -3rd order",			1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProSlience3rdFunc},
{12, "Codec select-4th order",		1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProCodecSele4thFunc},
{13, "Pkt period select-4th order",	1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProPktPeriodSele4thFunc},
{14, "Silence -4th order",			1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProSlience4thFunc},
{15, "OOB DTMF",					1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipMediaProOOBDtmfFunc},
{16, "RTP profile pointer",			2,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.6: Voice service profile

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListVoiceServiceProfile[]={
{ 0, "meID",                            2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x00000080, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 1, "Announcement type",               1, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x0000FFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 2, "Jitter target",                   2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x0000FFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 3, "Jitter buffer max",               2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 4, "Echo cancel ind",                 1, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 5, "PSTN protocol variant",           2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 6, "DTMF digit levels",               2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 7, "DTMF digit duration",             2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 8, "Hook flash minimum time",         2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 9, "Hook flash maximum time",         2, ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 10, "Tone pattern table",             20, ATTR_ACCESS_R_W,                ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 11, "Tone event table",               7, ATTR_ACCESS_R_W,                 ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 12, "Ringing pattern table",          5, ATTR_ACCESS_R_W,                 ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 13, "Ringing event table",            7, ATTR_ACCESS_R_W,                 ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.9.7: RTP profile data

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListRtpProfileData[]={
{ 0, "meID",                            2, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x00000080, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 1, "Local port min",                  2, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x0000FFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 2, "Local port max",                  2, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x0000FFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 3, "DSCP mark",                       1, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 4, "Piggyback events",                1, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 5, "Tone events",                     1, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 6, "DTMF events",                     1, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 7, "CAS events",                      1, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.9.8: VoIP application service profile

********************************************************************************************************************************/
omciAttriDescript_t omciAttriDescriptListVoIPAppSvcPro[]={
{ 0, "meID",                            2, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x00000080, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 1, "CID feature",                     1, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x00000080, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 2, "Call waiting feature",            1, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x00000080, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 3, "Call prog or trans features",     2, ATTR_ACCESS_R_W_SET_CREATE,	    ATTR_FORMAT_UNSIGNED_INT, 0, 0x00008000, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 4, "Call presentation features",      2, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x00008000, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 5, "Direct connect feature",          1, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_UNSIGNED_INT, 0, 0x00000002, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 6, "Direct connect URI pointer",      2, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_POINTER, 	  0, 0x0000FFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 7, "Brg line agent URI pointer",      2, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_POINTER, 	  0, 0x0000FFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{ 8, "Confence factory URI pointer",    2, ATTR_ACCESS_R_W_SET_CREATE,      ATTR_FORMAT_POINTER, 	  0, 0x0000FFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.9.12: Call Control performance monitoring history data

********************************************************************************************************************************/
static AttributeFunc CallCtrlPMHisDataThresholdFunc = {
    getGeneralValue,
    setCallCtrlPMThresholdData
};
static AttributeFunc CallCtrlPMHisDataStaticsFunc = {
    getPMCommonFunc,
    NULL
};
omciAttriDescript_t omciAttriDescriptListCallCtrlPMHistory[]={
{ 0, "meID",                       2, ATTR_ACCESS_R_SET_CREATE,   ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc },
{ 1, "interval end time",          1, ATTR_ACCESS_R,              ATTR_FORMAT_UNSIGNED_INT, 0, 0x000000FF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &generalGetIntervalEndTimeFunc},
{ 2, "Threshold data 1/2 id",      2, ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT, 0, 0x0000FFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &CallCtrlPMHisDataThresholdFunc},
{ 3, "Call setup failures",        4, ATTR_ACCESS_R,              ATTR_FORMAT_UNSIGNED_INT, 0, 0xFFFFFFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &CallCtrlPMHisDataStaticsFunc},
{ 4, "Call setup timer",           4, ATTR_ACCESS_R,              ATTR_FORMAT_UNSIGNED_INT, 0, 0xFFFFFFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &CallCtrlPMHisDataStaticsFunc},
{ 5, "Call terminate failures",    4, ATTR_ACCESS_R,              ATTR_FORMAT_UNSIGNED_INT, 0, 0xFFFFFFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &CallCtrlPMHisDataStaticsFunc},
{ 6, "Analog port releases",       4, ATTR_ACCESS_R,              ATTR_FORMAT_UNSIGNED_INT, 0, 0xFFFFFFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &CallCtrlPMHisDataStaticsFunc},
{ 7, "Analog port off-hook timer", 4, ATTR_ACCESS_R,              ATTR_FORMAT_UNSIGNED_INT, 0, 0xFFFFFFFF, 0, NULL, ATTR_FULLY_SUPPORT, NO_AVC, &CallCtrlPMHisDataStaticsFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
9.9.13: Rtp performance monitoring history data

********************************************************************************************************************************/
static AttributeFunc RtpPMHistoryDataThresholdDataFunc = {
    getGeneralValue,
    setRTPPMHistoryDataThresholdData
};

static AttributeFunc RtpPMHistoryDataRTPErrorsFunc = {
    getRTPPMHistoryDataRtpErrors,
    NULL
};

static AttributeFunc RtpPMHistoryDataPacketLossFunc = {
    getRTPPMHistoryDataPacketLoss,
    NULL
};

static AttributeFunc RtpPMHistoryDataMaxJitterFunc = {
    getRTPPMHistoryDataMaxJitter,
    NULL
};

static AttributeFunc RtpPMHistoryDataMaxTimeBetRTCPFunc = {
    getRTPPMHistoryDataMaxTimeBetRTCP,
    NULL
};

static AttributeFunc RtpPMHistoryDataNoRTCPFunc = {
    getRTPPMHistoryDataNoRTCPPacket,
    NULL
};

static AttributeFunc RtpPMHistoryDataTimeoutFunc = {
    getRTPPMHistoryDataTimeout,
    NULL
};

static AttributeFunc RtpPMHistoryDataBufferUnderflowsFunc = {
    getRTPPMHistoryDataBufferUnderflows,
    NULL
};

static AttributeFunc RtpPMHistoryDataBufferOverflowsFunc = {
    getRTPPMHistoryDataBufferOverflows,
    NULL
};


omciAttriDescript_t omciAttriDescriptListRtpPMHistoryData[]={
{0, "meId",							2,	ATTR_ACCESS_R_SET_CREATE,	    ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{1, "interval end time",			1,	ATTR_ACCESS_R,	                ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000000F,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",		2,	ATTR_ACCESS_R_W_SET_CREATE,	    ATTR_FORMAT_UNSIGNED_INT,   0,	0x000000FF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&RtpPMHistoryDataThresholdDataFunc},
{3, "rtp errors",           		4,	ATTR_ACCESS_R,	                ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&RtpPMHistoryDataRTPErrorsFunc},
{4, "packet loss",              	4,	ATTR_ACCESS_R,	                ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &RtpPMHistoryDataPacketLossFunc},
{5, "maximum jitter",           	4,	ATTR_ACCESS_R,	                ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &RtpPMHistoryDataMaxJitterFunc},
{6, "maximum time between RTCP", 	4,	ATTR_ACCESS_R,	                ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&RtpPMHistoryDataMaxTimeBetRTCPFunc},
{7, "buffer underflows",            4,  ATTR_ACCESS_R,                  ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &RtpPMHistoryDataBufferUnderflowsFunc},
{8, "buffer overflows",            4,  ATTR_ACCESS_R,                  ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &RtpPMHistoryDataBufferOverflowsFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};



/*******************************************************************************************************************************
9.9.14: SIP agent performance monitoring history data

********************************************************************************************************************************/
static AttributeFunc SipAgentPMHistoryDataThresholdDataFunc = {
    getGeneralValue,
    setSipAgentPMHistoryDataThresholdData
};

static AttributeFunc VoIPPMHistoryDataFunc = {
    getVoIPPMDataCounter,
    NULL
};

omciAttriDescript_t omciAttriDescriptListSipAgentPMHistoryData[]={
{0, "meId",							2,	ATTR_ACCESS_R_SET_CREATE,	    ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{1, "interval end time",			1,	ATTR_ACCESS_R,	                ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000000F,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",		2,	ATTR_ACCESS_R_W_SET_CREATE,	    ATTR_FORMAT_UNSIGNED_INT,   0,	0x000000FF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&SipAgentPMHistoryDataThresholdDataFunc},
{3, "transactions",		            4,	ATTR_ACCESS_R,	    ATTR_FORMAT_UNSIGNED_INT,   0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&VoIPPMHistoryDataFunc},
{4, "rx invite reqs",               4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{5, "rx invite retrans",            4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{6, "rx noninvite reqs",            4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{7, "rx noninvite retrans",         4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{8, "rx response",                  4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{9, "rx response retransmissions",  4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{10, "tx invite reqs",              4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{11, "tx invite retrans",           4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{12, "tx noninvite reqs",           4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{13, "tx noninvite retrans",        4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{14, "tx response",                 4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{15, "tx response retransmissions", 4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},

{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.15: SIP call initation performance monitoring history data

********************************************************************************************************************************/
static AttributeFunc SipCallInitMHistoryDataThresholdDataFunc = {
    getGeneralValue,
    setSipCallInitPMHistoryDataThresholdData
};

omciAttriDescript_t omciAttriDescriptListSipCallInitPMHistoryData[]={
{0, "meId",							    2,	ATTR_ACCESS_R_SET_CREATE,	    ATTR_FORMAT_UNSIGNED_INT,	0,	0x000000FF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{1, "interval end time",			    1,	ATTR_ACCESS_R,	                ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000000F,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",		    2,	ATTR_ACCESS_R_W_SET_CREATE,	    ATTR_FORMAT_UNSIGNED_INT,   0,	0x000000FF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&SipCallInitMHistoryDataThresholdDataFunc},
{3, "failed to connect counter",	    4,	ATTR_ACCESS_R,	    ATTR_FORMAT_UNSIGNED_INT,   0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&VoIPPMHistoryDataFunc},
{4, "failed to validate counter",       4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{5, "timeout counter",                  4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{6, "failure received counter",         4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},
{7, "failed to authenticate counter",   4,  ATTR_ACCESS_R,      ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT, NO_AVC, &VoIPPMHistoryDataFunc},

{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.16 MGC config data

********************************************************************************************************************************/
alarm_id_map_entry_t alarmIdTableMgcCfgData[] = {
	{OMCI_ALARM_ID_MGC_CFG_DATA_TIMEOUT_ALARM},
	{0}
};

static AttributeFunc mgcCfgDataPriMGCFunc = {
	getGeneralValue,
	setMGCCfgDataPriMGC
};
static AttributeFunc mgcCfgDataSecMGCFunc = {
	getGeneralValue,
	setMGCCfgDataSecMGC
};

#if 0
static AttributeFunc mgcCfgDataMsgFormatFunc = {
	getMGCCfgDataMsgFormat,
	setMGCCfgDataMsgFormat
};
static AttributeFunc mgcCfgDataMaxRetryTimeFunc = {
	getMGCCfgDataMaxRetryTime,
	setMGCCfgDataMaxRetryTime
};
#endif

static AttributeFunc mgcCfgDataDevIdFunc = {
	getMGCCfgDataDevId,
	setMGCCfgDataDevId
};

omciAttriDescript_t omciAttriDescriptListMgcCfgData[]={
{0, "meId", 				2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{1, "Primary MGC",			2,	ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &mgcCfgDataPriMGCFunc},
{2, "Secondary MGC",		2,	ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &mgcCfgDataSecMGCFunc},
{3, "TCP-UDP pointer",		2,	ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "Version", 				1,	ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "Message format",		1,	ATTR_ACCESS_R_W_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_PART_SUPPORT,	NO_AVC, &generalGetSetFunc},
{6, "Max retry time",		2,	ATTR_ACCESS_R_W,			 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{7, "Max retry attempts",	2,	ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{8, "service change delay",	2,	ATTR_ACCESS_R_W, 				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{9, "Termination ID base",	25,	ATTR_ACCESS_R_W,				ATTR_FORMAT_STRING,			0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &mgcCfgDataDevIdFunc},
{10, "Softswitch",			4, 	ATTR_ACCESS_R_W_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.18 VoIP config data

********************************************************************************************************************************/
alarm_id_map_entry_t alarmIdTableVoipCfgData[] = {
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_NAME},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_REACH},		
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_CONNECT},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_VALIDATE},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_AUTH},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_TIMEOUT},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SER_FAIL},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_FILE_ERROR},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_NAME},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_REACH},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_CONNECT},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_VALIDATE},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_AUTH},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_TIMEOUT},
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_SUBSCRIPTION_FAIL},	
	{OMCI_ALARM_ID_VOIP_CFG_DATA_VCD_CFG_REBOOT_REQUEST},	
	{0}
};

static AttributeFunc voipCfgDataAvaiProtoFunc = {
	getVOIPCfgDataAvaiProto,
	NULL
};
static AttributeFunc voipCfgDataProtoUsedFunc = {
	getVOIPCfgDataProtoUsed,
	setVOIPCfgDataProtoUsed
};
static AttributeFunc voipCfgDataAvaVoipCfgMethodsFunc = {
	getVOIPCfgDataAvaVoipCfgMethods,
	NULL
};
static AttributeFunc voipCfgDataMethodsUsedFunc = {
	NULL,
	setVOIPCfgDataMethodsUsed
};
static AttributeFunc voipCfgDataRetrieveProFunc = {
	NULL,
	setGeneralValue
};

static AttributeFunc voipCfgDatProVersionFunc = {
	getVOIPCfgDatProVersion,
	NULL
};

omciAttriDescript_t omciAttriDescriptListVoipCfgData[]={
{0, "meId", 						2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{1, "Available protocol", 			1,	ATTR_ACCESS_R,		ATTR_FORMAT_BIT_FIELD,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipCfgDataAvaiProtoFunc},
{2, "protocol used",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipCfgDataProtoUsedFunc},
{3, "Available voip cfg methods",	4,	ATTR_ACCESS_R,		ATTR_FORMAT_BIT_FIELD,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipCfgDataAvaVoipCfgMethodsFunc},
{4, "voip cfg methods used",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipCfgDataMethodsUsedFunc},
{5, "voip cfg addr pointer", 		2,	ATTR_ACCESS_R_W,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{6, "voip cfg state",				1,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{7, "retrieve profile", 			1,	ATTR_ACCESS_W,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &voipCfgDataRetrieveProFunc},
{8, "profile version",	 			25,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,			0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, OMCI_AVC_ID_VOIP_CFG_PROFILE_VERSION, &voipCfgDatProVersionFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.10 network dial plan table

********************************************************************************************************************************/
static AttributeFunc netDialPlanTblDialNumberFunc = {
	getNetDialPlanTblDialNumber,
	NULL
};
static AttributeFunc netDialPlanTblDialFormatFunc = {
	getGeneralValue,
	setNetDialPlanTblDialFormat
};
static AttributeFunc netDialPlanTblDialTblFunc = {
	getNetDialPlanTblDialTbl,
	setNetDialPlanTblDialTbl
};

omciAttriDescript_t omciAttriDescriptListNetDialPlanTbl[]={
{0, "meId", 					2,	ATTR_ACCESS_R_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{1, "Dial plan number",			2,	ATTR_ACCESS_R, 				ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &netDialPlanTblDialNumberFunc},
{2, "Dial plan tbl max size",	2,	ATTR_ACCESS_R_SET_CREATE, 	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{3, "Critical dial timeout",	2,	ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{4, "Partial dial timeout",		2,	ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{5, "Dial plan format",			1,	ATTR_ACCESS_R_W_SET_CREATE, ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_PART_SUPPORT,	NO_AVC, &netDialPlanTblDialFormatFunc},
{6, "Dial plan tbl",			30,	ATTR_ACCESS_R_W,			ATTR_FORMAT_TABLE,			0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &netDialPlanTblDialTblFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.1 Physical path termination point POTS UNI

********************************************************************************************************************************/
static AttributeFunc pptpPotsUNIAdminStatusFunc = {
	getPptpPotsUNIAdminStatus,
	setPptpPotsUNIAdminStatus
};

static AttributeFunc pptpPotsUNIARCFunc = {
	getGeneralValue,
	setPptpPotsUNIARCValue
};

static AttributeFunc pptpPotsUNIARCIntervalFunc = {
	getGeneralValue,
	setPptpPotsUNIARCIntervalValue
};
static AttributeFunc PptpPotsUNIRxGainFunc = {
	getPptpPotsUNIRxGain,
	setPptpPotsUNIRxGain
};

static AttributeFunc PptpPotsUNITxGainFunc = {
	getPptpPotsUNITxGain,
	setPptpPotsUNITxGain
};

static AttributeFunc PptpPotsUNIHookStateFunc = {
	getPptpPotsUNIHookState,
	NULL
};

omciAttriDescript_t omciAttriDescriptListPptpPotsUNI[]={
{0, "meId",						2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0x1,0xFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "Administrative state",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&pptpPotsUNIAdminStatusFunc},
{2, "Interworking TP pointer",	2,	ATTR_ACCESS_R_W,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{3, "ARC",						1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0x1,	0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_PPTPPotsUNI_ARC,	&pptpPotsUNIARCFunc},
{4, "ARC interval",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0xFE,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &pptpPotsUNIARCIntervalFunc},
{5, "Impedance",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{6, "Transmission path",		1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{7, "Rx gain",					1, 	ATTR_ACCESS_R_W,	ATTR_FORMAT_SIGNED_INT,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &PptpPotsUNIRxGainFunc},
{8, "Tx gain",					1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_SIGNED_INT, 	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &PptpPotsUNITxGainFunc},
{9, "Operational state",		1,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT, 	0,	0x1,	0,	NULL,	ATTR_FULLY_SUPPORT, OMCI_AVC_ID_PPTPPotsUNI_OPERATIONAL_STATE, &generalGetFunc},
{10, "Hook state",				1,	ATTR_ACCESS_R,		ATTR_FORMAT_ENUM, 	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &PptpPotsUNIHookStateFunc},
{11, "POTS holdover time",		2,	ATTR_ACCESS_R_W,	ATTR_FORMAT_UNSIGNED_INT, 	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.9.11: VoIP line status

********************************************************************************************************************************/
static AttributeFunc VoIPLineStatusCodecsUsedFunc = {
	getVoIPStatusDataCodecsUsed,
	NULL
};
static AttributeFunc VoIPLineStatusVoiceServerStatusFunc = {
	getVoIPLineStatusVoiceServerStatus,
	NULL
};
static AttributeFunc VoIPLineStatusPortSessionTypeFunc = {
	getVoIPLineStatusPortSessionType,
	NULL
};
static AttributeFunc VoIPLineStatusCall1PacketPeriodFunc = {
	getVoIPLineStatusCall1PacketPeriod,
	NULL
};
static AttributeFunc VoIPLineStatusCall2PacketPeriodFunc = {
	getVoIPLineStatusCall2PacketPeriod,
	NULL
};


omciAttriDescript_t omciAttriDescriptListVoIPLineStatus[]={
{0, "meId",							2,	ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "voip codec used",				2,	ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0x12,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&VoIPLineStatusCodecsUsedFunc},
{2, "voip voice server status",		1,	ATTR_ACCESS_R,	ATTR_FORMAT_ENUM,           0,	0xE,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&VoIPLineStatusVoiceServerStatusFunc},
{3, "voip port session type",		1,	ATTR_ACCESS_R,	ATTR_FORMAT_ENUM,			0,	0x5,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&VoIPLineStatusPortSessionTypeFunc},
{4, "voip call 1 packet period",	2,	ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &VoIPLineStatusCall1PacketPeriodFunc},
{5, "voip call 2 packet period",	2,	ATTR_ACCESS_R,	ATTR_FORMAT_UNSIGNED_INT,	0,	0x0000FFFF,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &VoIPLineStatusCall2PacketPeriodFunc},
{6, "voip call 1 dest addr",		25,	ATTR_ACCESS_R,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{7, "voip call 2 dest addr",		25, ATTR_ACCESS_R,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT, NO_AVC, &generalGetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


/*******************************************************************************************************************************
VoIP Call statistics
********************************************************************************************************************************/
#if defined (TCSUPPORT_OMCI_ALCATEL) && defined (TCSUPPORT_VOIP)

static AttributeFunc voipCallHistoryTableGetFunc = {
	getCallHistoryTable,
	NULL,
};

omciAttriDescript_t omciAttriDescriptListVoIPCallStatistics[]={
{0, "meId",                 2,  ATTR_ACCESS_R,  ATTR_FORMAT_UNSIGNED_INT,   0,  0x0000FFFF, 0,  NULL,   ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "Call History Table",   2,  ATTR_ACCESS_R,  ATTR_FORMAT_TABLE,          0,  0,          0,  NULL,   ATTR_FULLY_SUPPORT,	NO_AVC,	&voipCallHistoryTableGetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL} 
};
#endif
