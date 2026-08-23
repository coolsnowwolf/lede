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
	omci_utily.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Andy.Yi		2012/7/3	Create
*/

#ifndef _GENERAL_FUNC_H_
#define _GENERAL_FUNC_H_
#include "omci_types.h"
#include "omci_me.h"
#include "omci.h"
#include "omci_utily.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../../apps/public/mtd/tc_partition.h"
#include "../../../lib/gponmap/libgponmap.h"

#if 0
int tcapi_set(char *name, char *subName, char *value);
int tcapi_get(char *name, char *subName, char *value);
int tcapi_commit(char* value);


#else
#include "../lib/libtcapi.h"
#endif

#define OMCI_UPLOAD_FW "var/tmp/omci_fw"
// It is truly a disaster to define  these addresses(offset) randomly
// Always a good habit to define them in the same  place, for easy understanding and less opportunity to mess up with the flash layout

#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_COMMIT_FLAG_SIZE 	1
#define SET_COMMIT_FILE 	"/tmp/set_commit_flag"
#define READ_COMMIT_FILE 	"/tmp/read_commit_flag"
#define IMG_BOOT_FLAG_FILE 	"/tmp/image_boot_flag"
#define IMG_BOOT_FLAG_READ_FILE 	"/tmp/image_boot_flag_read"


#define GPON               			"GPON"
#define GPONCOMMONNODE 		"GPON_Common"
#define CURRENTATTRIBUTE 		"CurrentAttribute"
#define MAX_BUFFER_SIZE 		64
#define OMCI_SOFTWAREIMAGE_STATE       "OMCI_SoftWare_Image_State"

#define DOWNLOAD_SEC_AR		(1<<6)

#define TCBOOT_BASE (0x80020000)
#define FLASH_BASE (0xb0000000)
#define RESERVEAREA_BLOCK_BASE 0
#define SOFTWARE_RESTART 1
#define REBOOT_IMAGE_FLAG_SIZE 0x01
#define REBOOT_IMAGE_FLAG_OFFSET (RESERVEAREA_BLOCK_BASE+0X601)


#define PM_ENTRY                                		"PM_Entry"
#define PMMGR_HISTORY_DATA			0        //data flag get PM me instance history data
#define PMMGR_CURRENT_DATA			1	 //data flag get PM me instance current data
#define PMMGR_THRESHOLD                  		2       //data flag get /set pm me instance threshold
#define PMMGR_CREATE					3	//data flag create pm me instance
#define PMMGR_DELETE					4	//data flag delete pm me instance
#define PMMGR_SYNCHRONIZE_TIME		5	//data flag synchronize time
#define PMMGR_STATISTICS_TIME			6	//data flag get statistics time
#define PMMGR_INIT						7     //re-init pmgr
#define PMMGR_PORT_ID					"portId"	
#define PMMGR_CLASS_ID					"classId"
#define PMMGR_DATA_FLAG				"dataFlag"
#define PMMGR_ATTR_INDEX				"attrIndex"      //attr index in pm struct
#define PMMGR_ATTR_DATA				"attrData"       //attr data when get, or threshold data when set

typedef struct __pthreadArgImage{
		omciMeInst_t *imageInst_0;
		omciMeInst_t *imageInst_1;
		uint16 instanceId;		
		uint8 currentImageState;
}pthreadArgImage;

#define OMCI_USER_ISOLATION_DISABLE	 	0
#define OMCI_USER_ISOLATION_ENABLE	 	1
#define OMCI_USER_ISOLATION_SET_GROUP 	2

#define OMCI_USER_ISOLATION_MAX_PORT	3
typedef struct pthreadArgUserIso_s{
	uint8 action; 
	uint8 portId;
	uint8 reserved[2];
	uint32 group;
}pthreadArgUserIso_t, *pthreadArgUserIso_p;

#define OMCI_THREAD_BR_PRO_STP		(1<<0)
#define OMCI_THREAD_BR_PRO_PRI		(1<<1)
#define OMCI_THREAD_BR_PRO_MAXAGE	(1<<2)
#define OMCI_THREAD_BR_PRO_HELLO	(1<<3)
#define OMCI_THREAD_BR_PRO_FORWARD	(1<<4)
typedef struct pthreadArgOmciBrPro_s{//mac bridge service profile
	uint16 mask;
	uint16 mbStp;
	uint16 mbPri;
	uint16 mbMaxAge;
	uint16 mbHello;
	uint16 mbForward;	
}pthreadArgOmciBrPro_t, *pthreadArgOmciBrPro_p;


#define OMCI_CREATE_INSTANCE_BY_THREAD	1
#define OMCI_DEL_INSTANCE_BY_THREAD		2
typedef struct pthreadArgHandleOmciInst_s{
	uint16 classId;
	uint16 instId;
	uint8 action; // 1:create, 2:del
	uint8 reserve[3];	
}pthreadArgHandleOmciInst_t, *pthreadArgHandleOmciInst_p;

