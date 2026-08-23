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
	omci_me_layer2_data_services.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/

#ifndef _LAYER2_DATA_SERVICES_ME_H_
#define _LAYER2_DATA_SERVICES_ME_H_
#include "omci_types.h"
#include "omci_me.h"

//9.3.18: Dot1 rate limiter
extern omciAttriDescript_t omciAttriDescriptListDot1RateLimiter[];
int32 setDot1RateLimiterTPType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForDot1RateLimiter(omciManageEntity_t *omciManageEntity_p);

#ifdef TCSUPPORT_OMCI_DOT1AG
//9.3.19 Dot1ag maintenance domain
extern omciAttriDescript_t omciAttriDescriptListDot1agMaintenanceDomain[];
int32 setDot1agMaintenanceDomainMDLevel(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceDomainMDNameFormat(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceDomainMDName1(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceDomainMDName2(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceDomainMHFCreation(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceDomainSenderIDPermission(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForDot1agMaintenanceDomain(omciManageEntity_t *omciManageEntity_p);

//9.3.20 Dot1ag maintenance association
extern omciAttriDescript_t omciAttriDescriptListDot1agMaintenanceAssociation[];
int32 setDot1agMaintenanceDomainMANameFormat(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceAssociationShortMAName1(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceAssociationShortMAName2(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceAssociationCCMInterval(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceAssociationAssociatedVlans(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceAssociationMHFCreation(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setDot1agMaintenanceAssociationSenderIDPermission(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForDot1agMaintenanceAssociation(omciManageEntity_t *omciManageEntity_p);
#endif

int gponIoctl(int cmd, void *data);

int32 setEthnetFrameExtendedPMFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthnetFrameExtendedPMFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthnetFrameExtendedPMcontrolblockFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

extern omciAttriDescript_t omciAttriDescriptListMBEthnetFrameExtendedPM[];
extern omciAttriDescript_t omciAttriDescriptListEthernetFrameExtendedPM64bit[];
int omciMeInitForEthernetFrameExtendedPM(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForEthernetFrameExtendedPM64bit(omciManageEntity_t *omciManageEntity_p);

//9.3.33 MAC bridge port ICMPv6 process preassign table
extern omciAttriDescript_t omciAttriDescriptListMBPortICMPv6Process[];
int32 setMACBridgePortICMPv6ProcessMsg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int omciMeInitForMACBridgePortICMPv6Process(omciManageEntity_t *omciManageEntity_p);


//9.3.34 Ethnet Frame Extended PM 64Bit
#define PM_64BIT_CTRL_BLK_ATTRI_NUM		8
#pragma pack (1)
typedef struct Ctrlfield_s
{
    uint16 	vid_set		:1;
    uint16	p_bit_set	:1;
    uint16	resv		:12;
    uint16	direction	:1;
    uint16	lsb			:1;
}Ctrlfield_t,*Ctrlfield_ptr;

typedef struct EthExtndPM64CtrlBlk_s
{
	uint16 		threshold3;
    uint16		me_type;
    uint16		me_instance;
    uint16		accumulate_disable;
    uint16		tca_disable;
    Ctrlfield_t	ctrl_field;
    uint16 		tci;
    uint16		resved;
}EthExtndPM64CtrlBlk_t, *EthExtndPM64CtrlBlk_ptr;
#pragma pack ()

enum omciEthExtendPM64Idx {
	PM_DROP_EVENT = 1,
    PM_CRC_ERROR_FRAMES = 6,
    PM_UNDERSIZE_FRAMES,
    PM_OVERSIZE_FRAMES,    
};
enum omciEthExtendPM64ThrholdIdx {
	THRHOLD_DROP_EVENT = 1,
    THRHOLD_CRC_ERROR_FRAMES,
    THRHOLD_UNDERSIZE_FRAMES,
    THRHOLD_OVERSIZE_FRAMES,    
};

#define PM_64BIT_ALERT_ATTRI_NUM		4

typedef struct EthExtendPM64ThrholdPmMap_s 
{
	uint 	threhold_idx;
    uint 	pm_idx;
}EthExtendPM64ThrholdPmMap_t;

int32 setEthnetFrameExtendedPM64BitFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthnetFrameExtendedPM64BitFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getEthnetFrameExtendedPM64BitcontrolblockFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setEthnetFrameExtendedPM64BitctlblkFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#endif

