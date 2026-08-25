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
	9_1_ME.h
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/7/16	Create
*/

#ifndef _9_1_ME_H_
#define _9_1_ME_H_
#include "omci_types.h"
#include "omci_me.h"

#define	DEBUG_CMD_ASCII_FORMAT	0
#define	DEBUG_CMD_FREE_FORMAT	1

#define	SOFTWARE_IMAGE_STATE_S0			0
#define	SOFTWARE_IMAGE_STATE_S1_0		1
#define	SOFTWARE_IMAGE_STATE_S2_0		2
#define	SOFTWARE_IMAGE_STATE_S3_0		3
#define	SOFTWARE_IMAGE_STATE_S4_0		4
#define	SOFTWARE_IMAGE_STATE_S4_1		5
#define	SOFTWARE_IMAGE_STATE_S3_1		6
#define	SOFTWARE_IMAGE_STATE_S2_1		7
#define	SOFTWARE_IMAGE_STATE_S1_1		8

int omciMeInitForOnuRemoteDebug(omciManageEntity_t *omciManageEntity_p);
int omciInitInstForOnuRemoteDebug(void);
int omciMeInitForSoftwareImage(omciManageEntity_t *omciManageEntity_p);
int omciInitInstForSoftwareImage(void);


int32 setRemoteDebugCmd(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getRemoteDebugReplyTab(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

extern omciAttriDescript_t omciAttriDescriptListOunRemoteDebug[];
extern omciAttriDescript_t omciAttriDescriptListSoftwareImage[];

#define AVC_ONU2_G_OMCC_VERSION				((OMCI_CLASS_ID_ONU2_G <<16) | 2)

int omciInitInstForOnuG(void);
int omciInitInstForOnu2G(void);
int omciInitInstForOnuData(void);
int omciInitInstForCardholder(void);
int omciInitInstForCircuitPack(void);
int omciInitInstForOnuPowerShedding(void);
int omciInitInstForPortMapping(void);
int omciInitInstForOnuE(void);

int omciMeInitForOnuG(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForOnu2G(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForOnuData(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForCardholder(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForCircuitPack(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForOnuPowerShedding(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForPortMapping(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForExtensionPackage(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForProtectionData(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForProtectionProfile(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForOnuE(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForOnuDynamicPower(omciManageEntity_t *omciManageEntity_p);

int32 setONUGAdminStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getONUGOpStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getOMCCVersionValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setSecurityModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getSysUpTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getMIBDataSyncValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setMIBDataSyncValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

#ifdef TCSUPPORT_CUC
int32 getONUGLOID(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag);
int32 getONUGLOIDPWD(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag);
#endif

int8 getSoftwareImageState(omciMeInst_t *image_0, omciMeInst_t *image_1);
int8 setSoftwareImageState(omciMeInst_t *image_0, omciMeInst_t *image_1, int imageState);
int8 softwareImageStateMachine(int action, uint8 *currentState, uint16 instanceId, uint8 CRC_OK);

extern alarm_id_map_entry_t meMapTableOnuG[];

extern omciAttriDescript_t omciAttriDescriptListOnuG[];
extern omciAttriDescript_t omciAttriDescriptListOnu2G[];
extern omciAttriDescript_t omciAttriDescriptListOnuData[];
extern omciAttriDescript_t omciAttriDescriptListCardholder[];
extern omciAttriDescript_t omciAttriDescriptListCircuitPack[];
extern omciAttriDescript_t omciAttriDescriptListOnuPowerShedding[];
extern omciAttriDescript_t omciAttriDescriptListPortMapping[];
extern omciAttriDescript_t omciAttriDescriptListExtensionPackage[];
extern omciAttriDescript_t omciAttriDescriptListProtectionData[];
extern omciAttriDescript_t omciAttriDescriptListProtectionProfile[];
extern omciAttriDescript_t omciAttriDescriptListOnuE[];
extern omciAttriDescript_t omciAttriDescriptListOnuDynamicPower[];
#ifdef TCSUPPORT_OMCI_ALCATEL
extern omciAttriDescript_t omciAttriDescriptListONTOpticalSupervisionStatus[];
int omciInitInstForONTOpticalSupervisionStatus(void);
int omciInternalCreateForONTOpticalSupervisionStatus(uint16 classId, uint16 instanceId);
int omciMeInitForONTOpticalSupervisionStatus(omciManageEntity_t *omciManageEntity_p);
int32 getPowerFeedVoltageValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getReceiveOpticalPowerValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getTransmitOpticalPowerValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getLaserBiasCurrentValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getTemperatureValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

#define MAX_OMCI_LAN_PORT_NUM			4
#define MAX_OMCI_LAN_PORT_MASK			0x0f
#define OMCI_LAN_PORT1_OTHER_MASK		0xf0
#define MAX_OMCI_LAN_PORT_DEFAULT_MASK	0xff

#define OMCI_MAC_BRIDGE_LAN_PORT1		0 
#define OMCI_MAC_BRIDGE_LAN_PORT2		1 
#define OMCI_MAC_BRIDGE_LAN_PORT3		2 
#define OMCI_MAC_BRIDGE_LAN_PORT4		3
extern omciAttriDescript_t omciAttriDescriptListONTGenericV2[];
int omciInitInstForONTGenericV2(void);
int omciInternalCreateForONTGenericV2(uint16 classId, uint16 instanceId);
int omciMeInitForONTGenericV2(omciManageEntity_t *omciManageEntity_p);
int32 getEthernetPortToPortTrafficEnableIndicatorValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setEthernetPortToPortTrafficEnableIndicatorValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#endif

#endif
