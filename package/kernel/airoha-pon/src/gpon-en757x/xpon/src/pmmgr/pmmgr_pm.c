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
	omci.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	andy.Yi		2012/7/5	Create
*/
#include <stdio.h>
#include "pmmgr_pm.h"
#include "../lib/libtcapi.h"
#include "../omci/omci_external_msg.h"
#if !defined(TCSUPPORT_CT) 
#include "../cfg_manager/cfg_manager.h"
#endif
#ifdef TCSUPPORT_MT7530_SWITCH_API
#include "../ethcmd/libmii.h"
#include "../lib/mtkswitch_api.h"
#endif
#include "api/mgr_api.h"
#include <stddef.h>

extern mxml_node_t **gTreePtr;


#ifndef offsetof
#define offsetof(type, member) ((long) &((type *) 0)->member)
#endif


/*******************************************************************************************************************************
switch API

********************************************************************************************************************************/
#ifndef TCSUPPORT_MT7530_SWITCH_API
int macMT7530GetPortTxSuccFramesCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRxDropFramesCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}

int macMT7530GetPortRxLackDropPktCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRxOctetsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRxPktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRxBroadPktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRxMultiPktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRxCRCPktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int  macMT7530GetPortRxUnderSizePktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRxOverSizePktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRx64PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRx127PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRx255PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRx511PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRx1023PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortRx1518PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTxOctetsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTxPktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTxBroadPktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTxMultiPktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTxCRCPktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTx64PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTx127PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTx255PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTx511PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTx1023PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
int macMT7530GetPortTx1518PktsCnt(uint8 port, uint32 * cnt){
	*cnt = 2;
	return 0;
}
#endif
/*******************************************************************************************************************************
genera function

********************************************************************************************************************************/
/*******************************************************************************************
**function name
	findRowBufFirstStr
**description:
	find the first string in row buffer,
	find the remain buffer in row buffer.
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
	rowBuf: row buffer
	fromLen: from rowBuf+fromLen
	firstStr: first string
********************************************************************************************/
int findRowBufFirstStr(char * rowBuf, uint32 * fromLen, char * firstStr){
	char *rowPtr = NULL;	
	char buf[MAX_PM_BUFFER_SIZE] = {0};
	int len = 0;
	uint32 currLen = 0;
	int ret = -1;

	if((rowBuf == NULL) || (firstStr == NULL)){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n findRowBufFirstStr  fail ");
		goto end;
	}

//	pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n findRowBufFirstStr->rowBuf=%s, fromLen=%d",rowBuf, *fromLen);
	
	memset(buf, 0, MAX_PM_BUFFER_SIZE);
	currLen = *fromLen;

//	pmmgrPrintf(PMMGR_DEBUG_TRACE,"\r\n findMacBridgeFirstStr->fromLen =%d",*fromLen);
	
	rowPtr = rowBuf+currLen;
	/*strip the prefix space*/
	while(*rowPtr != '\0'){
		if((*rowPtr != ' ') && (*rowPtr != '\t'))
			break;
		currLen++;
		rowPtr++;
	}
	/*get first Str*/
	if(*rowPtr == '\0'){
		goto end;
	}
	len = 0;
	while(*rowPtr != '\0'){		
		if((*rowPtr == ' ') || (*rowPtr == '\t')){	
			break;
		}
		if(len >=  MAX_MAC_BRIDGE_PORT_ITEM_LEN){
			goto end;
		}
		buf[len++] = *rowPtr;
		currLen++;
		rowPtr++;
	}
	buf[MAX_PM_BUFFER_SIZE-1] = '\0';

	strcpy(firstStr, buf);
//	pmmgrPrintf(PMMGR_DEBUG_TRACE,"\r\n findMacBridgeFirstStr->firstStr =%s",firstStr);
	* fromLen = currLen;
//	pmmgrPrintf(PMMGR_DEBUG_TRACE,"\r\n findMacBridgeFirstStr->currLen =%d",currLen);
	
	ret =  0;
end:
//	pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n findMacBridgeFirstStr->len =%d",len);

	return ret;

}

