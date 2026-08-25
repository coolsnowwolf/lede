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
	enhanced_security_ctl_me_nodetable.c

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
#include "omci_me_enhanced_security_ctl.h"

/*******************************************************************************************************************************
						9.13.11: Enhanced security control

********************************************************************************************************************************/

static AttributeFunc enhancedSecurityCtlOLTCryptoCapabilitiesFunc = {
	NULL,
	setEnhancedSecurityCtlOLTCryptoCapabilities
};

static AttributeFunc enhancedSecurityCtlOLTRandomChallengeTbFunc = {
	getEnhancedSecurityCtlOLTRandomChallengeTb,
	setEnhancedSecurityCtlOLTRandomChallengeTb
};

static AttributeFunc enhancedSecurityCtlOLTChallengeStatusFunc = {
	getGeneralValue,
	setEnhancedSecurityCtlOLTChallengeStatus
};

static AttributeFunc enhancedSecurityCtlONURandomChallengeTBFunc = {
	getEnhancedSecurityCtlONURandomChallengeTB,
	NULL
};

static AttributeFunc enhancedSecurityCtlONUAuthenticationResultTBFunc = {
	getEnhancedSecurityCtlONUAuthenticationResultTB,
	NULL
};

static AttributeFunc enhancedSecurityCtlOLTAuthenticationResultTBFunc = {
	NULL,
	setEnhancedSecurityCtlOLTAuthenticationResultTB
};

static AttributeFunc enhancedSecurityCtlOLTResultStatusFunc = {
	getGeneralValue,
	setEnhancedSecurityCtlOLTResultStatus
};

omciAttriDescript_t omciAttriDescriptListEnhancedSecurityCtl[]={
{0, "meId",						2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,																&generalGetFunc},
{1, "OLT crypto capabilities",		16,	ATTR_ACCESS_W,		ATTR_FORMAT_BIT_FIELD,		0,		0,		0x7,	NULL,	ATTR_PART_SUPPORT,		NO_AVC,																&enhancedSecurityCtlOLTCryptoCapabilitiesFunc},
{2, "OLT random challenge tb",		17,	ATTR_ACCESS_R_W,	ATTR_FORMAT_TABLE,			0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,																&enhancedSecurityCtlOLTRandomChallengeTbFunc},
{3, "OLT challenge status",			1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,																&enhancedSecurityCtlOLTChallengeStatusFunc},
{4, "ONU selected crypto",			1,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,																&generalGetFunc},
{5, "ONU random challenge tb",		0,	ATTR_ACCESS_R,		ATTR_FORMAT_TABLE,			0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_RANDOM_CHALLENGE_TB,		&enhancedSecurityCtlONURandomChallengeTBFunc},
{6, "ONU authentication result tb",	0,	ATTR_ACCESS_R,		ATTR_FORMAT_TABLE,			0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_AUTHENTICATION_RESULT_TB,	&enhancedSecurityCtlONUAuthenticationResultTBFunc},
{7, "OLT authentication result tb",	17,	ATTR_ACCESS_W,		ATTR_FORMAT_TABLE,			0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,																&enhancedSecurityCtlOLTAuthenticationResultTBFunc},
{8, "OLT result status",				1,	ATTR_ACCESS_R_W,	ATTR_FORMAT_ENUM,			0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,																&enhancedSecurityCtlOLTResultStatusFunc},
{9, "ONU authentication state",		1,	ATTR_ACCESS_R,		ATTR_FORMAT_ENUM,			0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_AUTHENTICATION_STATUS,		&generalGetFunc},
{10, "master session key name",		16,	ATTR_ACCESS_R,		ATTR_FORMAT_STRING,		0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,																&generalGetFunc},
{11, "broadcast key table",			0,	ATTR_ACCESS_R_W,	ATTR_FORMAT_TABLE,			0,		0,		0,	NULL,	ATTR_UNSUPPORT,			NO_AVC,																NULL},
{12, "effective key length",			2,	ATTR_ACCESS_R,		ATTR_FORMAT_UNSIGNED_INT,	0,		0,		0,	NULL,	ATTR_FULLY_SUPPORT,		NO_AVC,																&generalGetFunc},
{255, "", 0, 0,  0, 0, 0, 0, NULL, 0, 0, NULL}
};


