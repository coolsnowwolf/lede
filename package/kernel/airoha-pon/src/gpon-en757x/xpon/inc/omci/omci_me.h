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
	omci_me.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/6/28	Create
*/


#ifndef _OMCI_ME_H_
#define _OMCI_ME_H_

#include "omci_types.h"
#include <pthread.h>
#include "omci_external_msg.h"
/************************************************************************
*                        			MACRO							
************************************************************************
*/
#ifndef GNU_PACKED
#define GNU_PACKED __attribute__ ((packed))
#endif//GNU_PACKED

#if !defined __UCLIBC__
#define MAX_STACK_SIZE 131072
#else
#define MAX_STACK_SIZE 16384
#endif

#define BASET101001000_SLOT		0x01	/*47	10/100/1000BaseT*/
#define BASET10100_SLOT			0x02	/*24	10/100BaseT*/
#define POTS_SLOT 					0x04	/*32	POTS*/
#define VEIP_SLOT 					0x0a	/*48 	VEIP*/
#define GPON_SLOT 					0x80	/*248 	GPON24881244*/

/************************************************************************
*                        			struct							
************************************************************************
*/

typedef struct omciAttriDescript_s
{
    uint8   attriIndex;
    uint8   attriName[32];
    uint16  attriLen;//0:un-inited;    0: variable length
    uint8   attriAccess;//R, W, R/W.....
    uint8   format;//POINTTER;BIT_FIELD...
    uint32  lowerLimit;
    uint32  uppererLimit;
    uint32  bitField;
    uint16 *codePointTbl;// point to the code point table buffer
    uint8   support;
    uint32  avcId;
    struct _Attributefunc *func;
}omciAttriDescript_t,*omciAttriDescript_ptr;

typedef int(*GetValueInfoFun)(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);/*if the ME is PM node, flag=0, get history data; flag=1, get current data*/
typedef int(*SetValueInfoFun)(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag);/*flag, 0:create action, 1:set action*/

typedef struct _Attributefunc
{
	GetValueInfoFun getValueInfo;
	SetValueInfoFun setValueInfo;
}AttributeFunc;


typedef struct omciAlarm_s
{
	uint8 alarmBitmap[MT_OMCI_ALARM_BITMAP_LENGTH];	
}omciAlarm_t,*omciAlarm_ptr;

#define VEIP_INST_ID 1

#define OMCI_ALM 	0
#define OMCI_NALM	1

typedef struct omciTableAttriValue_s{
	struct omciTableAttriValue_s *next;
	uint8 attriIndex;
	uint32 tableSize;
	uint8 *tableValue_p;
}omciTableAttriValue_t, *omciTableAttriValue_p;

typedef struct downloadSection_s
{
	struct downloadSection_s *nextSection;
	uint8 *secContent;
	uint16 secLength;	
	uint8 secNum;	/*section number*/
}downloadSection_t,*downloadSection_ptr;

typedef struct downloadWindow_s
{
	struct downloadWindow_s *nextWindow;
	uint32 windowNum;	/*window number*/
	downloadSection_t *sectionList_p;
}downloadWindow_t,*downloadWindow_ptr;

typedef struct downloadImage_s
{
	uint32 totalSize;
	uint32 currentSize;
	downloadWindow_t *currentWindowPtr;
	downloadWindow_t *preWindowPtr;
	downloadWindow_t *windowList;
}downloadImage_t,*downloadImage_ptr;


typedef struct omciMeInst_s
{
	struct omciMeInst_s *next;
	uint16 classId;
	uint8 *attributeVlaue_ptr;//pointer to the memory of instance data
	omciAlarm_t *meAlarm;//alarm bit map	
	uint16 deviceId;//device id for AVC,alarm,TCA,test report
	//for ARC use
	uint8 interval;//0~255 minites; 0: no arc; 255: not timeout
	uint8 arcState;//ALM:0 ;NALM: 1
	int arcTm;//record ARC timer
	//to store table value
	omciTableAttriValue_t *tableValueList;
	uint8 *otherInfo; //for some me maintain information
}omciMeInst_t,*omciMeInst_ptr;


typedef struct omciInst_s{
	pthread_mutex_t omciInstLock;
	omciMeInst_t *omciMeInstList;
}omciInst_t, *omciInst_p;


/* for alarm define */
typedef struct alarm_id_map_entry_s{
	uint32 alarmId;
}alarm_id_map_entry_t;

#define	NORMAL								(0)
#define	UNNEED_MIB_UPLOAD 				(1<<0)			/*when the bit is setted, the me is not needed to upload in MIB UPLOAD action*/
#define 	IS_CLASSICAL_PM_ME				(1<<1)			/*when the bit is setted, the me is classical performance monitor manage entity*/
#define	IS_EXTENDED_PM_ME				(1<<2)			/*when the bit is setted, the me is extended performance monitor manage entity*/