pmmgrPmAttriDescript_t pmmgrPmAttriDescriptFecPM[] ={
//{"correctedBytes", 		4, 0, OMCI_TCA_ID_FEC_PM_CORRECTEDBYTES, 			NULL},
{"correctedBytes", 		4, 0, NO_TCA, 			NULL},
//{"correctedCodeWords", 	4, 0, OMCI_TCA_ID_FEC_PM_CORRECTEDODEWORDS, 		NULL},
{"correctedCodeWords",	4, 0, NO_TCA, 		NULL},
//{"uncorrectableCodeWords", 4, 0, OMCI_TCA_ID_FEC_PM_UNCORRECTABLECODEWORDS, NULL},
{"uncorrectableCodeWords", 4, 0, NO_TCA, NULL},
{"totalCodeWords", 		4, 0, NO_TCA, 							   			NULL},
//{"FECSeconds", 			2, 0, OMCI_TCA_ID_FEC_PM_FECSECONDS, 				NULL},
{"FECSeconds",			2, 0, NO_TCA,				NULL},
{"", 						0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForFecPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	FECPmcounters_t pm;
	
	if (me_p == NULL)
	{
		printf("pmmgrPmInitForFecPM: me_p == NULL\n");
		return -1;
	}

	
	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(FECPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.correctedBytes - (uint8*)&pm.correctedBytes;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForFecPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.correctedCodeWords - (uint8*)&pm.correctedBytes;

	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForFecPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.uncorrectableCodeWords -(uint8*)&pm.correctedBytes;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForFecPM:attri_p->offset = %x\n", attri_p->offset);
	attri_p++;
	attri_p->offset = (uint8*)&pm.totalCodeWords -(uint8*)&pm.correctedBytes;

	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForFecPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.FECSeconds -(uint8*)&pm.correctedBytes;

	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForFecPM:attri_p->offset = %x\n", attri_p->offset);
	return 0;
}

pmmgrPmAttriDescript_t pmmgrPmAttriDescriptGemPortCtpPM[] ={
{"transmitGEMFrames", 			4, 0, NO_TCA, 			NULL},
{"receiveGEMFrames", 			4, 0, NO_TCA, 			NULL},
{"receivePayloadBytes", 		8, 0, NO_TCA,				NULL},
{"transmitPayloadBytes", 		8, 0, NO_TCA, 			NULL},
{"", 						0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForGemPortCtpPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	GEMPort_CTP_Pmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "pmmgrPmInitForGemPortCtpPM: me_p == NULL\n");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(GEMPort_CTP_Pmcounters_t);

/*3.calculate the me attribute number
*/
	attri_p = me_p->pmmgrAttriDescriptList;
	attri_p->offset = (uint8*)&pm.transmitGEMFrames - (uint8*)&pm.transmitGEMFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortCtpPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.receiveGEMFrames - (uint8*)&pm.transmitGEMFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortCtpPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.receivePayloadBytes -(uint8*)&pm.transmitGEMFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortCtpPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.transmitPayloadBytes -(uint8*)&pm.transmitGEMFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortCtpPM:attri_p->offset = %x\n", attri_p->offset);

	return 0;
}



pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEnergyConsumPM[] ={
{"dozeTime", 					4, 0, NO_TCA, 			NULL},
{"cyclicSleepTime", 			4, 0, NO_TCA, 			NULL},
{"energyConsumed", 			4, 0, NO_TCA,				NULL},
{"", 							0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForEnergyConsumPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	EnergyConsumed_Pmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "pmmgrPmInitForEnergyConsumPM: me_p == NULL\n");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(EnergyConsumed_Pmcounters_t);

/*3.calculate the me attribute number
*/
	attri_p = me_p->pmmgrAttriDescriptList;
	attri_p->offset = (uint8*)&pm.dozeTime- (uint8*)&pm.dozeTime;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEnergyConsumPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.cyclicSleepTime- (uint8*)&pm.dozeTime;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEnergyConsumPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.energyConsumed-(uint8*)&pm.dozeTime;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEnergyConsumPM:attri_p->offset = %x\n", attri_p->offset);
	
	return 0;
}

//9.5.2 Ethernet performance monitoring history data
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptPptpEthernetUniPM[] ={
{"FCSErrors", 					4, 0, OMCI_TCA_ID_ETHERNET_PM_2_FEC_ERRORS, 						NULL},
{"excessiveCollisionCnts", 		4, 0, OMCI_TCA_ID_ETHERNET_PM_2_EXCESSIVE_COLLISION_CNT, 		NULL},
{"lateCollisionCnts", 			4, 0, OMCI_TCA_ID_ETHERNET_PM_2_LATE_COLLISION_CNT, 				NULL},
{"framesTooLong", 			4, 0, OMCI_TCA_ID_ETHERNET_PM_2_FRAMES_TOO_LONG, 				NULL},
{"bufferOverFlowOnRcv", 		4, 0, OMCI_TCA_ID_ETHERNET_PM_2_BUFFER_OVERFLOWS_ON_RECEIVE, 	NULL},
{"singleCollisionFrameCnts", 		4, 0, OMCI_TCA_ID_ETHERNET_PM_2_SINGLE_COLLISION_FRAME_CNT, 		NULL},
{"multiCollisionFrameCnts", 		4, 0, OMCI_TCA_ID_ETHERNET_PM_2_MULTI_COLLISION_FRAME_CNT, 		NULL},
{"deferredTransmissionCnts", 	4, 0, OMCI_TCA_ID_ETHERNET_PM_2_DEFERRED_TRANSMISSION_CNT, 		NULL},
{"alignmentErrorCnts", 			4, 0, OMCI_TCA_ID_ETHERNET_PM_2_ALIGNMENT_ERROR_CNT, 			NULL},
{"", 							0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForPptpEthernetUniPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	PPTPEthernetUNIPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForPptpEthernetUniPM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(PPTPEthernetUNIPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.FCSErrors - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.excessiveCollisionCnts - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.lateCollisionCnts - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.framesTooLong - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.bufferOverFlowOnRcv - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.singleCollisionFrameCnts - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.multiCollisionFrameCnts - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.deferredTransmissionCnts - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.alignmentErrorCnts - (uint8*)&pm.FCSErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUniPM:attri_p->offset = %x\n", attri_p->offset);
	
	return 0;
}

//9.5.3 Ethernet performance monitoring history data 2
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptPptpEthernetUni2PM[] ={
{"pppoeFilerCnt", 			4, 0, OMCI_TCA_ID_ETHERNET_PM_3_PPPOE_FILTERED_FRAME_CNT, 	NULL},
{"", 						0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForPptpEthernetUni2PM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	PPTPEthernetUNI2Pmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForPptpEthernetUni2PM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(PPTPEthernetUNI2Pmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.pppoeFilterCnt - (uint8*)&pm.pppoeFilterCnt;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni2PM:attri_p->offset = %x\n", attri_p->offset);

	return 0;
}


//9.5.4 Ethernet performance monitoring history data 3
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptPptpEthernetUni3PM[] ={
{"dropEvents", 			4, 0, OMCI_TCA_ID_ETHERNET_PM_4_DROP_EVENTS, 		NULL},
{"octets", 				4, 0, NO_TCA, 										NULL},
{"packets", 				4, 0, NO_TCA, 										NULL},
{"broadcastPkts", 			4, 0, NO_TCA, 										NULL},
{"multicastPkts", 			4, 0, NO_TCA, 										NULL},
{"undersizePkts", 			4, 0, OMCI_TCA_ID_ETHERNET_PM_4_UNDERSIZE_PACKETS, 	NULL},
{"fragments", 				4, 0, OMCI_TCA_ID_ETHERNET_PM_4_FRAGMENTS, 			NULL},
{"jabbers", 				4, 0, OMCI_TCA_ID_ETHERNET_PM_4_JABBERS, 			NULL},
{"pkts64Octets", 			4, 0, NO_TCA, 										NULL},
{"pkts65To127Octets", 		4, 0, NO_TCA, 										NULL},
{"pkts128To255Octets", 	4, 0, NO_TCA, 										NULL},
{"pkts256To511Octets", 	4, 0, NO_TCA, 										NULL},
{"pkts512To1023Octets", 	4, 0, NO_TCA, 										NULL},
{"pkts1024To1518Octets", 	4, 0, NO_TCA, 										NULL},
{"", 						0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForPptpEthernetUni3PM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	PPTPEthernetUNI3Pmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForPptpEthernetUni3PM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(PPTPEthernetUNI3Pmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.dropEvents - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.octets - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.packets - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.broadcastPkts - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.multicastPkts - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.undersizePkts - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.fragments - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.jabbers - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts64Octets- (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts65To127Octets - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts128To255Octets - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts256To511Octets - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts512To1023Octets - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts1024To1518Octets - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForPptpEthernetUni3PM:attri_p->offset = %x\n", attri_p->offset);
	
	return 0;
}

// 9.3.32 Ethernet frame extended PM
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetFrameExtendedPM[] ={
{"dropEvents",				4, offsetof(EthernetFrameExtendedPmcounters_t,dropEvents),			  OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_DROP_EVENTS,		NULL},
{"octets",					4, offsetof(EthernetFrameExtendedPmcounters_t,octets),				  NO_TCA,													NULL},
{"frames",					4, offsetof(EthernetFrameExtendedPmcounters_t,frames),				  NO_TCA,													NULL},
{"broadcastFrames", 		4, offsetof(EthernetFrameExtendedPmcounters_t,broadcastFrames), 	  NO_TCA,													NULL},
{"multicastFrames", 		4, offsetof(EthernetFrameExtendedPmcounters_t,multicastFrames), 	  NO_TCA,													NULL},
{"crcerroredFrames",		4, offsetof(EthernetFrameExtendedPmcounters_t,crcerroredFrames),	  OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_CRC_ERROR_FRAMES,	NULL},
{"undersizeFrames", 		4, offsetof(EthernetFrameExtendedPmcounters_t,undersizeFrames), 	  OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_UNDERSIZE_FRAMES,	NULL},	
{"oversizeFrames",			4, offsetof(EthernetFrameExtendedPmcounters_t,oversizeFrames),		  OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_OVERSIZE_FRAMES,	NULL},
{"frames64Octets",			4, offsetof(EthernetFrameExtendedPmcounters_t,frames64Octets),		  NO_TCA,													NULL},
{"frames65To127Octets", 	4, offsetof(EthernetFrameExtendedPmcounters_t,frames65To127Octets),   NO_TCA,													NULL},
{"frames128To255Octets",	4, offsetof(EthernetFrameExtendedPmcounters_t,frames128To255Octets),  NO_TCA,													NULL},
{"frames256To511Octets",	4, offsetof(EthernetFrameExtendedPmcounters_t,frames256To511Octets),  NO_TCA,													NULL},
{"frames512To1023Octets",	4, offsetof(EthernetFrameExtendedPmcounters_t,frames512To1023Octets), NO_TCA,													NULL},
{"frames1024To1518Octets",	4, offsetof(EthernetFrameExtendedPmcounters_t,frames1024To1518Octets),NO_TCA,													NULL},
{"",						0, 0, NO_TCA, NULL}
};


int pmmgrPmInitForEthernetFrameExtendedPM(pmmgrPmMe_t *me_p)
{
	int attriIndex = 0;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForEthernetFrameExtendedPM: me_p == NULL");
		return -1;
	}
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(EthernetFrameExtendedPmcounters_t);

	return 0;
}

// 9.3.34 Ethernet frame extended PM 64-bit
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetFrameExtendedPM64bit[] ={
{"dropEvents", 				8, offsetof(EthernetFrameExtendedPm64bitcounters_t,dropEvents), 			OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_64BIT_DROP_EVENTS, 		NULL},
{"octets", 					8, offsetof(EthernetFrameExtendedPm64bitcounters_t,octets), 				NO_TCA, 															NULL},
{"frames", 					8, offsetof(EthernetFrameExtendedPm64bitcounters_t,frames), 				NO_TCA, 															NULL},
{"broadcastFrames", 		8, offsetof(EthernetFrameExtendedPm64bitcounters_t,broadcastFrames),		NO_TCA, 															NULL},
{"multicastFrames", 		8, offsetof(EthernetFrameExtendedPm64bitcounters_t,multicastFrames), 		NO_TCA, 															NULL},
{"crcerroredFrames",		8, offsetof(EthernetFrameExtendedPm64bitcounters_t,crcerroredFrames), 		OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_64BIT_CRC_ERROR_FRAMES,	NULL},
{"undersizeFrames", 		8, offsetof(EthernetFrameExtendedPm64bitcounters_t,undersizeFrames), 		OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_64BIT_UNDERSIZE_FRAMES, 	NULL},	
{"oversizeFrames", 			8, offsetof(EthernetFrameExtendedPm64bitcounters_t,oversizeFrames), 		OMCI_TCA_ID_ETHERNET_FRAME_EXTENDED_PM_64BIT_OVERSIZE_FRAMES, 	NULL},
{"frames64Octets", 			8, offsetof(EthernetFrameExtendedPm64bitcounters_t,frames64Octets), 		NO_TCA, 															NULL},
{"frames65To127Octets", 	8, offsetof(EthernetFrameExtendedPm64bitcounters_t,frames65To127Octets), 	NO_TCA, 															NULL},
{"frames128To255Octets", 	8, offsetof(EthernetFrameExtendedPm64bitcounters_t,frames128To255Octets), 	NO_TCA, 															NULL},
{"frames256To511Octets", 	8, offsetof(EthernetFrameExtendedPm64bitcounters_t,frames256To511Octets), 	NO_TCA, 															NULL},
{"frames512To1023Octets", 	8, offsetof(EthernetFrameExtendedPm64bitcounters_t,frames512To1023Octets), 	NO_TCA, 															NULL},
{"frames1024To1518Octets", 	8, offsetof(EthernetFrameExtendedPm64bitcounters_t,frames1024To1518Octets), NO_TCA, 															NULL},
{"", 						8, 0, NO_TCA, NULL}
};

int pmmgrPmInitForEthernetFrameExtendedPM64bit(pmmgrPmMe_t *me_p)
{
	int attriIndex = 0;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForEthernetFrameExtendedPM64bit: me_p == NULL");
		return -1;
	}
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(EthernetFrameExtendedPm64bitcounters_t);

	return 0;
}



#ifdef TCSUPPORT_VOIP
//9.9.12 call control  performance monitoring history data
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptCallCtrlPM[] ={
{"call setup failure",      4, offsetof(CallCtrlPmcounters_t,SetupFailure), 	OMCI_TCA_ID_CALL_CTRL_SETUP_FAIL, 	NULL},
{"call setup timer",        4, offsetof(CallCtrlPmcounters_t,SetupTimer), 		OMCI_TCA_ID_CALL_CTRL_SETUP_TIMER , 	NULL},
{"call terminate failure",  4, offsetof(CallCtrlPmcounters_t,TerminateFailure), OMCI_TCA_ID_CALL_CTRL_TMINAT_FAIL,   NULL},
{"Analog port release",     4, offsetof(CallCtrlPmcounters_t,PortRelease), 		OMCI_TCA_ID_CALL_CTRL_PORT_RELEASE, 	NULL},
{"Analog port OH timer",    4, offsetof(CallCtrlPmcounters_t,PortOffhookTimer), OMCI_TCA_ID_CALL_CTRL_PORT_OH_TIMER, 	NULL},
{"", 					0, 0, NO_TCA, NULL}
};

const char* g_call_ctrl_node[] = 
{
	"CallsSetUpFailed",
    "MaxCallSetUpTime",
    "CallsDropped",
    "OffHookNoDialed",
    "SeizeMin",
};
int pmmgrPmInitForCallCtrlPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	CallCtrlPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForCallCtrlPM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(CallCtrlPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"%s:attri_p->offset = %x\n", __FUNCTION__,attri_p->offset);

	return 0;
}

int getAllDataFromApiForCallCtrlPM(char *data, uint16 portId)
{
	CallCtrlPmcounters_t *pm = NULL;
	CallCtrlPmcounters_t local_cnt = {0};
    uint32 *val = NULL;
    int ret = -1;
    uint16 entryIndex = 0;
    uint8 index = 0;
    char tempBuffer[MAX_PM_BUFFER_SIZE] = {0};
    mxml_node_t * top = *gTreePtr;
    char nodeName[MAX_ARG_NUM][MAX_NODE_NAME];

	if(data == NULL)
		return ret;
   
    memset(nodeName, 0, sizeof(nodeName));
    strncpy(nodeName[0],"InfoVoIP",9);
    
    /* portId is ME instrance Id , for RTP PM history data, the instranceId is linked to PPTP POTS UNI,  
       which is ((POTS_SLOT<<8) | i) */
    entryIndex = (portId & 0xFF) -1;
    snprintf(nodeName[1], MAX_NODE_NAME,"%s%x", "Entry", entryIndex);  
	val = &(local_cnt.SetupFailure);
	for(index = 0; index < ARRAY_SIZE(g_call_ctrl_node);index++,val++)
    {
    	/* get rtp error packet */
	    memset(tempBuffer, 0, MAX_PM_BUFFER_SIZE);
	    if(getAttrNValue(top, nodeName, g_call_ctrl_node[index], tempBuffer,MAX_PM_BUFFER_SIZE) != TCAPI_PROCESS_OK)
        {
	        pmmgrPrintf(PMMGR_DEBUG_ERROR,"\r\n [ERROR] %s tcapi get %s  fail\n",__FUNCTION__,g_call_ctrl_node[index]);
			continue;
	        //goto end;
	    }    
	    *val = atoi(tempBuffer);
    }
	pm = (CallCtrlPmcounters_t *)data;

	memcpy(pm,&local_cnt,sizeof(local_cnt));
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Call Control syscle get current data is %lu %lu %lu %lu %lu\n",
		pm->SetupFailure,pm->SetupTimer,pm->TerminateFailure,pm->PortRelease,pm->PortOffhookTimer);
	
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"------%s display begin----\n", __FUNCTION__); 
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Call Ctrl Data pm->SetupFailure = %lu\n", pm->SetupFailure);
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Call Ctrl Data pm->SetupTimer = %lu\n", pm->SetupTimer);
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Call Ctrl Data pm->TerminateFailure = %lu\n", pm->TerminateFailure); 
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Call Ctrl Data pm->PortRelease = %lu\n", pm->PortRelease);
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Call Ctrl Data pm->PortOffhookTimer = %lu\n", pm->PortOffhookTimer);
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"------%s display end----\n", __FUNCTION__);      
#endif
    ret = 0;
    
    end:
        return ret;
}


//9.9.13 rtp performance monitoring history data
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptRtpDataPM[] ={
{"rtp errors", 			4, 0, OMCI_TCA_ID_RTP_RTP_ERROR, 	NULL},
{"packet loss", 		4, 0, OMCI_TCA_ID_RTP_PACKET_LOSS, 	NULL},
{"maximum jitter",      4, 0, OMCI_TCA_ID_RTP_MAX_JITTER,   NULL},
{"maximum time between RTCP", 		4, 0, OMCI_TCA_ID_RTP_MAX_BET_RTCP, 	NULL},
{"buffer underflows", 	4, 0, OMCI_TCA_ID_RTP_BUFFER_UNDERFLOWS, 	NULL},
{"buffer overflows", 	4, 0, OMCI_TCA_ID_RTP_BUFFER_OVERFLOWS, 	NULL},
{"", 					0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForRTPDataPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	RTPDataPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForRTPDataPM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(RTPDataPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.RtpErrors - (uint8*)&pm.RtpErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForRTPDataPM:attri_p->offset = %x\n", attri_p->offset);

    attri_p++;
    attri_p->offset = (uint8*)&pm.PacketLoss - (uint8*)&pm.RtpErrors;

    attri_p++;
    attri_p->offset = (uint8*)&pm.MaxJitter - (uint8*)&pm.RtpErrors;

    attri_p++;
    attri_p->offset = (uint8*)&pm.MaxTimeBetRTCP - (uint8*)&pm.RtpErrors;

    attri_p++;
    attri_p->offset = (uint8*)&pm.BufferUnderflows - (uint8*)&pm.RtpErrors;

    attri_p++;
    attri_p->offset = (uint8*)&pm.BufferOverflows - (uint8*)&pm.RtpErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForRTPDataPM:attri_p->offset = %x\n", attri_p->offset);

	return 0;
}

int getAllDataFromApiForRTPDataPM(char *data, uint16 portId)
{
	RTPDataPmcounters_t *pm = NULL;

    int ret = -1;
    uint16 entryIndex = 0;
    uint32 RtpErrors = 0;
    uint32 PacketLoss = 0;
    uint32 MaxJitter = 0;
    uint32 RTCPTime = 0;
    uint32 BufferUnderflows = 0;
    uint32 BufferOverflows = 0;
    char tempBuffer[MAX_PM_BUFFER_SIZE] = {0};
    char nodeName[MAX_ARG_NUM][MAX_NODE_NAME];
    char nodeName_char[32] = {0};

	if(data == NULL)
		return ret;
   
    memset(nodeName, 0, sizeof(nodeName));
    strcpy(nodeName[0], "InfoVoIP");
    
    /* portId is ME instrance Id , for RTP PM history data, the instranceId is linked to PPTP POTS UNI,  
       which is ((POTS_SLOT<<8) | i) */
    entryIndex = (portId & 0xFF) -1;
    sprintf(nodeName[1], "%s%x", "Entry", entryIndex);  

    memset(nodeName_char, 0, sizeof(nodeName_char));
    sprintf(nodeName_char, "%s_%s", nodeName[0], nodeName[1]);
    /* get rtp error packet */
    memset(tempBuffer, 0, 64);
    if(tcapi_nget(nodeName_char, OMCI_RTP_PM_RTPERROR, tempBuffer,sizeof(tempBuffer)) != TCAPI_PROCESS_OK){
        pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForRTPDataPM->RtpErrors ,  fail");
        goto end;
    }    
    RtpErrors = atoi(tempBuffer);
    
    /* get packet loss rate */
    memset(tempBuffer, 0, 64);
    if(tcapi_nget(nodeName_char, OMCI_RTP_PM_PacketLoss, tempBuffer, sizeof(tempBuffer)) != TCAPI_PROCESS_OK){
        pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForRTPDataPM->Packetslost ,  fail");
        goto end;
    }    
    PacketLoss = atoi(tempBuffer);

    /* get max jitter */
    memset(tempBuffer, 0, 64);
    if(tcapi_nget(nodeName_char, OMCI_RTP_PM_MaxJitter, tempBuffer, sizeof(tempBuffer)) != TCAPI_PROCESS_OK){
        pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForRTPDataPM->MaxJitter ,  fail");
        goto end;
    }
    MaxJitter = RTP_TIMESTAMP_UNIT*atoi(tempBuffer);

    /* get max RTC interval */
    memset(tempBuffer, 0, 64);
    if(tcapi_nget(nodeName_char, OMCI_RTP_PM_MaxTimeBetRTCP, tempBuffer, sizeof(tempBuffer)) != TCAPI_PROCESS_OK){
        pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForRTPDataPM->MaxTimeBetRTCP ,  fail");
        goto end;
    }
    RTCPTime = atoi(tempBuffer);

    /* get buf under flow */
    memset(tempBuffer, 0, 64);
    if(tcapi_nget(nodeName_char, OMCI_RTP_PM_BUFUNDERFLOW, tempBuffer,sizeof(tempBuffer)) != TCAPI_PROCESS_OK){
        pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForRTPDataPM->BufferUnderflows ,  fail");
        goto end;
    }
    BufferUnderflows = atoi(tempBuffer);

    /* get buf over flow */
    memset(tempBuffer, 0, 64);
    if(tcapi_nget(nodeName_char, OMCI_RTP_PM_BUFOVERFLOW, tempBuffer, sizeof(tempBuffer)) != TCAPI_PROCESS_OK){
        pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForRTPDataPM->BufferOverflows ,  fail");
        goto end;
    }
    BufferOverflows = atoi(tempBuffer);

	pm = (RTPDataPmcounters_t *)data;

	pm->RtpErrors= RtpErrors;
	pm->PacketLoss= PacketLoss;
	pm->MaxJitter= MaxJitter;
	pm->MaxTimeBetRTCP= RTCPTime;
	pm->BufferUnderflows= BufferUnderflows;
	pm->BufferOverflows= BufferOverflows;

#ifdef PMMGR_DEBUG
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForRTPDataPM pm->RtpErrors = %lu\n", pm->RtpErrors);
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForRTPDataPM pm->PacketLoss = %lu\n", pm->PacketLoss);
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForRTPDataPM pm->MaxTimeBetRTCP = %lu\n", pm->MaxJitter); 
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForRTPDataPM pm->MaxJitter = %lu\n", pm->MaxTimeBetRTCP);
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForRTPDataPM pm->BufferUnderflows = %lu\n", pm->BufferUnderflows);
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForRTPDataPM pm->BufferOverflows = %lu\n", pm->BufferOverflows);      
#endif

    ret = 0;
    
    end:
        return ret;

}

//9.9.14 SIP agent performance monitoring history data
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptSipAgentPM[] ={
{"SIPAMD Transactions",                 4, offsetof(SipAgentPmcounters_s_t,Transactions), NO_TCA,                                   NULL},
{"SIPAMD rx invite req", 			    4, offsetof(SipAgentPmcounters_s_t,RxInviteReqs), OMCI_TCA_ID_SIPAMD_RX_INV_REQ, 	        NULL},
{"SIPAMD rx invite req retrans", 		4, offsetof(SipAgentPmcounters_s_t,RxInviteRetrans), OMCI_TCA_ID_SIPAMD_RX_INV_REQ_RETRAS, 	NULL},
{"SIPAMD rx noninv req",                4, offsetof(SipAgentPmcounters_s_t,RxNoninviteReqs), OMCI_TCA_ID_SIPAMD_RX_NOINV_REQ,          NULL},
{"SIPAMD rx noninv req retrans", 		4, offsetof(SipAgentPmcounters_s_t,RxNoninviteRetrans), OMCI_TCA_ID_SIPAMD_RX_NOINV_REQ_RETRAS, 	NULL},
{"SIPAMD rx resp", 	                    4, offsetof(SipAgentPmcounters_s_t,RxResponse), OMCI_TCA_ID_SIPAMD_RX_RESPONSE, 	        NULL},
{"SIPAMD rx resp retrans", 	            4, offsetof(SipAgentPmcounters_s_t,RxResponseRetransmissions), OMCI_TCA_ID_SIPAMD_RX_RESPONS_RETRANS, 	NULL},
{"SIPAMD tx invite req",                4, offsetof(SipAgentPmcounters_s_t,TxInviteReqs), NO_TCA,                                   NULL},
{"SIPAMD tx invite retrans",            4, offsetof(SipAgentPmcounters_s_t,TxInviteRetrans), NO_TCA,                                   NULL},
{"SIPAMD tx noninv req",                4, offsetof(SipAgentPmcounters_s_t,TxNonInviteReqs), NO_TCA,                                   NULL},
{"SIPAMD tx noninvite retrans",         4, offsetof(SipAgentPmcounters_s_t,TxNonInviteRetrans), NO_TCA,                                   NULL},
{"SIPAMD tx response",                  4, offsetof(SipAgentPmcounters_s_t,TxResponse), NO_TCA,                                   NULL},
{"SIPAMD tx response retrans",          4, offsetof(SipAgentPmcounters_s_t,TxResponseRetransmissions), NO_TCA,                                   NULL},
{"", 					                0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForSipAgentPM(pmmgrPmMe_t *me_p)
{
    pmmgrPmAttriDescript_t *attri_p = NULL;
    SipAgentPmcounters_s_t pm;

    if (me_p == NULL)
    {
    	pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForSiAgentPM: me_p == NULL");
    	return -1;
    }

    memset(&pm, 0, sizeof(pm));
	
    /*1.calculate the me attribute number*/
    me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

    /*2.get the me struct total size*/
    me_p->attriTotalSize = sizeof(SipAgentPmcounters_s_t);

    /*3.calculate the me attribute number*/
    attri_p = me_p->pmmgrAttriDescriptList;

    return 0;
}

uint32 SipAgentPMInfo(char * attr)
{
    int voip_num = 1;
    char nodeName[32] = {0};
    char tempBuffer[MAX_PM_BUFFER_SIZE] = {0};
    int i = 0;
    int val = 0;
    
    if(attr == NULL)
		return -1;

    strcpy(nodeName, "InfoVoIP_Common");

    if(tcapi_nget(nodeName, attr, tempBuffer,sizeof(tempBuffer)) != TCAPI_PROCESS_OK){
        pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n %s  get %s   fail, return val = %d", __FUNCTION__,attr,val);
        return val;
    }  
    val = strtoul(tempBuffer,0,0);
    return val;
}


int getAllDataFromApiForSipAgentPM(char *data, uint16 portId)
{
    SipAgentPmcounters_s_t *pm = NULL;
    int ret = -1;

    if(data == NULL)
        return ret;

    pm = (SipAgentPmcounters_s_t *)data;
    pm->Transactions = SipAgentPMInfo("Transactions");
    pm->RxInviteReqs = SipAgentPMInfo("RxInviteReqs");
    pm->RxInviteRetrans = SipAgentPMInfo("RxInviteRetrans");
    pm->RxNoninviteReqs = SipAgentPMInfo("RxNonInviteReqs");
    pm->RxNoninviteRetrans = SipAgentPMInfo("RxNonInviteRetrans");
    pm->RxResponse = SipAgentPMInfo("RxResponse");
    pm->RxResponseRetransmissions = SipAgentPMInfo("RxResponseRetrans");
    pm->TxInviteReqs = SipAgentPMInfo("TxInviteReqs");
    pm->TxInviteRetrans = SipAgentPMInfo("TxInviteRetrans");
    pm->TxNonInviteReqs = SipAgentPMInfo("TxNonInviteReqs");
    pm->TxNonInviteRetrans = SipAgentPMInfo("TxNonInviteRetrans");
    pm->TxResponse = SipAgentPMInfo("TxResponse");
    pm->TxResponseRetransmissions = SipAgentPMInfo("TxResponseRetrans");
    ret = 0;
    
    end:
        return ret;
}

//9.9.15 SIP call initiation performance monitoring history data
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptSipCallInitPM[] ={
{"SIP call PM failed connect", 			    4, offsetof(SipCallInitPmcounters_t,FailedToConnect), OMCI_TCA_ID_SIP_CALL_PM_FAIL_CONNECT, 	        NULL},
{"SIP call PM failed to validate", 		    4, offsetof(SipCallInitPmcounters_t,FailedToValidate), OMCI_TCA_ID_SIP_CALL_PM_FAIL_VALIDATE, 	        NULL},
{"SIP call PM timeout",                     4, offsetof(SipCallInitPmcounters_t,Timeout), OMCI_TCA_ID_SIP_CALL_PM_TIMEOUT,                  NULL},
{"SIP call PM failure error code received", 4, offsetof(SipCallInitPmcounters_t,FailureReceived), OMCI_TCA_ID_SIP_CALL_PM_FAIL_ERROR_CODE_RECEIVED, NULL},
{"SIP call PM failed to authenticate", 	    4, offsetof(SipCallInitPmcounters_t,FailedToAuthenticate), OMCI_TCA_ID_SIP_CALL_PM_FAILED_AUTH, 	            NULL},
{"", 					0, 0, NO_TCA, NULL}
};

uint32 SipCallInitPMInfo(char * attr)
{
    int voip_num = 1;
    char nodeName[32] = {0};
    char tempBuffer[MAX_PM_BUFFER_SIZE] = {0};
    int i = 0;
    int val = 0;
    
    if(attr == NULL)
        return -1;

    strcpy(nodeName, "VoIPSysParam_Common");

    if(tcapi_nget(nodeName, "SC_SYS_CFG_MAX_LINE", tempBuffer, sizeof(tempBuffer)) == TCAPI_PROCESS_OK){
        voip_num = atoi(tempBuffer);
    }
 
    for (i = 0 ;i < voip_num; i++)
    {
        memset(nodeName, 0, sizeof(nodeName));
        snprintf(nodeName, sizeof(nodeName), "InfoVoIP_%s%x", "Entry", i);  

        if(tcapi_nget(nodeName, attr, tempBuffer, sizeof(tempBuffer)) != TCAPI_PROCESS_OK){
            pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n %s  get %s   fail, return val = %d", __FUNCTION__,attr,val);
            return val;
        }  
        val += strtoul(tempBuffer,0,0);
    }
    return val;
}



int pmmgrPmInitForSipCallInitPM(pmmgrPmMe_t *me_p)
{
    pmmgrPmAttriDescript_t *attri_p = NULL;
    SipCallInitPmcounters_t pm;

    if (me_p == NULL)
    {
    	pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForSiAgentPM: me_p == NULL");
    	return -1;
    }

    memset(&pm, 0, sizeof(pm));
	
    /*1.calculate the me attribute number*/
    me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

    /*2.get the me struct total size*/
    me_p->attriTotalSize = sizeof(SipCallInitPmcounters_t);

    /*3.calculate the me attribute number*/
    attri_p = me_p->pmmgrAttriDescriptList;

    return 0;
}

int getAllDataFromApiForSipCallInitPM(char *data, uint16 portId)
{
    SipCallInitPmcounters_t *pm = NULL;

    int ret = -1;

    if(data == NULL)
    	return ret;

    pm = (SipCallInitPmcounters_t *)data;

    pm->FailedToConnect = SipCallInitPMInfo("FailToConnect");
    pm->FailedToValidate = SipCallInitPMInfo("FailToValidate");
    pm->Timeout = SipCallInitPMInfo("FailTimeout");
    pm->FailureReceived = SipCallInitPMInfo("FailureRecv");
    pm->FailedToAuthenticate = SipCallInitPMInfo("FailToAuth");

    ret = 0;

    end:
        return ret;
}

#endif

//9.4.2 IP host config data
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptIpHostCfgDataPM[] ={
//{"icmpErrors", 		4, 0, OMCI_TCA_ID_IP_HOST_PM_ICMP_ERRORS, 		NULL},
{"icmpErrors", 		4, 0, NO_TCA, 		NULL},
//{"dnsErrors", 			4, 0, OMCI_TCA_ID_IP_HOST_PM_DNS_ERRORS, 		NULL},
{"dnsErrors", 			4, 0, NO_TCA, 		NULL},
//{"dhcpTimeout", 		2, 0, OMCI_TCA_ID_IP_HOST_PM_DHCP_TIMEOUT, 		NULL},
{"dhcpTimeout", 		2, 0, NO_TCA,		NULL},
//{"ipConflict", 			2, 0, OMCI_TCA_ID_IP_HOST_PM_IP_CONFLICT, 		NULL},
{"ipConflict",			2, 0, NO_TCA,		NULL},
//{"outOfMemory", 		2, 0, OMCI_TCA_ID_IP_HOST_PM_OUT_OF_MEMORY, 	NULL},
{"outOfMemory", 		2, 0, NO_TCA, 	NULL},
//{"internalError", 		2, 0, OMCI_TCA_ID_IP_HOST_PM_INTERNAL_ERROR, 	NULL},
{"internalError",		2, 0, NO_TCA,	NULL},
{"", 					0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForIpHostCfgDataPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	IPHostCfgDataPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForIpHostCfgDataPM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(IPHostCfgDataPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.icmpErrors - (uint8*)&pm.icmpErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForIpHostCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.dnsErrors - (uint8*)&pm.icmpErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForIpHostCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.dhcpTimeout - (uint8*)&pm.icmpErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForIpHostCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.ipConflict - (uint8*)&pm.icmpErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForIpHostCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.outOfMemory - (uint8*)&pm.icmpErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForIpHostCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.internalError - (uint8*)&pm.icmpErrors;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForIpHostCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	return 0;
}

//9.4.4 TCP/UDP config data
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptTcpUdpCfgDataPM[] ={
//{"socketFailed", 	4, 0, OMCI_TCA_ID_TCP_UDP_PM_SOCKET_FAILED, 		NULL},
{"socketFailed", 	4, 0, NO_TCA, 		NULL},
//{"listenFailed", 	4, 0, OMCI_TCA_ID_TCP_UDP_PM_LISTEN_FAILED, 		NULL},
{"listenFailed",	4, 0, NO_TCA, 		NULL},
//{"bindFailed", 		4, 0, OMCI_TCA_ID_TCP_UDP_PM_BIND_FAILED, 		NULL},
{"bindFailed",		4, 0, NO_TCA,		NULL},
//{"acceptFailed", 	4, 0, OMCI_TCA_ID_TCP_UDP_PM_ACCEPT_FAILED, 		NULL},
{"acceptFailed",	4, 0, NO_TCA, 		NULL},
//{"selectFailed", 	4, 0, OMCI_TCA_ID_TCP_UDP_PM_SELECT_FAILED, 		NULL},
{"selectFailed",	4, 0, NO_TCA, 		NULL},
{"", 				0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForTcpUdpCfgDataPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	TcpUdpCfgDataPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForTcpUdpCfgDataPM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(TcpUdpCfgDataPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.socketFailed - (uint8*)&pm.socketFailed;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTcpUdpCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.listenFailed - (uint8*)&pm.socketFailed;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTcpUdpCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.bindFailed - (uint8*)&pm.socketFailed;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTcpUdpCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.acceptFailed - (uint8*)&pm.socketFailed;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTcpUdpCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.selectFailed - (uint8*)&pm.socketFailed;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTcpUdpCfgDataPM:attri_p->offset = %x\n", attri_p->offset);

	return 0;
}

//9.2.6 GEM port performance monitoring PM
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptGemPortPM[] ={
	{"Lost packets", 		4, 0, OMCI_TCA_ID_GEM_PORT_PM_LOST_PACKETS, NULL},
	{"Misinserted packets",	4, 0, OMCI_TCA_ID_GEM_PORT_PM_MISINSERTED_PACKETS, NULL},
	{"Received packets", 	8, 0, NO_TCA, NULL},
	{"Received blocks", 	8, 0, NO_TCA, NULL},
	{"Transmitted blocks", 	8, 0, NO_TCA, NULL},
	{"Impaired blocks", 	4, 0, OMCI_TCA_ID_GEM_PORT_PM_IMPAIRED_PACKETS, NULL},
	{"", 					0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForGemPortPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	GemPortPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForGalEthernetPM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(GemPortPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.lostPackets- (uint8*)&pm.lostPackets;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.misinsertedPackets- (uint8*)&pm.lostPackets;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.receivedPackets- (uint8*)&pm.lostPackets;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.receivedBlocks- (uint8*)&pm.lostPackets;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.transmittedBlocks- (uint8*)&pm.lostPackets;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.impairedBlocks- (uint8*)&pm.lostPackets;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGemPortPM:attri_p->offset = %x\n", attri_p->offset);

	return 0;
}


 /***********************************************************************************
MACRO_CMD_STRUCT(GponGemCounter)
{
	MGR_U16 			GemPortId ;
	MGR_U32				RxGemFrameH ;
	MGR_U32				RxGemFrameL ;
	MGR_U32				RxGemPayloadH ;
	MGR_U32				RxGemPayloadL ;
	MGR_U32				TxGemFrameH ;
	MGR_U32				TxGemFrameL ;	
	MGR_U32				TxGemPayloadH ;	
	MGR_U32				TxGemPayloadL ;		
} ;
***********************************************************************************/
int getAllDataFromApiForGemPortPM(char *data, uint16 portId)
{
	GemPortPmcounters_t *pm = NULL;
	MACRO_CMD_STRUCT(GponGemCounter) gponGemCounter;

	if(data == NULL)
		return -1;

	pm = (GemPortPmcounters_t *)data;

	memset(&gponGemCounter, 0, sizeof(gponGemCounter)) ;
	gponGemCounter.GemPortId = portId ;
	if(PonApi_GponGemCounter_Get(&gponGemCounter) != 0) {
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"getAllDataFromApiForGemPortPM :  PonApi_GponGemCounter_Get fail\n");
		return -1;
	}

	pm->lostPackets = 0;
	pm->misinsertedPackets= 0;

	put32((char *)&pm->receivedPackets, htonl(gponGemCounter.RxGemFrameH));
	put32((char *)&pm->receivedPackets+4, htonl(gponGemCounter.RxGemFrameL));

	put32((char *)&pm->receivedBlocks, htonl(gponGemCounter.RxGemPayloadH));
	put32((char *)&pm->receivedBlocks + 4, htonl(gponGemCounter.RxGemPayloadL));
	pm->receivedBlocks /= 48;

	put32((char *)&pm->transmittedBlocks, htonl(gponGemCounter.TxGemPayloadH));
	put32((char *)&pm->transmittedBlocks + 4, htonl(gponGemCounter.TxGemPayloadL));
	pm->transmittedBlocks /= 48;
	pm->impairedBlocks	=	0;
	
		//api
		
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortPM pm->lostPackets = %x\n", pm->lostPackets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortPM pm->misinsertedPackets = %x\n", pm->misinsertedPackets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortPM pm->receivedPackets = %llx\n", pm->receivedPackets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortPM pm->receivedBlocks = %llx\n", pm->receivedBlocks);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortPM pm->transmittedBlocks = %llx\n",pm->transmittedBlocks);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortPM pm->impairedBlocks = %x\n", pm->impairedBlocks);
#endif
	return 0;
}

#ifdef TCSUPPORT_OMCI_ALCATEL
//ALCATEL Vendor Specific ME:   Total GEM Port PM
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptTotalGemPortPM[] = {
	{"Lost GEM Fragment Counter Downstream", 	4, 0, OMCI_TCA_ID_TOTAL_GEM_PORT_PM_LOST_COUNTER_DOWNSTREAM, NULL},
	{"Lost GEM Fragment Counter Upstream",		4, 0, OMCI_TCA_ID_TOTAL_GEM_PORT_PM_LOST_COUNTER_UPSTREAM, NULL},
	{"Received GEM Fragment Counter", 			8, 0, NO_TCA, NULL},
	{"Received GEM Blocks", 					8, 0, NO_TCA, NULL},
	{"Transmitted GEM Blocks", 					8, 0, NO_TCA, NULL},
	{"Impaired blocks", 						4, 0, OMCI_TCA_ID_TOTAL_GEM_PORT_PM_IMPAIRED_BLOCKS, NULL},
	{"Transmitted GEM Fragment Counter", 		8, 0, NO_TCA, NULL},
	{"Bad GEM Headers Received Counter", 		4, 0, OMCI_TCA_ID_TOTAL_GEM_PORT_PM_BAD_RECEIVED_CONTERS, NULL},
	{"Received GEM Blocks 64Bit", 				8, 0, NO_TCA, NULL},
	{"Transmitted GEM Blocks 64Bit",		 	8, 0, NO_TCA, NULL},
	{"", 										0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForTotalGemPortPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	TotalGemPortPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForTotalGemPortPM: me_p == NULL");
		return -1;
	}
	
	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);
	
	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(TotalGemPortPmcounters_t);
	
	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.lostFragmentDown - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.lostFragmentUp - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.receivedFragment - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.receivedBlocks - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.transmittedBlocks - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.impairedBlocks - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.transmittedFragment - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.badReceivedPackets - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.receivedBlocks64 - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.transmittedBlocks64 - (uint8*)&pm.lostFragmentDown;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForTotalGemPortPM:attri_p->offset = %x\n", attri_p->offset);
	
	return 0;
}

int getAllDataFromApiForTotalGemPortPM(char *data, uint16 portId)
{
	int ret = -1;
	uint16 portID = NULL;
	char buffer[256] = {0};
	char tempBuf[MAX_PM_BUFFER_SIZE] = {0};
	char attrName[32] = {0};
	uint32 fromLen = 0;
	char nodeName[32]={0};
	TotalGemPortPmcounters_t *pm = NULL;
	TotalGemPortPmcounters_t *pm_tmp = NULL;
	MACRO_CMD_STRUCT(GponGemCounter) gponGemCounter;
	
	if(data == NULL)
		return -1;
		
	strcpy(nodeName, "OMCI_Entry");
	pm = (TotalGemPortPmcounters_t *)data;
	/*init PM counter to 0*/
	memset(pm, 0, sizeof(TotalGemPortPmcounters_t));
	
	pm_tmp = calloc(1, sizeof(TotalGemPortPmcounters_t));
	if(pm_tmp == NULL)
		return -1;

	/*get attribute name*/
	sprintf(attrName, "%s", PMGR_ATTR_TOTALGEMPORT);
	if(tcapi_nget(nodeName, attrName, buffer, sizeof(buffer)) != TCAPI_PROCESS_OK) {
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n getAllDataFromApiForTotalGemPortPM->getAttrValue ,  fail");
		ret = -1;
		goto end;
	}
	
	fromLen = 0;
	while(findRowBufFirstStr(buffer, &fromLen, tempBuf) == 0) {
		sscanf(tempBuf, "%x", &portID);
		
		memset(&gponGemCounter, 0, sizeof(gponGemCounter)) ;
		gponGemCounter.GemPortId = portID;
		if(PonApi_GponGemCounter_Get(&gponGemCounter) != 0) {
			pmmgrPrintf(PMMGR_DEBUG_ERROR,"getAllDataFromApiForTotalGemPortPM :	PonApi_GponGemCounter_Get fail\n");
			ret = -1;
			goto end;
		}
		
		//put32((char *)&pm_tmp->receivedFragment, htonl(gponGemCounter.RxGemFrameH));
		//put32((char *)&pm_tmp->receivedFragment+4, htonl(gponGemCounter.RxGemFrameL));
		//pm->receivedFragment += pm_tmp->receivedFragment;
		
		put32((char *)&pm_tmp->receivedBlocks, htonl(gponGemCounter.RxGemPayloadH));
		put32((char *)&pm_tmp->receivedBlocks + 4, htonl(gponGemCounter.RxGemPayloadL));
		pm->receivedBlocks += pm_tmp->receivedBlocks / 48;
		
		put32((char *)&pm_tmp->transmittedBlocks, htonl(gponGemCounter.TxGemPayloadH));
		put32((char *)&pm_tmp->transmittedBlocks + 4, htonl(gponGemCounter.TxGemPayloadL));
		pm->transmittedBlocks += pm_tmp->transmittedBlocks / 48;
		
		//put32((char *)&pm_tmp->transmittedFragment, htonl(gponGemCounter.TxGemPayloadH));
		//put32((char *)&pm_tmp->transmittedFragment + 4, htonl(gponGemCounter.TxGemPayloadL));
		//pm->transmittedFragment += pm_tmp->transmittedFragment;
		
		put32((char *)&pm_tmp->receivedBlocks64, htonl(gponGemCounter.RxGemPayloadH));
		put32((char *)&pm_tmp->receivedBlocks64 + 4, htonl(gponGemCounter.RxGemPayloadL));
		pm->receivedBlocks64 += pm_tmp->receivedBlocks64 / 48;
		
		put32((char *)&pm_tmp->transmittedBlocks64, htonl(gponGemCounter.TxGemPayloadH));
		put32((char *)&pm_tmp->transmittedBlocks64 + 4, htonl(gponGemCounter.TxGemPayloadL));
		pm->transmittedBlocks64 += pm_tmp->transmittedBlocks64 / 48;
	}

#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->lostPacketsDown = %x\n", pm->lostFragmentDown);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->lostPacketsUp = %x\n", pm->lostFragmentUp);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->receivedFragment = %llx\n", pm->receivedFragment);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->receivedBlocks = %llx\n", pm->receivedBlocks);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->transmittedBlocks = %llx\n",pm->transmittedBlocks);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->impairedBlocks = %x\n", pm->impairedBlocks);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->transmittedFragment = %x\n", pm->transmittedFragment);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->badReceivedPackets = %x\n", pm->badReceivedPackets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->receivedBlocks64 = %x\n", pm->receivedBlocks64);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForTotalGemPortPM pm->transmittedBlocks64 = %x\n", pm->transmittedBlocks64);
#endif

	ret = 0;

end:
	free(pm_tmp);
	return ret;
}

//ALCATEL Vendor Specific ME:   Ethernet Traffic PM
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetTrafficPM[] = {
	{"Upstream Frames Counter", 				4, 0, NO_TCA, NULL},
	{"Downstream Frames Counter",				4, 0, NO_TCA, NULL},
	{"Upstream Bytes Counter", 					4, 0, NO_TCA, NULL},
	{"Downstream Bytes Counter", 				4, 0, NO_TCA, NULL},
	{"Dropped Upstream Frames Counter", 		4, 0, OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_FRAMES_UPSTREAM_COUNTER, NULL},
	{"Dropped Upstream Bytes Counter", 			4, 0, OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_BYTES_UPSTREAM_COUNTER, NULL},
	{"Dropped Downstream Frames Counter", 		4, 0, OMCI_TCA_ID_ETHERNET_TRAFFIC_PM_DROPPED_FRAMES_DOWNSTREAM_COUNTER, NULL},
	{"Multicast Upstream Frames Counter", 		4, 0, NO_TCA, NULL},
	{"Multicast Downstream Frames Counter", 	4, 0, NO_TCA, NULL},
	{"Upstream Bytes Counter 64Bit",		 	8, 0, NO_TCA, NULL},
	{"Downstream Bytes Counter 64Bit",		 	8, 0, NO_TCA, NULL},
	{"Upstream Frames Counter 64Bit",		 	8, 0, NO_TCA, NULL},
	{"Downstream Frames Counter 64Bit",		 	8, 0, NO_TCA, NULL},
	{"", 										0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForEthernetTrafficPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	EthernetTrafficPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForTotalGemPortPM: me_p == NULL");
		return -1;
	}
	
	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);
	
	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(EthernetTrafficPmcounters_t);
	
	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.UpstreamFrames - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.DownstreamFrames - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.UpstreamBytes - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.DownstreamBytes - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.DropUpstreamFrames - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.DropUpstreamBytes - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.DropDownstreamFrames - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.MulticastUpstreamFrames - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.MulticastDownstreamFrames - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.UpstreamBytes64 - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.DownstreamBytes64 - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.UpstreamFrames64 - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.DownstreamFrames64 - (uint8*)&pm.UpstreamFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForEthernetTrafficPM:attri_p->offset = %x\n", attri_p->offset);
	
	return 0;
}

int getAllDataFromApiForEthernetTrafficPM(char *data, uint16 portId)
{
	EthernetTrafficPmcounters_t *pm = NULL;
	uint32 tempPortId = 0;
	uint32 cnt = 0;

	if(data == NULL){
		return -1;
	}
	pm = (EthernetTrafficPmcounters_t *)data;

	/*init PM counter to 0*/
	memset(pm, 0, sizeof(EthernetTrafficPmcounters_t));
	tempPortId = (portId & 0xFF)-1;		
	
	if(macMT7530GetPortRxOctetsCnt(tempPortId, &cnt) == 0){
		pm->UpstreamBytes += cnt;
	}	
	if(macMT7530GetPortTxOctetsCnt(tempPortId, &cnt) == 0){
		pm->DownstreamBytes += cnt;
	}
	
	if(macMT7530GetPortRxPktsCnt(tempPortId, &cnt) == 0){
		pm->UpstreamFrames += cnt;
	}
	if(macMT7530GetPortTxPktsCnt(tempPortId, &cnt) == 0){
		pm->DownstreamFrames += cnt;
	}
	
	if(macMT7530GetPortRxMultiPktsCnt(tempPortId, &cnt) == 0){
		pm->MulticastUpstreamFrames += cnt;
	}
	if(macMT7530GetPortTxMultiPktsCnt(tempPortId, &cnt) == 0){
		pm->MulticastDownstreamFrames += cnt;
	}

	if(macMT7530GetPortRxDropFramesCnt(tempPortId, &cnt) == 0){
		pm->DropUpstreamFrames += cnt;
	}
	if(macMT7530GetPortTxDropFramesCnt(tempPortId, &cnt) == 0){
		pm->DropDownstreamFrames += cnt;
	}
	
	pm->UpstreamBytes64 = pm->UpstreamBytes;
	pm->DownstreamBytes64 = pm->DownstreamBytes;
	pm->UpstreamFrames64 = pm->UpstreamFrames;
	pm->DownstreamFrames64 = pm->DownstreamFrames;
	
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->UpstreamBytes = %x\n", pm->UpstreamBytes);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->DownstreamBytes = %x\n", pm->DownstreamBytes);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->UpstreamFrames = %llx\n", pm->UpstreamFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->DownstreamFrames = %llx\n", pm->DownstreamFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->DropUpstreamFrames = %llx\n",pm->DropUpstreamFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->DropUpstreamBytes = %x\n", pm->DropUpstreamBytes);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->DropDownstreamFrames = %x\n", pm->DropDownstreamFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->MulticastUpstreamFrames = %x\n", pm->MulticastUpstreamFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->MulticastDownstreamFrames = %x\n", pm->MulticastDownstreamFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->UpstreamBytes64 = %x\n", pm->UpstreamBytes64);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->DownstreamBytes64 = %x\n", pm->DownstreamBytes64);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->UpstreamFrames64 = %x\n", pm->UpstreamFrames64);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetTrafficPM pm->DownstreamFrames64 = %x\n", pm->DownstreamFrames64);
#endif

	return 0;
}
#endif


//9.2.8 GAl Ethernet performance monitoring PM
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptGalEthernetPM[] ={
//{"discardedFrames", 	4, 0, OMCI_TCA_ID_GAL_ETHERNET_PM_DISCARDED_FRAMES, NULL},
{"discardedFrames", 	4, 0, NO_TCA, NULL},
{"", 					0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForGalEthernetPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	GALEthernetPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForGalEthernetPM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(GALEthernetPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.discardedFrames - (uint8*)&pm.discardedFrames;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGalEthernetPM:attri_p->offset = %x\n", attri_p->offset);

	return 0;
}

int getDataFromApi(char *data, uint16 portId)
{
	*(uint32 *)data = gpPmmgrGponDbAddr->times;

	return 0;
}

int getAllDataFromApiForFecPM(char *data, uint16 portId)
{
	FECPmcounters_t *pm = NULL;
	MACRO_CMD_STRUCT(PhyFecCounter) phyFecCounter;

	if(data == NULL)
		return -1;

	//ponmgr api
	memset(&phyFecCounter, 0, sizeof(phyFecCounter)) ;
	if(PonApi_PhyFecCounter_Get(&phyFecCounter) != 0) {
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"getAllDataFromApiForFecPM :  PonApi_PhyFecCounter_Get fail\n");
		return -1;
	}

	pm = (FECPmcounters_t *)data;
	pm->correctedBytes = phyFecCounter.CorrBytes;
	pm->correctedCodeWords = phyFecCounter.CorrCodeWords;
	pm->uncorrectableCodeWords = phyFecCounter.UncorrCodeWords;
	pm->totalCodeWords = phyFecCounter.TotalRxCodeWords;
	pm->FECSeconds = phyFecCounter.FecSeconds;


	
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForFecPM pm->correctedBytes = %x\n", pm->correctedBytes);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForFecPM pm->correctedCodeWords = %x\n", pm->correctedCodeWords);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForFecPM pm->uncorrectableCodeWords = %x\n", pm->uncorrectableCodeWords);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForFecPM pm->totalCodeWords = %x\n", pm->totalCodeWords);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForFecPM pm->FECSeconds = %x\n", pm->FECSeconds);
#endif
	return 0;
}

int getAllDataFromApiForGemPortCtpPM(char *data, uint16 portId)
{
	GEMPort_CTP_Pmcounters_t *pm = (GEMPort_CTP_Pmcounters_t *)data;
	pm->transmitGEMFrames= gpPmmgrGponDbAddr->times;
	pm->receiveGEMFrames= gpPmmgrGponDbAddr->times +1;
	pm->receivePayloadBytes = gpPmmgrGponDbAddr->times +2;
	pm->transmitPayloadBytes= gpPmmgrGponDbAddr->times +3;

#if 1
	/*wait for PON MAC's Api*/

	if((portId & 0x8000) != 0)
	{
		// WAN interface,wait for PON MAC API
	//	ret = getGEMPort_CTP_PM_From_PONMAC(pm, portId);
	}
	else
	{
		// LAN interface
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "getAllDataFromApiForGemPortCtpPM: portId is lan interface---error !\n");
		return -1;
		
	}


#endif

#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortCtpPM pm->transmitGEMFrames = %lu\n", pm->transmitGEMFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortCtpPM pm->receiveGEMFrames = %lu\n", pm->receiveGEMFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortCtpPM pm->receivePayloadBytes = %llu\n", pm->receivePayloadBytes);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGemPortCtpPM pm->transmitPayloadBytes = %llu\n", pm->transmitPayloadBytes);
#endif
	return 0;
}

int getAllDataFromApiForEnergyConsumPM(char *data, uint16 portId)
{
	EnergyConsumed_Pmcounters_t *pm = (EnergyConsumed_Pmcounters_t *)data;
	pm->dozeTime= gpPmmgrGponDbAddr->times;
	pm->cyclicSleepTime= gpPmmgrGponDbAddr->times +1;
	pm->energyConsumed= gpPmmgrGponDbAddr->times +2;

	// wait for API
//	ret = getEnergyConsum_PM(pm, portId);
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEnergyConsumPM pm->dozeTime = %lu\n", pm->dozeTime);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEnergyConsumPM pm->cyclicSleepTime = %lu\n", pm->cyclicSleepTime);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEnergyConsumPM pm->energyConsumed = %lu\n", pm->energyConsumed);	
#endif
	return 0;
}

/*******************************************************************************************************************************
start
9.3.3 MAC bridge performance monitoring history data ME

********************************************************************************************************************************/
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptMACBridgePM[] ={
//{"entry discard count", 		4, 0, OMCI_TCA_ID_MAC_BRIDGE_PM_LEARN_ENTRY_DISCARD_CNTS, NULL},
{"entry discard count", 		4, 0, NO_TCA, NULL},
{"", 						0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForMACBridgePM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	MACBridgePmCounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForMACBridgePM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(MACBridgePmCounters_t));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(MACBridgePmCounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.learnEntryDiscardCnts - (uint8*)&pm.learnEntryDiscardCnts;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForMACBridgePM:attri_p->offset = %x\n", attri_p->offset);
	
	return 0;
}

int getAllDataFromApiForMACBridgePM(char *data, uint16 portId){
	MACBridgePmCounters_t *pm = NULL;;
//	uint16 temp = 0;
//	uint8 port = 0;
	int ret = -1;

	if(data == NULL){
		return -1;
	}

	pm = (MACBridgePmCounters_t *)data;
	pm->learnEntryDiscardCnts = 0;
	
#ifdef PMMGR_DEBUG
//	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForMACBridgePM pm->learnEntryDiscardCnts = %x\n", pm->learnEntryDiscardCnts);
#endif

	ret = 0;
end:
	return ret;
}

/*******************************************************************************************************************************
start
9.3.9 mac bridge port performance monitoring history data

********************************************************************************************************************************/
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptMACBridgePortPM[] ={
	{"Forward Frame cnt", 			4, 0, NO_TCA, NULL},
	{"Delay exceeded discard cnt", 	4, 0, OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_DELAY_EXCEED_DISCARD_CNTS, NULL},
	{"MTU exceeded discard cnt", 	4, 0, OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_MTU_EXCEED_DISCARD_CNTS, NULL},
	{"Received frame cnt", 			4, 0, NO_TCA, NULL},
	{"Received and discarded cnt", 	4, 0, OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_RECEIVED_DISCARD_CNTS, NULL},
	{"", 							0, 0, NO_TCA, NULL}
};


int pmmgrPmInitForMACBridgePortPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	MACBridgePortPmCounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForMACBridgePortPM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(MACBridgePortPmCounters_t));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(MACBridgePortPmCounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.forwardFrameCnts - (uint8*)&pm.forwardFrameCnts;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForMACBridgePortPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.delayExceededDiscardCnts - (uint8*)&pm.forwardFrameCnts;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForMACBridgePortPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.mtuExceededDiscardCnts -(uint8*)&pm.forwardFrameCnts;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForMACBridgePortPM:attri_p->offset = %x\n", attri_p->offset);
	
	attri_p++;
	attri_p->offset = (uint8*)&pm.receivedFrameCnts -(uint8*)&pm.forwardFrameCnts;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForMACBridgePortPM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.receivedDiscardCnts -(uint8*)&pm.forwardFrameCnts;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForMACBridgePortPM:attri_p->offset = %x\n", attri_p->offset);
	
	return 0;
}

int getAllDataFromApiForMACBridgePortPM(char *data, uint16 portId){
	int ret = -1;
	MACBridgePortPmCounters_t *pm = NULL;;
//	uint16 temp = 0;
//	uint8 port = 0;
	uint32 ifcType = 0;
	char buffer[MAX_PM_BUFFER_SIZE] = {0};
	char tempBuf[MAX_PM_BUFFER_SIZE] = {0};
	char attrName[32] = {0};
	uint32 fromLen = 0;
	uint32 tempPortId = 0;
	uint32 cnt = 0;
	char nodeName[32]={0};

	strcpy(nodeName, "OMCI_Entry");
	if(data == NULL){
		goto end;
	}
	pm = (MACBridgePortPmCounters_t *)data;

	/*init PM counter to 0*/
	memset(pm, 0, sizeof(MACBridgePortPmCounters_t));
	
	/*get attribute name*/
	sprintf(attrName, "%s%x", PMGR_ATTR_MACBRIDGEPORT_PREFIX, portId);
	if(tcapi_nget(nodeName, attrName, buffer,sizeof(buffer)) != TCAPI_PROCESS_OK){
		ret = 0;
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForMACBridgePortPM->getAttrValue ,  fail");
		goto end;
	}
	
	/*find interface type (LAN(0)/WAN(1)*/
//	pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForMACBridgePortPM->buffer = %s, fromLen=%d",buffer,fromLen);
	if(findRowBufFirstStr(buffer, &fromLen, tempBuf) != 0){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForMACBridgePortPM->findRowBufFirstStr 1,  fail");
		goto end;
	}
	sscanf(tempBuf, "%x", &ifcType);

	if((ifcType != PM_LAN_INTERFACE_0) && (ifcType != PM_WAN_INTERFACE_1) ){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForMACBridgePortPM->ifcType =%d,  fail", ifcType);
		goto end;
	}

	/*get all port and caculate PM counter*/
	while(findRowBufFirstStr(buffer, &fromLen, tempBuf) == 0){
		sscanf(tempBuf, "%x", &tempPortId);		
		if(ifcType == PM_LAN_INTERFACE_0){//LAN port
			if(macMT7530GetPortTxSuccFramesCnt(tempPortId, &cnt) == 0){
				pm->forwardFrameCnts += cnt;
			}
			
			pm->delayExceededDiscardCnts = 0;
			pm->mtuExceededDiscardCnts = 0;
			
			if(macMT7530GetPortRxPktsCnt(tempPortId, &cnt) == 0){
				pm->receivedFrameCnts += cnt;
			}
			if(macMT7530GetPortRxDropFramesCnt(tempPortId, &cnt) == 0){
				pm->receivedDiscardCnts += cnt;
			}
			
		}else if(ifcType == PM_WAN_INTERFACE_1){//gem port
#if 0
			if(PonApi_PonTestCmd_Get(&getEntry) == EXEC_OK){
				pm->forwardFrameCnts += getEntry.value1;
				pm->delayExceededDiscardCnts += getEntry.value1;
				pm->mtuExceededDiscardCnts += getEntry.value1;
				pm->receivedFrameCnts += getEntry.value1;
				pm->receivedDiscardCnts += getEntry.value1;
			}
#endif

		}else{
			//nothing
		}
	}	
	
#if 0//def PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForMACBridgePortPM pm->forwardFrameCnts = %x\n", pm->forwardFrameCnts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForMACBridgePortPM pm->delayExceededDiscardCnts = %x\n", pm->delayExceededDiscardCnts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForMACBridgePortPM pm->mtuExceededDiscardCnts = %x\n", pm->mtuExceededDiscardCnts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForMACBridgePortPM pm->receivedFrameCnts = %x\n", pm->receivedFrameCnts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForMACBridgePortPM pm->receivedDiscardCnts = %x\n", pm->receivedDiscardCnts);
#endif

	ret = 0;
end:
	return ret;
}

/*******************************************************************************************************************************
start
9.3.30 ethernet frame performance monitoring history data upstream

********************************************************************************************************************************/
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetFrameUpPM[] ={
	{"Drop events", 				4, 0, OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_UP_DELAY_EXCEED_DISCARD_CNTS, NULL},
	{"Octets", 					4, 0, NO_TCA, NULL},
	{"Packets", 					4, 0, NO_TCA, NULL},
	{"Broadcast packets", 			4, 0, NO_TCA, NULL},
	{"Multicast packets", 			4, 0, NO_TCA, NULL},
	{"CRC errored packets", 		4, 0, OMCI_TCA_ID_ETH_FRAME_PM_UP_CRC_ERRORS_PKTS, NULL},
	{"Undersize packets", 			4, 0, OMCI_TCA_ID_ETH_FRAME_PM_UP_UNDERSIZE_PKTS, NULL},
	{"Oversize packets", 			4, 0, OMCI_TCA_ID_ETH_FRAME_PM_UP_OVERSIZE_PKTS, NULL},
	{"Packets 64 octets", 			4, 0, NO_TCA, NULL},
	{"Packets 65-127 octets", 		4, 0, NO_TCA, NULL},
	{"Packets 128-255 octets", 		4, 0, NO_TCA, NULL},
	{"Packets 256-511 octets", 		4, 0, NO_TCA, NULL},
	{"Packets 512-1023 octets", 		4, 0, NO_TCA, NULL},
	{"Packets 1027-1518 octets",	4, 0, NO_TCA, NULL},
	{"", 							0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForEthernetFrameStreamPM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	EthFramePmStreamCounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmgrPmInitFor_EthernetFrameUp_PM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(EthFramePmStreamCounters_t));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(EthFramePmStreamCounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	/*3.1 set  Drop events offset*/
	attri_p->offset = (uint8*)&pm.dropEvents - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.2 set  Octets offset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.octets - (uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.3 set  Packets offset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.packets -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.4 set  Broadcast packets offset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.broadcastPkts -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.5 set  Multicast packets offset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.multicastPkts -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.6 set  CRC errored packets offset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.crcErroredPkts -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.7 set  Undersize packets offset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.undersizePkts -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.8 set  Oversize packets offset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.oversizePkts -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.9 set  Packets 64 octetsoffset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts64Octets -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.10 set  Packets 65-127 octetsoffset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts127Octets -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.11 set  Packets 128-255 octetsoffset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts255Octets -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.12 set  Packets 256-511 octetsoffset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts511Octets -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.13 set  Packets 512-1023 octetsoffset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts1023Octets -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);

	/*3.14 set  Packets 1027-1518 octetsoffset*/
	attri_p++;
	attri_p->offset = (uint8*)&pm.pkts1518Octets -(uint8*)&pm.dropEvents;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmgrPmInitFor_EthernetFrameUp_PM:attri_p->offset = %x\n", attri_p->offset);	
	
	return 0;
}

int getAllDataFromApiForEthernetFrameUpPM(char *data, uint16 portId){
	int ret = -1;
	EthFramePmStreamCounters_t *pm = NULL;;
//	uint16 temp = 0;
//	uint8 port = 0;
	uint32 ifcType = 0;
	char buffer[MAX_PM_BUFFER_SIZE] = {0};
	char tempBuf[MAX_PM_BUFFER_SIZE] = {0};
	char attrName[32] = {0};
	uint32 fromLen = 0;
	uint32 tempPortId = 0;
	uint32 cnt = 0;
	char nodeName[32]={0};
	
	if(data == NULL){
		goto end;
	}
	strcpy(nodeName, "OMCI_Entry");
	pm = (EthFramePmStreamCounters_t *)data;

	/*init PM counter to 0*/
	memset(pm, 0, sizeof(EthFramePmStreamCounters_t));

	/*get attribute name*/
	sprintf(attrName, "%s%x", PMGR_ATTR_MACBRIDGEPORT_PREFIX, portId);
	if(tcapi_nget(nodeName, attrName, buffer,sizeof(buffer)) != TCAPI_PROCESS_OK){
		ret = 0;
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForMACBridgePortPM->getAttrValue ,  fail");
		goto end;
	}	

	/*find interface type (LAN(0)/WAN(1)*/
	//pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForEthernetFrameUpPM->buffer = %s, fromLen=%d",buffer,fromLen);

	if(findRowBufFirstStr(buffer, &fromLen, tempBuf) != 0){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForEthernetFrameUpPM->findRowBufFirstStr 1,  fail");
	}	
	sscanf(tempBuf, "%x", &ifcType);

	if((ifcType != PM_LAN_INTERFACE_0) && (ifcType != PM_WAN_INTERFACE_1) ){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForEthernetFrameUpPM->ifcType =%d,  fail", ifcType);
		goto end;
	}

	/*get all port and caculate PM counter*/
	while(findRowBufFirstStr(buffer, &fromLen, tempBuf) == 0){
		sscanf(tempBuf, "%x", &tempPortId);	
		
		if(ifcType == PM_LAN_INTERFACE_0){//LAN port
			if(macMT7530GetPortRxLackDropPktCnt(tempPortId, &cnt) == 0){
				pm->dropEvents += cnt;
			}
			if(macMT7530GetPortRxOctetsCnt(tempPortId, &cnt) == 0){
				pm->octets += cnt;
			}
			if(macMT7530GetPortRxPktsCnt(tempPortId, &cnt) == 0){
				pm->packets += cnt;
			}

			if(macMT7530GetPortRxBroadPktsCnt(tempPortId, &cnt) == 0){
				pm->broadcastPkts += cnt;
			}
			if(macMT7530GetPortRxMultiPktsCnt(tempPortId, &cnt) == 0){
				pm->multicastPkts += cnt;
			}
			if(macMT7530GetPortRxCRCPktsCnt(tempPortId, &cnt) == 0){
				pm->crcErroredPkts += cnt;
			}
			if(macMT7530GetPortRxUnderSizePktsCnt(tempPortId, &cnt) == 0){
				pm->undersizePkts += cnt;
			}
			if(macMT7530GetPortRxOverSizePktsCnt(tempPortId, &cnt) == 0){
				pm->oversizePkts += cnt;
			}
			if(macMT7530GetPortRx64PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts64Octets += cnt;
			}
			if(macMT7530GetPortRx127PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts127Octets += cnt;
			}
			if(macMT7530GetPortRx255PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts255Octets += cnt;
			}
			if(macMT7530GetPortRx511PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts511Octets += cnt;
			}
			if(macMT7530GetPortRx1023PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts1023Octets += cnt;
			}
			if(macMT7530GetPortRx1518PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts1518Octets += cnt;
			}	
		}else if(ifcType == PM_WAN_INTERFACE_1){//gem port
#if 0
			if(PonApi_PonTestCmd_Get(&getEntry) == EXEC_OK){
				pm->dropEvents += getEntry.value1;
				pm->octets += getEntry.value1;
				pm->packets += getEntry.value1;
				pm->broadcastPkts += getEntry.value1;
				pm->multicastPkts += getEntry.value1;
				pm->crcErroredPkts += getEntry.value1;
				pm->undersizePkts += getEntry.value1;
				pm->oversizePkts += getEntry.value1;
				pm->pkts64Octets += getEntry.value1;
				pm->pkts127Octets += getEntry.value1;
				pm->pkts255Octets += getEntry.value1;
				pm->pkts511Octets += getEntry.value1;
				pm->pkts1023Octets += getEntry.value1;
				pm->pkts1518Octets += getEntry.value1;
			}
#endif
		}else{
			//nothing
		}
	}	
#if 0//def PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->dropEvents = %x\n", pm->dropEvents);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->octets = %x\n", pm->octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->packets = %x\n", pm->packets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->broadcastPkts = %x\n", pm->broadcastPkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->multicastPkts = %x\n", pm->multicastPkts);

	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->crcErroredPkts = %x\n", pm->crcErroredPkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->undersizePkts = %x\n", pm->undersizePkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->oversizePkts = %x\n", pm->oversizePkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->pkts64Octets = %x\n", pm->pkts64Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->pkts127Octets = %x\n", pm->pkts127Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->pkts255Octets = %x\n", pm->pkts255Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->pkts511Octets = %x\n", pm->pkts511Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->pkts1023Octets = %x\n", pm->pkts1023Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameUpPM pm->pkts1518Octets = %x\n", pm->pkts1518Octets);
#endif

	ret = 0;
end:
	return ret;
}
	
/*******************************************************************************************************************************
start
9.3.31 ethernet frame performance monitoring history data downstream

********************************************************************************************************************************/
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptEthernetFrameDownPM[] ={
	{"Drop events", 				4, 0, OMCI_TCA_ID_MAC_BRIDGE_PORT_PM_DOWN_DELAY_EXCEED_DISCARD_CNTS, NULL},
	{"Octets", 					4, 0, NO_TCA, NULL},
	{"Packets", 					4, 0, NO_TCA, NULL},
	{"Broadcast packets", 			4, 0, NO_TCA, NULL},
	{"Multicast packets", 			4, 0, NO_TCA, NULL},
	{"CRC errored packets", 		4, 0, OMCI_TCA_ID_ETH_FRAME_PM_DOWN_CRC_ERRORS_PKTS, NULL},
	{"Undersize packets", 			4, 0, OMCI_TCA_ID_ETH_FRAME_PM_DOWN_UNDERSIZE_PKTS, NULL},
	{"Oversize packets", 			4, 0, OMCI_TCA_ID_ETH_FRAME_PM_DOWN_OVERSIZE_PKTS, NULL},
	{"Packets 64 octets", 			4, 0, NO_TCA, NULL},
	{"Packets 65-127 octets", 		4, 0, NO_TCA, NULL},
	{"Packets 128-255 octets", 		4, 0, NO_TCA, NULL},
	{"Packets 256-511 octets", 		4, 0, NO_TCA, NULL},
	{"Packets 512-1023 octets", 		4, 0, NO_TCA, NULL},
	{"Packets 1027-1518 octets",	4, 0, NO_TCA, NULL},
	{"", 							0, 0, NO_TCA, NULL}
};

int getAllDataFromApiForEthernetFrameDownPM(char *data, uint16 portId){
	int ret = -1;
	EthFramePmStreamCounters_t *pm = NULL;;
//	uint16 temp = 0;
//	uint8 port = 0;
	uint32 ifcType = 0;
	char buffer[MAX_PM_BUFFER_SIZE] = {0};
	char tempBuf[MAX_PM_BUFFER_SIZE] = {0};
	char attrName[32] = {0};
	uint32 fromLen = 0;
	uint32 tempPortId = 0;
	uint32 cnt = 0;
	char nodeName[32]={0};

	if(data == NULL){
		goto end;
	}
	strcpy(nodeName, "OMCI_Entry");
	pm = (EthFramePmStreamCounters_t *)data;

	/*init PM counter to 0*/
	memset(pm, 0, sizeof(EthFramePmStreamCounters_t));

	/*get attribute name*/
	sprintf(attrName, "%s%x", PMGR_ATTR_MACBRIDGEPORT_PREFIX, portId);
	if(tcapi_nget(nodeName, attrName, buffer,sizeof(buffer)) != TCAPI_PROCESS_OK){
		ret = 0;
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForMACBridgePortPM->getAttrValue ,  fail");
		goto end;
	}

	/*find interface type (LAN(0)/WAN(1)*/
//	pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForEthernetFrameDownPM->buffer = %s, fromLen=%d",buffer,fromLen);
	if(findRowBufFirstStr(buffer, &fromLen, tempBuf) != 0){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForEthernetFrameDownPM->findRowBufFirstStr 1,  fail");
	}	
	sscanf(tempBuf, "%x", &ifcType);

	if((ifcType != PM_LAN_INTERFACE_0) && (ifcType != PM_WAN_INTERFACE_1) ){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n getAllDataFromApiForEthernetFrameDownPM->ifcType =%d,  fail", ifcType);
		goto end;
	}

	/*get all port and caculate PM counter*/
	while(findRowBufFirstStr(buffer, &fromLen, tempBuf) == 0){
		sscanf(tempBuf, "%x", &tempPortId);	
		
		if(ifcType == PM_LAN_INTERFACE_0){//LAN port
			pm->dropEvents = 0;
			if(macMT7530GetPortTxOctetsCnt(tempPortId, &cnt) == 0){
				pm->octets += cnt;
			}
			if(macMT7530GetPortTxPktsCnt(tempPortId, &cnt) == 0){
				pm->packets += cnt;
			}

			if(macMT7530GetPortTxBroadPktsCnt(tempPortId, &cnt) == 0){
				pm->broadcastPkts += cnt;
			}
			if(macMT7530GetPortTxMultiPktsCnt(tempPortId, &cnt) == 0){
				pm->multicastPkts += cnt;
			}
			if(macMT7530GetPortTxCRCPktsCnt(tempPortId, &cnt) == 0){
				pm->crcErroredPkts += cnt;
			}
			pm->undersizePkts = 0;
			pm->oversizePkts = 0;
			if(macMT7530GetPortTx64PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts64Octets += cnt;
			}
			if(macMT7530GetPortTx127PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts127Octets += cnt;
			}
			if(macMT7530GetPortTx255PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts255Octets += cnt;
			}
			if(macMT7530GetPortTx511PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts511Octets += cnt;
			}
			if(macMT7530GetPortTx1023PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts1023Octets += cnt;
			}
			if(macMT7530GetPortTx1518PktsCnt(tempPortId, &cnt) == 0){
				pm->pkts1518Octets += cnt;
			}	
		}else if(ifcType == PM_WAN_INTERFACE_1){//gem port
#if 0		
			if(PonApi_PonTestCmd_Get(&getEntry) == EXEC_OK){
				pm->dropEvents += getEntry.value1;
				pm->octets += getEntry.value1;
				pm->packets += getEntry.value1;
				pm->broadcastPkts += getEntry.value1;
				pm->multicastPkts += getEntry.value1;
				pm->crcErroredPkts += getEntry.value1;
				pm->undersizePkts += getEntry.value1;
				pm->oversizePkts += getEntry.value1;
				pm->pkts64Octets += getEntry.value1;
				pm->pkts127Octets += getEntry.value1;
				pm->pkts255Octets += getEntry.value1;
				pm->pkts511Octets += getEntry.value1;
				pm->pkts1023Octets += getEntry.value1;
				pm->pkts1518Octets += getEntry.value1;
			}
#endif
		}else{
			//nothing
		}
	}	
#if 0//def PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->dropEvents = %x\n", pm->dropEvents);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->octets = %x\n", pm->octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->packets = %x\n", pm->packets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->broadcastPkts = %x\n", pm->broadcastPkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->multicastPkts = %x\n", pm->multicastPkts);

	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->crcErroredPkts = %x\n", pm->crcErroredPkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->undersizePkts = %x\n", pm->undersizePkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->oversizePkts = %x\n", pm->oversizePkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->pkts64Octets = %x\n", pm->pkts64Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->pkts127Octets = %x\n", pm->pkts127Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->pkts255Octets = %x\n", pm->pkts255Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->pkts511Octets = %x\n", pm->pkts511Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->pkts1023Octets = %x\n", pm->pkts1023Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameDownPM pm->pkts1518Octets = %x\n", pm->pkts1518Octets);
#endif

	ret = 0;
end:
	return ret;
}

int getAllDataFromApiForPptpEthernetUniPM(char *data, uint16 portId)
{
	PPTPEthernetUNIPmcounters_t *pm = NULL;
	uint32 cnt = 0;
	uint8 validPortId = 0;

	if(data == NULL)
		return -1;
	/*init PM counter to 0*/
	pm = (PPTPEthernetUNIPmcounters_t *)data;
	memset(pm, 0, sizeof(PPTPEthernetUNIPmcounters_t));
	validPortId = (portId & 0xff);
		
#ifndef TCSUPPORT_MT7530_SWITCH_API		
	
	pm->FCSErrors = gpPmmgrGponDbAddr->times;
	pm->excessiveCollisionCnts = gpPmmgrGponDbAddr->times + 1;
	pm->lateCollisionCnts = gpPmmgrGponDbAddr->times + 2;
	pm->framesTooLong = gpPmmgrGponDbAddr->times + 3;
	pm->bufferOverFlowOnRcv = gpPmmgrGponDbAddr->times + 4;
	pm->singleCollisionFrameCnts = gpPmmgrGponDbAddr->times + 5;
	pm->multiCollisionFrameCnts = gpPmmgrGponDbAddr->times + 6;
	pm->deferredTransmissionCnts = gpPmmgrGponDbAddr->times + 7;
	pm->alignmentErrorCnts = gpPmmgrGponDbAddr->times + 8;
#else
	if(macMT7530GetFCSErrors(validPortId, &cnt) == 0){
		pm->FCSErrors = cnt;
	}
	if(macMT7530GetExcessiveCollisionCounter(validPortId, &cnt) == 0){
		pm->excessiveCollisionCnts = cnt;
	}
	if(macMT7530GetLateCollisionCounter(validPortId, &cnt) == 0){
		pm->lateCollisionCnts = cnt;
	}	
	if(macMT7530GetFramesTooLong(validPortId, &cnt) == 0){
		pm->framesTooLong = cnt;
	}
	if(macMT7530GetBufferOverflowsRx(validPortId, &cnt) == 0){
		pm->bufferOverFlowOnRcv = cnt;
	}	
	if(macMT7530GetSingleCollisionFrameCounter(validPortId, &cnt) == 0){
		pm->singleCollisionFrameCnts = cnt;
	}
	if(macMT7530GetMultipleCollisionsFrameCounter(validPortId, &cnt) == 0){
		pm->multiCollisionFrameCnts = cnt;
	}	
	if(macMT7530GetDeferredTxCounter(validPortId, &cnt) == 0){
		pm->deferredTransmissionCnts = cnt;
	}	
	if(macMT7530GetAlignmentErrorCounter(validPortId, &cnt) == 0){
		pm->alignmentErrorCnts = cnt;
	}
#endif

#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->FCSErrors = %x\n", pm->FCSErrors);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->excessiveCollisionCnts = %x\n", pm->excessiveCollisionCnts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->lateCollisionCnts = %x\n", pm->lateCollisionCnts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->framesTooLong = %x\n", pm->framesTooLong);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->bufferOverFlowOnRcv = %x\n", pm->bufferOverFlowOnRcv);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->singleCollisionFrameCnts = %x\n", pm->singleCollisionFrameCnts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->multiCollisionFrameCnts = %x\n", pm->multiCollisionFrameCnts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->deferredTransmissionCnts = %x\n", pm->deferredTransmissionCnts);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUniPM pm->alignmentErrorCnts = %x\n", pm->alignmentErrorCnts);
#endif
	return 0;
}

int getAllDataFromApiForPptpEthernetUni2PM(char *data, uint16 portId)
{
	PPTPEthernetUNI2Pmcounters_t *pm = NULL;;

	if(data == NULL)
		return -1;
	
	pm = (PPTPEthernetUNI2Pmcounters_t *)data;
	pm->pppoeFilterCnt = 0; 

	return 0;
}

int getAllDataFromApiForPptpEthernetUni3PM(char *data, uint16 portId)
{
	PPTPEthernetUNI3Pmcounters_t *pm = NULL;
	uint32 cnt = 0;
	uint8 validPortId = 0;
	
	if(data == NULL)
		return -1;
	
	pm = (PPTPEthernetUNI3Pmcounters_t *)data;
	memset(pm, 0, sizeof(PPTPEthernetUNI3Pmcounters_t));
	validPortId = (portId & 0xff);
	
#ifndef TCSUPPORT_MT7530_SWITCH_API	
	pm->dropEvents = gpPmmgrGponDbAddr->times;
	pm->octets = gpPmmgrGponDbAddr->times + 1;
	pm->packets = gpPmmgrGponDbAddr->times + 2;
	pm->broadcastPkts = gpPmmgrGponDbAddr->times + 3;
	pm->multicastPkts = gpPmmgrGponDbAddr->times + 4;
	pm->undersizePkts = gpPmmgrGponDbAddr->times + 5;
	pm->fragments = gpPmmgrGponDbAddr->times + 6;
	pm->jabbers = gpPmmgrGponDbAddr->times + 7;
	pm->pkts64Octets = gpPmmgrGponDbAddr->times + 8;
	pm->pkts65To127Octets = gpPmmgrGponDbAddr->times + 9;
	pm->pkts128To255Octets = gpPmmgrGponDbAddr->times + 10;
	pm->pkts256To511Octets = gpPmmgrGponDbAddr->times + 11;
	pm->pkts512To1023Octets = gpPmmgrGponDbAddr->times + 12;
	pm->pkts1024To1518Octets = gpPmmgrGponDbAddr->times + 13;
#else
	if(macMT7530GetPortRxLackDropPktCnt(validPortId, &cnt) == 0){
		pm->dropEvents = cnt;
	}
	if(macMT7530GetPortRxOctetsCnt(validPortId, &cnt) == 0){
		pm->octets = cnt;
	}
	if(macMT7530GetPortRxPktsCnt(validPortId, &cnt) == 0){
		pm->packets = cnt;
	}
	if(macMT7530GetPortRxBroadPktsCnt(validPortId, &cnt) == 0){
		pm->broadcastPkts = cnt;
	}
	if(macMT7530GetPortRxMultiPktsCnt(validPortId, &cnt) == 0){
		pm->multicastPkts = cnt;
	}
	if(macMT7530GetPortRxUnderSizePktsCnt(validPortId, &cnt) == 0){
		pm->undersizePkts = cnt;
	}
	if(macMT7530GetFragments(validPortId, &cnt) == 0){
		pm->fragments = cnt;
	}
	if(macMT7530GetJabbers(validPortId, &cnt) == 0){
		pm->jabbers = cnt;
	}
	if(macMT7530GetPortRx64PktsCnt(validPortId, &cnt) == 0){
		pm->pkts64Octets = cnt;
	}
	if(macMT7530GetPortRx127PktsCnt(validPortId, &cnt) == 0){
		pm->pkts65To127Octets = cnt;
	}
	
	if(macMT7530GetPortRx255PktsCnt(validPortId, &cnt) == 0){
		pm->pkts128To255Octets = cnt;
	}
	if(macMT7530GetPortRx511PktsCnt(validPortId, &cnt) == 0){
		pm->pkts256To511Octets = cnt;
	}
	if(macMT7530GetPortRx1023PktsCnt(validPortId, &cnt) == 0){
		pm->pkts512To1023Octets = cnt;
	}
	if(macMT7530GetPortRx1518PktsCnt(validPortId, &cnt) == 0){
		pm->pkts1024To1518Octets = cnt;
	}
#endif
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->dropEvents = %x\n", pm->dropEvents);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->octets = %x\n", pm->octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->packets = %x\n", pm->packets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->broadcastPkts = %x\n", pm->broadcastPkts);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->multicastPkts = %x\n", pm->multicastPkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->undersizePkts = %x\n", pm->undersizePkts);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->fragments = %x\n", pm->fragments);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->jabbers = %x\n", pm->jabbers);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->pkts64Octets = %x\n", pm->pkts64Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->pkts65To127Octets = %x\n", pm->pkts65To127Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->pkts128To255Octets = %x\n", pm->pkts128To255Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->pkts256To511Octets = %x\n", pm->pkts256To511Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->pkts512To1023Octets = %x\n", pm->pkts512To1023Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForPptpEthernetUni3PM pm->pkts1024To1518Octets = %x\n", pm->pkts1024To1518Octets);
#endif
	return 0;
}
/*9.3.32 Ethernet Frame Extended PM ME*/

