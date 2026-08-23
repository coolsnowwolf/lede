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
	enhanced_security_ctl_me.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/

#ifndef _ENHANCED_SECURITY_CTL_ME_H_
#define _ENHANCED_SECURITY_CTL_ME_H_
#include "omci_types.h"
#include "omci_me.h"

#define GPON_ONU           			"GPON_ONU"
#define ENHANCED_PSK			"EnhancedPSK"

//9.13.11: Enhanced security control
extern omciAttriDescript_t omciAttriDescriptListEnhancedSecurityCtl[];
int32 setEnhancedSecurityCtlOLTCryptoCapabilities(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEnhancedSecurityCtlOLTRandomChallengeTb(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setEnhancedSecurityCtlOLTRandomChallengeTb(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setEnhancedSecurityCtlOLTChallengeStatus(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEnhancedSecurityCtlONURandomChallengeTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEnhancedSecurityCtlONUAuthenticationResultTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setEnhancedSecurityCtlOLTAuthenticationResultTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setEnhancedSecurityCtlOLTResultStatus(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 omciMeInitForEnhancedSecurityCtl(omciManageEntity_t *omciManageEntity_p);

#endif

