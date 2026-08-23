
#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_layer3_data_services.h"


/*******************************************************************************************************************************
9.4.1: IP host config data

********************************************************************************************************************************/
#if defined(TCSUPPORT_PON_IP_HOST)
static AttributeFunc IPHostConfigDataIPOptionsFunc = {
	getGeneralValue,
	setIPHostConfigDataIPOptions
};
#endif

static AttributeFunc IPHostConfigDataMACAddrFunc = {
	getIPHostConfigDataMACAddr,
	NULL
};

static AttributeFunc IPHostConfigDataIPAddrFunc = {
	getIPHostConfigDataIPAddr,
	setIPHostConfigDataIPAddr
};

static AttributeFunc IPHostConfigDataMaskFunc = {
	getIPHostConfigDataMask,
	setIPHostConfigDataMask
};

static AttributeFunc IPHostConfigDataGatewayFunc = {
	getGeneralValue,
	setIPHostConfigDataGateway
};

static AttributeFunc IPHostConfigDataPrimaryDNSFunc = {
	getGeneralValue,
	setIPHostConfigDataPrimaryDNS
};

static AttributeFunc IPHostConfigDataSecondaryDNSFunc = {
	getGeneralValue,
	setIPHostConfigDataSecondaryDNS
};

static AttributeFunc IPHostConfigDataCurAddrFunc = {
	getIPHostConfigDataCurAddr,
	NULL
};

static AttributeFunc IPHostConfigDataCurMaskFunc = {
	getIPHostConfigDataCurMask,
	NULL
};

static AttributeFunc IPHostConfigDataCurGatewayFunc = {
	getIPHostConfigDataCurGateway,
	NULL
};

static AttributeFunc IPHostConfigDataCurPrimaryDNSFunc = {
	getIPHostConfigDataCurPrimaryDNS,
	NULL
};

static AttributeFunc IPHostConfigDataCurSecondaryDNSFunc = {
	getIPHostConfigDataCurSecondaryDNS,
	NULL
};