void getEthernetFrameExtendedPmmeDatafromSwitchApi(uint8 validPortId,EthernetFrameExtendedPmcounters_t *pm) {
	uint32 cnt = 0;
	if(macMT7530GetPortRxLackDropPktCnt(validPortId, &cnt) == 0){
		pm->dropEvents = cnt;
	}
	if(macMT7530GetPortRxOctetsCnt(validPortId, &cnt) == 0){
		pm->octets = cnt;
	}
	if(macMT7530GetPortRxPktsCnt(validPortId, &cnt) == 0){
		pm->frames = cnt;
	}
	if(macMT7530GetPortRxBroadPktsCnt(validPortId, &cnt) == 0){
		pm->broadcastFrames = cnt;
	}
	if(macMT7530GetPortRxMultiPktsCnt(validPortId, &cnt) == 0){
		pm->multicastFrames = cnt;
	}
	if(macMT7530GetPortRxUnderSizePktsCnt(validPortId, &cnt) == 0){
		pm->undersizeFrames = cnt;
	}
	if(macMT7530GetPortRxCRCPktsCnt(validPortId, &cnt) == 0){
		pm->crcerroredFrames = cnt;
	}
	if(macMT7530GetPortRxOverSizePktsCnt(validPortId, &cnt) == 0){
		pm->oversizeFrames = cnt;
	}
	if(macMT7530GetPortRx64PktsCnt(validPortId, &cnt) == 0){
		pm->frames64Octets = cnt;
	}
	if(macMT7530GetPortRx127PktsCnt(validPortId, &cnt) == 0){
		pm->frames65To127Octets = cnt;
	}
	
	if(macMT7530GetPortRx255PktsCnt(validPortId, &cnt) == 0){
		pm->frames128To255Octets = cnt;
	}
	if(macMT7530GetPortRx511PktsCnt(validPortId, &cnt) == 0){
		pm->frames256To511Octets = cnt;
	}
	if(macMT7530GetPortRx1023PktsCnt(validPortId, &cnt) == 0){
		pm->frames512To1023Octets = cnt;
	}
	if(macMT7530GetPortRx1518PktsCnt(validPortId, &cnt) == 0){
		pm->frames1024To1518Octets = cnt;
	}
}
int getAllDataFromApiForEthernetFrameExtendedPM(char *data, uint16 portId)
{
	EthernetFrameExtendedPmcounters_t *pm = NULL;
	uint8 validPortId = 0;
	
	if(data == NULL)
		return -1;
	
	pm = (EthernetFrameExtendedPmcounters_t *)data;
	memset(pm, 0, sizeof(EthernetFrameExtendedPmcounters_t));
	validPortId = (portId & 0xff);
	getEthernetFrameExtendedPmmeDatafromSwitchApi(validPortId,pm);
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->dropEvents = %x\n", pm->dropEvents);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->octets = %x\n", pm->octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->packets = %x\n", pm->frames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->broadcastPkts = %x\n", pm->broadcastFrames);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->multicastPkts = %x\n", pm->multicastFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->undersizePkts = %x\n", pm->undersizeFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->fragments = %x\n", pm->crcerroredFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->jabbers = %x\n", pm->oversizeFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->pkts64Octets = %x\n", pm->frames64Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->pkts65To127Octets = %x\n", pm->frames65To127Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->pkts128To255Octets = %x\n", pm->frames128To255Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->pkts256To511Octets = %x\n", pm->frames256To511Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->pkts512To1023Octets = %x\n", pm->frames512To1023Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM pm->pkts1024To1518Octets = %x\n", pm->frames1024To1518Octets);
#endif
	return 0;
}


