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
	oam_ctc_node.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef INCLUDE_OAM_INIT_H
#define INCLUDE_OAM_INIT_H
#include "../epon_oam_types.h"

/*
 * restore and clean func for cuc oam configuration.
 */
typedef struct oamParameterInit{
	char	name[OAM_MAX_PARAM_NAME_LEN];	// The Attribute/Action's name
	int	 (*oamInitFunc)(void);	//restore configuration from romfile	
	int	 (*oamCleanFunc)(void);  // clean configuration in romfile
}OamParamInit_t, *OamParamInit_Ptr;

extern OamParamInit_t cucOamInitTable[];

int oamClearRomfileConf(void);
int eponOamInitParam2(void);

int oamInitFecMode(void);
int oamInitPowerSavingConfig(void);
int oamInitProtectionParameters(void);

int oamInitServiceSLA(void);
int oamInitHoldoverConfig(void);
int oamInitActivePONIFAdminstate(void);
int oamInitEthPortPause(void);

int oamInitEthPortUSPolicing(void);
int oamInitVoIPPort(void);
int oamInitE1Port(void);
int oamInitEthPortDSRateLimiting(void);
int oamInitPortLoopDetect(void);
int oamInitPortDisableLooped(void);
int oamInitPortMacAgingTime(void);
int oamInitPortMacLimit(void);
int oamInitPortModeCfg(void);

int oamInitVlan(void);
int oamInitClassificationMarking(void);

int oamInitMulticastVlan(void);
int oamInitMulticastTagOper(void);
int oamInitMulticastSwitch(void);
int oamInitMulticastControl(void);

int oamInitGroupNumMax(voip);
int oamInitLlidQueueConfig(void);
int oamInitMxUMngGlobalParameter(void);

int oamInitONUTxPowerSupplyControl(void);

int oamInitGlobalParameterConfig(void);
int oamInitSIPParameterConfig(void);
int oamInitSIPUserParameterConfig(void);
int oamInitFaxModemConfig(void);

int oamInitAlarmAdminState(void);
int oamInitAlarmThresholds(void);

int oamInitPerformanceStatus(void);

int oamInitPhyAdminControl(void);
int oamInitAutoNegAdminControl(void);
int oamInitSleepControl(void);

int oamInitMultiLlidAdminControl(void);
int oamInitFastLeaveAdminControl(void);
int oamInitSIPDigitMap(void);
#endif