typedef struct omciManageEntity_s
{
	uint16 omciMeClassId;
	uint8 omciMeName[32];
	uint8 access;// OMCI_ME_ACCESS_OLT.....
	uint8 support;//
	/*function define*/
	int (*omciMeInitFunc)(struct omciManageEntity_s *omciManageEntity_p);
	int (*omciActionFunc[MT_OMCI_MSG_TYPE_MAX])(/*uint16 omciMeIndex,*/ uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t *omciPayLoad, uint8 msgSrc);

	/*ATTRIBUTE define*/
	omciAttriDescript_t *omciAttriDescriptList;	

	/*instant define*/
	omciInst_t omciInst;	
	/*ALARM */
	alarm_id_map_entry_t *alarmIdMapTable;
	uint32	specficFlag; 						/*consider some specfic case, when the bit is setted, the me is belong to this case*/
}omciManageEntity_t,*omciManageEntity_ptr;

typedef struct omciInterCreateME_s
{
	uint16 index;
	uint16 mainClassId;
	uint16 secondClassId;/*the secondClassId ME depend on mainClassId ME instance*/
}omciInterCreateME_t,*omciInterCreateME_Ptr;



#define UNIT_TYPE_MIXED_SERVICES_EQUIPMENT 	45
#define UNIT_TYPE_BASET_10100					24
#define UNIT_TYPE_BASET_101001000				47
#define UNIT_TYPE_VEIP							48
#define UNIT_TYPE_GPON24881244				248
#define UNIT_TYPE_POTS							32

#define MAP_PORTS_NUM 8
typedef struct omciSystemCapablity_s
{
	uint8 onuType;
	uint8 gponNum;
	uint8 feNum;
	uint8 geNum;
	uint8 veipNum;
	uint8 potsNum;
}omciSystemCapablity_t, *omciSystemCapablity_ptr;

#define OMCI_SN_LENS 							8
#define OMCI_VERSION_LENS						14
#define OMCI_EQUIPMENTID_LENS					20
typedef struct omciOnuCapablity_s
{
	char		sn[OMCI_SN_LENS] ;	/* onu serial number, the default is MTKG00000001 */
	char		version[OMCI_VERSION_LENS];
	char 	equipmentId[OMCI_EQUIPMENTID_LENS];
	uint16 	vendorProCode;
	uint8 	omccVersion;
	uint8 	tmOption;
	uint8 	onuSurvivalTime;
	uint8 	extendedTcLayer;
	uint8 	securityCap;
	uint8  	totalTSNum;
	uint16 	totalPQNum;
	uint16	totalGEMPortNum;
	uint16	connectivityCap;
	uint16	qoSConfFlex;
	char    linkMode;
	char    VoIPbyTR069;
}omciOnuCapablity_t, *omciOnuCapablity_ptr;

typedef struct omciAniCapablity_s
{
	uint8	srIndication;
	uint8	piggybackDBA;
	uint16 	totalTcontNum;
	uint16	gemBlockLength;
	uint16	onuResponseTime;
}omciAniCapablity_t, *omciAniCapablity_ptr;

typedef struct omciCapablity_s
{
	omciSystemCapablity_t systemCap;
	omciOnuCapablity_t	onuCap;
	omciAniCapablity_t		aniCap;
}omciCapablity_t,*omciCapablity_ptr;

typedef struct omciNodeInfo_s
{
	char *nodeName;
	char *attrName;
	char *value;
}omciNodeInfo_t; 

/**********************************************
*************Function declare**************
*******************/
int omciMeTableInit(void);
int omciGeMeAlarmSize(omciManageEntity_t *meTmp);
int  omciInitInst(void);
int omciInitCapability(void);
extern omciManageEntity_t  omciMeTable[];
extern omciInterCreateME_t interCreateMETab[];
extern omciOnuCapablity_ptr	pOnuCap;


#if 0
int omciMeInitForOnuG(omciManageEntity_t *omciManageEntity_p);
int omciGetActionFor_ONU_G(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciSetActionFor_ONU_G(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciRebootActionFor_ONU_G(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciTestActionFor_ONU_G(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciSyncTimeActionFor_ONU_G(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciMeInitFor_IPHostCfgData(omciManageEntity_t *omciManageEntity_p);
int omciGetActionFor_IPHostCfgData(/*uint16 MeIndex, */ uint16 meClassId , uint8 *meInstant_ptr, omciPayload_t *omciPayLoad, uint8 msgSrc);
int omciSetActionFor_IPHostCfgData(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciTestActionFor_IPHostCfgData(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciCreateActionFor_IPHostCfgData(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
int omciDeleteActionFor_IPHostCfgData(/*uint16 MeIndex, */uint16 meClassId , uint8 *meInstant_ptr, omciPayload_ptr omciPayLoad, uint8 msgSrc);
#endif

typedef struct __pthreadArgType{
		uint16 tranCorrelatId;
		uint8 msgContent[OMCI_BASELINE_CONT_LEN];
		uint8 msgFormat;		//0x0A:baseline format   0X0B:extended format		
		uint16 meClassId;
		uint16 meInstId;
}pthreadArgType;

typedef struct testSupport
{
    uint16 meClassId;
    uint8 (*isSupport)(pthreadArgType *);
    int (*testAction)(pthreadArgType *);
}testSupport_s,*testSupport_ptr;

#endif // _OMCI_ME_H_