void getRXEthernetExtendedPmme64BitfromSwitchApi(uint8 validPortId,EthernetFrameExtendedPm64bitcounters_t *pm) {
	uint32 cnt = 0;
	if(macMT7530GetPortRxLackDropPktCnt(validPortId, &cnt) == 0){
		pm->dropEvents = cnt;
	}
	if(macMT7530GetPortRxOctetsCnt(validPortId, &cnt) == 0){
		pm->octets = cnt;
	}
	if(macMT7530GetPortRxPktsCnt(validPortId, &cnt) == 0){
		pm->frames = cnt;
	}
	if(macMT7530GetPortRxBroadPktsCnt(validPortId, &cnt) == 0){
		pm->broadcastFrames = cnt;
	}
	if(macMT7530GetPortRxMultiPktsCnt(validPortId, &cnt) == 0){
		pm->multicastFrames = cnt;
	}
	if(macMT7530GetPortRxUnderSizePktsCnt(validPortId, &cnt) == 0){
		pm->undersizeFrames = cnt;
	}
	if(macMT7530GetPortRxCRCPktsCnt(validPortId, &cnt) == 0){
		pm->crcerroredFrames = cnt;
	}
	if(macMT7530GetPortRxOverSizePktsCnt(validPortId, &cnt) == 0){
		pm->oversizeFrames = cnt;
	}
	if(macMT7530GetPortRx64PktsCnt(validPortId, &cnt) == 0){
		pm->frames64Octets = cnt;
	}
	if(macMT7530GetPortRx127PktsCnt(validPortId, &cnt) == 0){
		pm->frames65To127Octets = cnt;
	}
	
	if(macMT7530GetPortRx255PktsCnt(validPortId, &cnt) == 0){
		pm->frames128To255Octets = cnt;
	}
	if(macMT7530GetPortRx511PktsCnt(validPortId, &cnt) == 0){
		pm->frames256To511Octets = cnt;
	}
	if(macMT7530GetPortRx1023PktsCnt(validPortId, &cnt) == 0){
		pm->frames512To1023Octets = cnt;
	}
	if(macMT7530GetPortRx1518PktsCnt(validPortId, &cnt) == 0){
		pm->frames1024To1518Octets = cnt;
	}
}
void getTXEthernetExtendedPmme64BitfromSwitchApi(uint8 validPortId,EthernetFrameExtendedPm64bitcounters_t *pm) {
	uint32 cnt = 0;
	if(macMT7530GetPortTxLackDropPktCnt(validPortId, &cnt) == 0){
		pm->dropEvents = cnt;
	}
	if(macMT7530GetPortTxOctetsCnt(validPortId, &cnt) == 0){
		pm->octets = cnt;
	}
	if(macMT7530GetPortTxPktsCnt(validPortId, &cnt) == 0){
		pm->frames = cnt;
	}
	if(macMT7530GetPortTxBroadPktsCnt(validPortId, &cnt) == 0){
		pm->broadcastFrames = cnt;
	}
	if(macMT7530GetPortTxMultiPktsCnt(validPortId, &cnt) == 0){
		pm->multicastFrames = cnt;
	}
	if(macMT7530GetPortTxUnderSizePktsCnt(validPortId, &cnt) == 0){
		pm->undersizeFrames = cnt;
	}
	if(macMT7530GetPortTxCRCPktsCnt(validPortId, &cnt) == 0){
		pm->crcerroredFrames = cnt;
	}
	if(macMT7530GetPortTxOverSizePktsCnt(validPortId, &cnt) == 0){
		pm->oversizeFrames = cnt;
	}
	if(macMT7530GetPortTx64PktsCnt(validPortId, &cnt) == 0){
		pm->frames64Octets = cnt;
	}
	if(macMT7530GetPortTx127PktsCnt(validPortId, &cnt) == 0){
		pm->frames65To127Octets = cnt;
	}
	
	if(macMT7530GetPortTx255PktsCnt(validPortId, &cnt) == 0){
		pm->frames128To255Octets = cnt;
	}
	if(macMT7530GetPortTx511PktsCnt(validPortId, &cnt) == 0){
		pm->frames256To511Octets = cnt;
	}
	if(macMT7530GetPortTx1023PktsCnt(validPortId, &cnt) == 0){
		pm->frames512To1023Octets = cnt;
	}
	if(macMT7530GetPortTx1518PktsCnt(validPortId, &cnt) == 0){
		pm->frames1024To1518Octets = cnt;
	}
}
/*9.3.34 Ethernet Frame Extended PM64bit ME*/