omciAttriDescript_t omciAttriDescriptListIPHostConfigData[]={
{0, "meId",					2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&generalGetFunc},
#ifdef TCSUPPORT_PON_IP_HOST
{1, "ip options",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&IPHostConfigDataIPOptionsFunc},
#else
{1, "ip options",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_BIT_FIELD,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&generalGetSetFunc},
#endif
{2, "mac addr",				6,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&IPHostConfigDataMACAddrFunc},
{3, "onu identifier",				25,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&generalGetSetFunc},
{4, "ip addr",					4,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&IPHostConfigDataIPAddrFunc},
{5, "mask",					4,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&IPHostConfigDataMaskFunc},
{6, "gateway",				4,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&IPHostConfigDataGatewayFunc},
{7, "primary dns",				4,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&IPHostConfigDataPrimaryDNSFunc},
{8, "secondary dns",			4,	ATTR_ACCESS_R_W,	ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,										&IPHostConfigDataSecondaryDNSFunc},
{9, "current addr",				4,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_IP_HOST_CUR_ADDR,				&IPHostConfigDataCurAddrFunc},
{10, "current mask",			4,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_IP_HOST_CUR_MASK,				&IPHostConfigDataCurMaskFunc},
{11, "current gateway",			4,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_IP_HOST_CUR_GATEWAY,			&IPHostConfigDataCurGatewayFunc},
{12, "current primary dns",		4,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_IP_HOST_CUR_PRIMARY_DNS,		&IPHostConfigDataCurPrimaryDNSFunc},
{13, "current secondary dns",	4,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_IP_HOST_CUR_SECONDARY_DNS,	&IPHostConfigDataCurSecondaryDNSFunc},
{14, "domain name",			25,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_IP_HOST_DOMAIN_NAME,			&generalGetFunc},
{15, "host name",				25,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	OMCI_AVC_ID_IP_HOST_HOST_NAME,				&generalGetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.4.2: IP host performance monitoring history data

********************************************************************************************************************************/

static AttributeFunc IPHostPMThresholdDataFunc = {
	getGeneralValue,
	setIPHostPMThresholdData
};

static AttributeFunc IPHostPMICMPErrorsFunc = {
	getIPHostPMICMPErrors,
	NULL
};

static AttributeFunc IPHostPMDNSErrorsFunc = {
	getIPHostPMDNSErrors,
	NULL
};

static AttributeFunc IPHostPMDHCPTimeoutFunc = {
	getIPHostPMIPHostPMDHCPTimeout,
	NULL
};

static AttributeFunc IPHostPMIPConflictFunc = {
	getIPHostPMIPHostPMIPConflict,
	NULL
};

static AttributeFunc IPHostPMOutOfMemoryFunc = {
	getIPHostPMOutOfMemory,
	NULL
};

static AttributeFunc IPHostPMInternalErrorFunc = {
	getIPHostPMInternalError,
	NULL
};

omciAttriDescript_t omciAttriDescriptListIPHostPM[]={
{0, "meId",				2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "interval end time",		1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",	2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&IPHostPMThresholdDataFunc},
{3, "icmp errors",			4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&IPHostPMICMPErrorsFunc},
{4, "dns errors",			4,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&IPHostPMDNSErrorsFunc},
{5, "dhcp timeouts",		2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&IPHostPMDHCPTimeoutFunc},
{6, "ip addr conflict",		2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&IPHostPMIPConflictFunc},
{7, "out of memory",		2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&IPHostPMOutOfMemoryFunc},
{8, "internal error",			2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&IPHostPMInternalErrorFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.4.3: TCP/UDP config data

********************************************************************************************************************************/

omciAttriDescript_t omciAttriDescriptListTcpUdpConfigData[]={
{0, "meId",				2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "port id",				2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{2, "protocol",				1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{3, "tos or diffserv field",	1,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_ENUM,			0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{4, "ip host pointer",		2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetSetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};

/*******************************************************************************************************************************
9.4.4: TCP/UDP performance monitoring history data

********************************************************************************************************************************/

static AttributeFunc TcpUdpPMThresholdDataFunc = {
	getGeneralValue,
	setTcpUdpPMThresholdData
};

static AttributeFunc TcpUdpPMSocketFailedFunc = {
	getTcpUdpPMSocketFailed,
	NULL
};

static AttributeFunc TcpUdpPMListenFailedFunc = {
	getTcpUdpPMListenFailed,
	NULL
};

static AttributeFunc TcpUdpPMBindFailedFunc = {
	getTcpUdpPMBindFailed,
	NULL
};

static AttributeFunc TcpUdpPMAcceptFailedFunc = {
	getTcpUdpPMAcceptFailed,
	NULL
};

static AttributeFunc TcpUdpPMSelectFailedFunc = {
	getTcpUdpPMSelectFailed,
	NULL
};

omciAttriDescript_t omciAttriDescriptListTcpUdpPM[]={
{0, "meId",				2,	ATTR_ACCESS_R_SET_CREATE,		ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetFunc},
{1, "interval end time",		1,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&generalGetIntervalEndTimeFunc},
{2, "threshold data 1/2 id",	2,	ATTR_ACCESS_R_W_SET_CREATE,	ATTR_FORMAT_POINTER,		0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&TcpUdpPMThresholdDataFunc},
{3, "socket failed",			2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&TcpUdpPMSocketFailedFunc},
{4, "listen failed",			2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&TcpUdpPMListenFailedFunc},
{5, "bind failed",			2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&TcpUdpPMBindFailedFunc},
{6, "accept failed",			2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&TcpUdpPMAcceptFailedFunc},
{7, "select failed",			2,	ATTR_ACCESS_R,					ATTR_FORMAT_UNSIGNED_INT,	0,	0,	0,	NULL,	ATTR_FULLY_SUPPORT,	NO_AVC,	&TcpUdpPMSelectFailedFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