typedef struct commitNode_s{
	char nodeName[32];
	struct commitNode_s *next;
}commitNode_t,*commitNode_p;

typedef struct commitNodeList_s{
	commitNode_p pHeadNode;
	pthread_mutex_t lock;
}commitNodeList_t,*commitNodeList_p;

typedef struct setNode_s{
    char nodeName[32];
    char attName[32];
    char val[32];
}setNode_t,*setNode_p;

typedef struct setPMNode_s{
    uint16 classId;
    uint16 deviceId;
    uint8 dataFlag;
    uint8 attrIndex;
    char *buffer;
}setPMNode_t,*setPMNode_p;


extern int omciCommitIPHostConfigData(uint16 meClassId , omciMeInst_t *meInstant_ptr);
int pmmgrTcapiGet(uint16 classId, uint16 portId, uint8 dataFlag, uint8 attrIndex, char *buffer);
int pmmgrTcapiSet(uint16 classId, uint16 portId, uint8 dataFlag, uint8 attrIndex, char *buffer);

void *osMalloc(size_t size);
void osFree(void *ptr);
int32 getTheValue(char* value, char *valueString, uint8 size, omciAttriDescript_t *omciAttribute);

OUT int getThresholdDataValue(uint16 classId, uint16 instanceId, uint8 index, OUT uint32 *thresholdData);
OUT int get64BitThresholdDataValue(uint16 classId, uint16 instanceId, uint8 index, uint64 *thresholdData);
OUT uint16 omciGetMeOfInstanceId(omciManageEntity_t *omciManageEntity_p, uint16 start, uint16 end, int8 step);
OUT int addOneStringToInstance(uint16 classId, uint16 instanceId, IN uint8 *string, IN uint16 length);
OUT int getOneStringFromInstance(uint16 classId, uint16 instanceId, OUT uint8 *buffer, OUT uint16 *length);

int omciCreateAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);

int omciDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);

int omciSetAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
				

int omciGetAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
	
	
int omciGetAllAlarmsAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);

int omciGetAllAlarmsNextAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);


int omciMIBUploadAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
	
	
int omciMIBUploadNextAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);

int omciMIBResetAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);

int omciTestAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);

int omciSyncTimeAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);

int omciRebootAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciGetNextAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);

int omciGetCurrentDataAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
#ifdef TCSUPPORT_OMCI_EXTENDED_MSG
int omciSetTableAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
#endif
void softImageSetBootFlag(int flag);
uchar softImageGetBootFlag(void);
uchar softImageReadBootFlag(void);

void softImageSetCommitFlag(int flag);
uchar softImageReadCommitFlag(void);

int omciStartSwDownloadAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciDownloadSecAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciEndSwDownloadAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciActiveSwAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciCommitSwAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciHandshakeAVCResponseAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);

int tcApiSetAttriubte(char *name, char *subName, char *value);

OUT int omciInternalNotificationPktSend( IN omciPayload_t *omciPayLoad);

int32 getGeneralValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);
int32 setGeneralValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);

uint16 parseAttributeMask(uint8 *contentPtr);
int reparseAttributeMask(uint16 attributeMask, uint8 *contentPtr);
void responseMessageTypeHandle(uint8 *mtPtr);

int writeAttributeValueToOmciPayLoad(uint8* attributeValuePtr, uint8 *responseContentPtr, uint16 length);
int writeResultAndReasonToOmciPayLoad(uint8 result, uint8* responseContentPtr);
uint8 *omciGetAttriValueFromInst(uint16 meClassId, uint16 instanceId, uint8 attrIndex);

int getGemPortIdFromGemInterTP(IN uint16 classId, IN  uint16 instId, OUT uint16 *gemPortId);
int getAttributeValueByInstId(uint16 meClassId,  uint16 instId, uint8 index, uint8 * value, uint8 len);
int32 getIntervalEndTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag);
int8 getPMHDCntValue(char *value, uint8 *meInstantPtr,  omciAttriDescript_ptr omciAttribute, uint8 flag);
int updateOmci63365HwNatRule(void);

int handleMeInstByOtherMe(uint16 mainClassId, uint16 instId, uint8 action);

int clearXponRule(void);
int resetOmciMib(void);


int delayOmciCommitNode(char* nodeName);
int addOmciCommitNodelist(char* nodeName);
int omciCommitNodeListTimeout(void);

extern AttributeFunc generalGetFunc;
extern AttributeFunc generalGetSetFunc;
extern AttributeFunc performanceCntFunc;
extern AttributeFunc generalGetIntervalEndTimeFunc;
#endif