int getAllDataFromApiForEthernetFrameExtendedPM64bit(char *data, uint16 portId)
{
	EthernetFrameExtendedPm64bitcounters_t *pm = NULL;
	uint8 validPortId = portId & 0xFF;
	uint8 direction = (portId & 0xFF00) >> 8; 
	if(data == NULL)
		return -1;
	
	pm = (EthernetFrameExtendedPm64bitcounters_t *)data;
	memset(pm, 0, sizeof(EthernetFrameExtendedPm64bitcounters_t));
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"PM eth get port %x %s 64bit data\n",validPortId,(direction?"downstream":"upstream"));
    if(direction == ETHERNET_PM_64_BIT_DIRECTION_UP)
    {
		getRXEthernetExtendedPmme64BitfromSwitchApi(portId,pm);
    }
    else if(direction == ETHERNET_PM_64_BIT_DIRECTION_DOWM)
    {
		getTXEthernetExtendedPmme64BitfromSwitchApi(portId,pm);
    }
    else
    {
    	pmmgrPrintf(PMMGR_DEBUG_ERROR,"direction Error\n");
    }
    
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->dropEvents = %x\n", pm->dropEvents);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->octets = %x\n", pm->octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->packets = %x\n", pm->frames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->broadcastPkts = %x\n", pm->broadcastFrames);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->multicastPkts = %x\n", pm->multicastFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->undersizePkts = %x\n", pm->undersizeFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->fragments = %x\n", pm->crcerroredFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->jabbers = %x\n", pm->oversizeFrames);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->pkts64Octets = %x\n", pm->frames64Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->pkts65To127Octets = %x\n", pm->frames65To127Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->pkts128To255Octets = %x\n", pm->frames128To255Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->pkts256To511Octets = %x\n", pm->frames256To511Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->pkts512To1023Octets = %x\n", pm->frames512To1023Octets);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForEthernetFrameExtendedPM64bit pm->pkts1024To1518Octets = %x\n", pm->frames1024To1518Octets);
#endif
	return 0;
}



