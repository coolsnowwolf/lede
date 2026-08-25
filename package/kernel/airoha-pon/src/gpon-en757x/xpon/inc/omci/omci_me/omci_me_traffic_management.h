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
	omci_me_traffic_management.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/

#ifndef _TRAFFIC_MANAGEMENT_ME_H_
#define _TRAFFIC_MANAGEMENT_ME_H_
#include "omci_types.h"
#include "omci_me.h"

//9.2.7: GAL Ethernet profile
extern omciAttriDescript_t omciAttriDescriptListGALEthernetProfile[];
int32 getGALEthernetProfileMaxGemPayloadSize(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setGALEthernetProfileMaxGemPayloadSize(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForGALEthernetProfile(omciManageEntity_t *omciManageEntity_p);

//9.2.8: GAL Ethernet performance monitoring history data
extern omciAttriDescript_t omciAttriDescriptListGALEthernetPM[];
int32 setGALEthernetPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGALEthernetPMDiscardedFrames(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForGALEthernetPM(omciManageEntity_t *omciManageEntity_p);

//9.2.9: FEC performance monitoring history data
extern omciAttriDescript_t omciAttriDescriptListFecPM[];
int32 setFecPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getFecPMCorrectedBytes(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getFecPMCorrectedCodeWords(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getFecPMUnCorrectedCodeWords(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getFecPMTotalCorrectedWords(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getFecPMFecSeconds(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForFecPM(omciManageEntity_t *omciManageEntity_p);

//9.2.4: GEM interworking termination point
extern omciAttriDescript_t omciAttriDescriptListGemIWTP[];
int32 getGemIWTPGalLoopbackCfg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setGemIWTPGalLoopbackCfg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGemIWTPPPTPCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getGemIWTPOperState(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int omciMeInitForGemIWTP(omciManageEntity_t *omciManageEntity_p);

//9.2.5: Multicast GEM interworking termination point
extern omciAttriDescript_t omciAttriDescriptListMulticastGemIWTP[];
int32 setMulticastGemIWTPNotUsed1(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getMulticastGemIWTPPPTPCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getMulticastGemIWTPOperState(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setMulticastGemIWTPNotUsed2(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getMulticastGemIWTPIPv4MultiAddrTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setMulticastGemIWTPIPv4MultiAddrTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int omciMeInitForMulticastGemIWTP(omciManageEntity_t *omciManageEntity_p);

#endif

