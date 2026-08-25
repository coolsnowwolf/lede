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
	omci.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/6/28	Create
*/
#ifndef _OMCI_H_
#define _OMCI_H_


#include <semaphore.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//#include <linux/ipc.h>
//#include <linux/msg.h>
#include <fcntl.h>
#include "omci_types.h"
#include "omci_me.h"
#include "omci_timer.h"
#include "omci_external_msg.h"
#include "omci_message_handle.h"


/************************************************************************
*                        			MACRO							
************************************************************************
*/

#ifndef GNU_PACKED 
#define GNU_PACKED  __attribute__ ((packed))
#endif // GNU_PACKED //

#define MODULE_VERSION_OMCI	("1.0")
#define PONCOMMON_ATTR_XPON_PHY_STATUS	"phyStatus"
#define PONCOMMON_XPON_PHY_DOWN			"down"
#define PONCOMMON_XPON_PHY_GPON_DOWN	"gpon_phy_down"
#define PONCOMMON_XPON_PHY_GPON_UP		"gpon_phy_up"
#define PONCOMMON_XPON_PHY_EPON_DOWN	"epon_phy_down"
#define PONCOMMON_XPON_PHY_EPON_UP		"epon_phy_up"

#define PONCOMMON_ATTR_DATA_TRAFFIC_STA  	"trafficStatus"
#define ONU_WAN_DATA_LINK_UP 				"up"
#define ONU_WAN_DATA_LINK_DOWN 				"down"

#define PON_LINK_CFG_ATTR_XPON_MODE  	"Mode"
#define PONCOMMON_ATTR_XPON_LINK_STA  	"LinkSta"
#define OMCI_GET_INSTANC_BY_ATTR_MAX_NUM  10


/************************************************************************
*                        			struct							
************************************************************************
*/


typedef struct omci_s
{
	uint16 version;
	int meTableSize; /*the value is calculated at initilized stage.*/
	omciManageEntity_t *omciTable_p;
	omciCapablity_t omciCap;
	uint8 omciDataSync;//call omciDataSyncSeqIncret to increase it
	uint8 omciAlarmSeq;//   should to 1~255	; when OMCI reset, the omciAlarmSeq=0
	uint8 onuAdminState; //ONU-G administrator state
	uint16 onuId;
}omci_t,*omci_ptr;



typedef struct tableAttriNode_s{
	struct tableAttriNode_s *next;
	uint16 classId;
	uint16 instId;
	uint16 seqNum;//from 0 on
	uint8 attributeName[32];
	uint32 totalLength;
	uint32 tableLength;
	uint8 *tableContant;
	int tmrId;
}tableAttriNode_t , *tableAttriNode_ptr;




typedef struct tableAttri_s{
	pthread_mutex_t tableAttriLock;
	tableAttriNode_t *tableList;
}tableAttri_t, *tableAttri_ptr;

//keep olt_type_enum defined same as modules/private/xpon/inc/common/drv_global.h
typedef enum{
	PON_OLT_FIBERHOME_551601 = 1,
	PON_OLT_DASAN ,

	/* add new olt type id before here */
}PON_OltType_t;

typedef struct {
	char *oltName;
	PON_OltType_t  oltId;
}PON_OltTypeInfo_t;

extern int oltTypeId;
#define isFiberhome_551601  (oltTypeId == PON_OLT_FIBERHOME_551601)
#define isDasan			(oltTypeId == PON_OLT_DASAN)

OUT int omciPktSend( IN omciPayload_t *omciPayLoad, IN uint8 msgDst );
OUT omciMeInst_t *omciGetInstanceByMeId(IN omciManageEntity_ptr me_p ,  IN uint16 meId);
void omcidbgPrintf(uint8 debugLevel, char *fmt,...);
int inetOpenRawsock(char *iface, int protocol);
int omciGetMeTableSize(void);
uint8 omciGetMsgType(omciPayload_t *msg);
omciManageEntity_t *omciGetMeByClassId(uint16 classId);
OUT int omciDumpPkt(omciPayload_t * txPkt);
int omciSendAlarmMsg(uint32 alarmId, uint16 instanceId, uint8 *alarmContent);
#ifdef TCSUPPORT_OMCI_EXTENDED_MSG
int omciSendAlarmExtendMsg(uint32 alarmId, uint16 instanceId, uint8 *alarmContent);
#endif
int omciArcTmOut(uint32 meId);
uint8 omciGetAttriNumOfMe(omciManageEntity_t *me_p);
OUT uint8 *omciGetInstAttriByName(IN omciMeInst_t *inst_p , IN uint8 *attriName, OUT uint16 *length);
OUT uint8 *omciGetInstAttriByMask(IN omciMeInst_t *inst_p , IN uint16 attriMask, OUT uint16 *length);
OUT omciTableAttriValue_t * omciAddTableValueToInst(IN omciMeInst_t *inst_p , IN uint8 attriIndex, IN uint32 tableValueLength, IN uint8 *TableValue_p);
OUT int omciDelTableValueOfInst(IN omciMeInst_t *inst_p , IN uint8 attriIndex);
OUT omciTableAttriValue_t *omciGetTableValueByIndex(IN omciMeInst_t *inst_p , IN uint8 attriIndex);
OUT int omciAddTableContent(IN uint16 meClassId , IN uint16 instId, IN uint8 *table_ptr, IN uint32 tableLength ,IN uint8 *attributeName);
OUT uint32 omciGetTableContent(IN uint16 meClass, IN uint16 instId, IN uint8 *attributeName, IN uint32 length , OUT uint8 *outBuf, OUT uint16 *seqNum);
OUT int omciGetInstByAttriValue(IN uint16 classId , IN uint8 *attriName,  IN uint8 *value, OUT omciMeInst_t **instList, IN uint8 listLen);
omciMeInst_t *omciAllocInstance(omciManageEntity_t *me);
int omciFreeInstance(omciMeInst_t *inst);
int omciDelInstance(omciManageEntity_ptr me , omciMeInst_t *inst_p);
int omciAddInstanceToTail(omciManageEntity_ptr me, omciMeInst_t *newInst);
int omciArcEnable(omciMeInst_t *inst , uint8 interval );
int omciArcDisable(omciMeInst_t *inst );

int omciDataSyncSeqIncret(void);
int setOmciDataSyncSeq(uint8 value);
uint8 getOmciDataSyncSeq(void);
extern omci_t omci;
extern omciSystemCapablity_ptr pSystemCap;
extern omciOnuCapablity_ptr	pOnuCap;
extern omciAniCapablity_ptr	pAniCap;

int restartcommitNodeTimer(void);
int omciRxHandle(omciPayload_t * omciPkt);
#endif // _OMCI_H_