int getAllDataFromApiForIpHostCfgDataPM(char *data, uint16 portId)
{
	IPHostCfgDataPmcounters_t *pm = NULL;
	
	if(data == NULL)
		return -1;

	pm = (IPHostCfgDataPmcounters_t *)data;
	pm->icmpErrors = 0;
	pm->dnsErrors = 0; 
	pm->dhcpTimeout = 0;
	pm->ipConflict = 0;
	pm->outOfMemory = 0;
	pm->internalError = 0;

	return 0;
}

int getAllDataFromApiForTcpUdpCfgDataPM(char *data, uint16 portId)
{
	TcpUdpCfgDataPmcounters_t *pm = NULL;
	
	if(data == NULL)
		return -1;

	pm = (TcpUdpCfgDataPmcounters_t *)data;
	pm->socketFailed = 0;
	pm->listenFailed = 0;
	pm->bindFailed = 0;
	pm->acceptFailed = 0;
	pm->selectFailed = 0;
	
	return 0;
}

int getAllDataFromApiForGalEthernetPM(char *data, uint16 portId)
{
	GALEthernetPmcounters_t *pm = NULL;
	
	if(data == NULL)
		return -1;

	pm = (GALEthernetPmcounters_t *)data;
	pm->discardedFrames = 0;

#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGalEthernetPM pm->discardedFrames = %x\n", pm->discardedFrames);
#endif
	return 0;
}


