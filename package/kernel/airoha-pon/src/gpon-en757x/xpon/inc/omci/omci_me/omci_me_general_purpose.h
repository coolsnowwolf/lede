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
	omci_me_general_purpose.h
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/7/16	Create
        Andy.Yi                 2012/7/27       add the 9.12.1-11 me node
        Andy.Yi                 2012/8/7        add the 9.12.12-14 me node
*/
#ifndef _9_12_ME_H_
#define _9_12_ME_H_
#include "omci_types.h"
#include "omci_me.h"

#define ONLY_OMCI 			0
#define ONLY_NOOMCI		1
#define OMCI_AND_NOOMCI 	2

#define MAX_GERNEL_BUFFER_SIZE 	1024 *16
#define MAX_STRING_SIZE 			375

typedef struct __bufferMngType
{
	uint8 *bufferPoint;
	uint32 bufferSize;
	uint32 actualSize;
}bufferMngType;

int addOneBuffer(omciMeInst_t *meInstant_ptr, uint32 bufferSize);
int delOneBuffer(omciMeInst_t *meInstant_ptr);

int omciInitInstForUniG(uint16 instanceId);
int omciInitInstForOltG(void);
int omciInitInstForOmci(void);
int omciInitInstForManagedEntity(void);
int omciInitInstForAttribute(void);
int omciInitInstForFileTransController(void);


int omciMeInitForUniG(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForOltG(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForNetworkAddress(omciManageEntity_t *omciManageEntity_p);	
int omciMeInitForAuthenticationMethod(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForLargeString(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForThresholdData1(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForThresholdData2(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForOmci(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForManagedEntity(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForAttribute(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForOctetString(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForGeneralBuffer(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForFileTransController(omciManageEntity_t *omciManageEntity_p);
int omciMeInitForGenericStatus(omciManageEntity_t *omciManageEntity_p);



int32 setOLTVendorIdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setOLTEquipmentIdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setOLTVersionValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setTimeOfDayValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 setLargeStringValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 getTimeOfDayValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getMessageTypeTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getMETypeTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);


int32 getAttributeTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);


int32 getAlarmsTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getAVCsTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getInstancesTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getCodePointsTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 setMaximumSizeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 getGeneralBufferTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 setFileTypeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setFileInstanceValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setLocalFileNameValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 setNetworkAddressValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);



int32 setFileTransferTriggerValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);

int32 getStatusDocumentValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getConfugurationdocumentValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 setAVCReportRateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

int omciMeCreateHandleForGeneralBuffer(omciManageEntity_t *me_p, omciMeInst_t *meInstant_ptr, uint8 *contentPtr);
int omciMeDeleteHandleForGeneralBuffer(omciMeInst_t *meInstant_ptr);


extern omciAttriDescript_t omciAttriDescriptListUniG[];
extern omciAttriDescript_t omciAttriDescriptListOltG[];
extern omciAttriDescript_t omciAttriDescriptListNetworkAddress[];
extern omciAttriDescript_t omciAttriDescriptListAuthenticationMethod[];
extern omciAttriDescript_t omciAttriDescriptListLargeString[];
extern omciAttriDescript_t omciAttriDescriptListThresholdData1[];
extern omciAttriDescript_t omciAttriDescriptListThresholdData2[];
extern omciAttriDescript_t omciAttriDescriptListOmci[];
extern omciAttriDescript_t omciAttriDescriptListManagedEntity[];
extern omciAttriDescript_t omciAttriDescriptListAttribute[];
extern omciAttriDescript_t omciAttriDescriptListOctetString[];
extern omciAttriDescript_t omciAttriDescriptListGeneralBuffer[];
extern omciAttriDescript_t omciAttriDescriptListFileTransController[];
extern omciAttriDescript_t omciAttriDescriptListGenericStatus[];

#ifdef TCSUPPORT_CWMP
int omciInitInstForTr069ManageServer(void);
#endif

#ifdef TCSUPPORT_SNMP
int omciMeInitForSnmpConfigData(omciManageEntity_t *omciManageEntity_p);
#endif
#ifdef TCSUPPORT_CWMP
int omciMeInitForTr069ManageServer(omciManageEntity_t *omciManageEntity_p);
#endif

#ifdef TCSUPPORT_SNMP
int32 setSnmpVersionValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setSnmpAgentIpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setSnmpServerIpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setSnmpServerPortValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setSnmpReadCommunityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setSnmpWriteCommunityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setSnmpSysNameValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
#endif
#ifdef TCSUPPORT_CWMP
int32 getTr069ManageAdmin(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag);
int32 setTr069ManageAdmin(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setTr069AcsAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setTr069AssociateTag(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
#endif

#ifdef TCSUPPORT_SNMP
extern omciAttriDescript_t omciAttriDescriptListSnmpConfigData[];
#endif
#ifdef TCSUPPORT_CWMP
extern omciAttriDescript_t omciAttriDescriptListTr069ManageServer[];
#endif

#ifdef TCSUPPORT_CUC
extern omciAttriDescript_t omciAttriDescriptListCucOnuCapability[];
int omciInitInstForCucOnuCapability();
int omciMeInitForCucOnuCapability(omciManageEntity_t *omciManageEntity_p);
int32 getCucOnuCapOperatorID(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getCucOnuCapSpecVer(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getCucOnuCapOnuType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int32 getCucOnuCapOnuTxPowerCtrl(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
#endif

#ifdef TCSUPPORT_VNPTT
int omciInitInstForRsv347ME();
#endif

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
int omciInitInstForRsv350ME();
int omciInitInstForRsv352ME();
int omciInitInstForRsv353ME();
int omciInitInstForRsv367ME();
int omciInitInstForRsv373ME();
#endif


#endif