#ifdef TCSUPPORT_OMCI_ALCATEL
pmmgrPmAttriDescript_t pmmgrPmAttriDescriptGEMPortProtocolMonitoringHDPart2PM[] ={
{"Lost GEM Fragment Counter(Upstream)", 			4, 0, OMCI_TCA_ID_LOST_GEM_FRAGMENT_COUNTER, 	NULL},
{"Transmitted GEM Fragment Counter", 			4, 0, NO_TCA,	NULL},
{"", 						0, 0, NO_TCA, NULL}
};

int pmmgrPmInitForGEMPortProtocolMonitoringHDPart2PM(pmmgrPmMe_t *me_p)
{
	pmmgrPmAttriDescript_t *attri_p = NULL;
	GEMPortProtocolPmcounters_t pm;
	
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "\r\n pmmgrPmInitForGEMPortProtocolMonitoringHDPart2PM: me_p == NULL");
		return -1;
	}

	memset(&pm, 0, sizeof(pm));
	
	/*1.calculate the me attribute number*/
	me_p->attriNum = pmmgrGetAttriNumOfMe(me_p);

	/*2.get the me struct total size*/
	me_p->attriTotalSize = sizeof(GEMPortProtocolPmcounters_t);

	/*3.calculate the me attribute number*/
	attri_p = me_p->pmmgrAttriDescriptList;
	
	attri_p->offset = (uint8*)&pm.lostGEMFragmentCounter - (uint8*)&pm.lostGEMFragmentCounter;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGEMPortProtocolMonitoringHDPart2PM:attri_p->offset = %x\n", attri_p->offset);

	attri_p++;
	attri_p->offset = (uint8*)&pm.transmittedGEMFragmentCounter- (uint8*)&pm.lostGEMFragmentCounter;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrPmInitForGEMPortProtocolMonitoringHDPart2PM:attri_p->offset = %x\n", attri_p->offset);
	
	return 0;
}

int getAllDataFromApiForGEMPortProtocolMonitoringHDPart2PM(char *data, uint16 portId)
{
	GEMPortProtocolPmcounters_t *pm = NULL;

	if(data == NULL)
		return -1;

	pm = (GEMPortProtocolPmcounters_t *)data;

	pm->lostGEMFragmentCounter = 0;
	pm->transmittedGEMFragmentCounter = gpPmmgrGponDbAddr->times;
		
#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGEMPortProtocolMonitoringHDPart2PM pm->lostGEMFragmentCounter = %x\n", pm->lostGEMFragmentCounter);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter getAllDataFromApiForGEMPortProtocolMonitoringHDPart2PM pm->transmittedGEMFragmentCounter = %x\n", pm->transmittedGEMFragmentCounter);
#endif
	return 0;
}

#endif
