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
	oam_ext_mgr.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lightman		2012/8/24	Create
*/
#include <malloc.h>
#include "epon_oam.h"
#include "ctc/oam_ext_mgr.h"
#include "ctc/oam_ctc_dspch.h"

extern ExtHandler_t gCtcHandler;
extern u_char gOuiCtc[3];	


ExtOrgMgr_t gExtOrgHdlrTbl[] = {
		{&gCtcHandler, initCtcHandler},
};


void initExtOrgMgr()
{
	int i;
	int cnt = sizeof(gExtOrgHdlrTbl) / sizeof(gExtOrgHdlrTbl[0]);

	for (i = 0; i < cnt; i++){
		gExtOrgHdlrTbl[i].initExtHandler(gExtOrgHdlrTbl[i].pHandler);
	}

}

void resetExtOrgDscvyState(u_char llid)
{
	int i;
	int cnt = sizeof(gExtOrgHdlrTbl) / sizeof(gExtOrgHdlrTbl[0]);

	for (i = 0; i < cnt; i++){
		gExtOrgHdlrTbl[i].pHandler->dscvryState[llid] = DS_PASSIVE_WAIT;
	}
	eponOamExtDbg(DBG_OAM_L3, "\r\nCTC: reset LLID %d all extend discovery state to passive wait.\n", llid);
}

ExtHandler_Ptr findHandlerByOui(u_char *oui)
{
	int i;
	int cnt = sizeof(gExtOrgHdlrTbl) / sizeof(gExtOrgHdlrTbl[0]);

	for (i = 0; i < cnt; i++){
		if (isSameOui(oui, gExtOrgHdlrTbl[i].pHandler->oui))
			return gExtOrgHdlrTbl[i].pHandler;
	}
	return NULL;
}

void eponOamSetCtcDiscoveryState(unsigned char llid, unsigned char state){
	ExtHandler_Ptr pExtHdlr = findHandlerByOui(gOuiCtc);

	if (pExtHdlr && llid < EPON_LLID_MAX_NUM && state <= DS_SEND_ANY){
		pExtHdlr->dscvryState[llid] = state;
	}
}

void eponOamProcessAlarm(OamAlarmMsg_Ptr pAlarm){
	static unsigned int sSeqNum = 0;

	Buff_t txPktBuf = {0};
	eponOamLlid_t *pOam = NULL;
	oamPduHdr_t *pOamHdr = NULL;
	oamPacket_t *pTxOamPkt = NULL;
	ExtHandler_Ptr pExtHdlr = NULL;
	int ret = 0;

	pExtHdlr = findHandlerByOui(gOuiCtc);
	if (pExtHdlr == NULL){
		eponOamExtDbg(DBG_OAM_L1, "\n eponOamProcessAlarm pExtHdlr == NULL!\n");
		return ;
	}

	pTxOamPkt = eponOamAllocPkt();
	if (pTxOamPkt == NULL){
		eponOamExtDbg(DBG_OAM_L1, "\n eponOamProcessAlarm pTxOamPkt == NULL!\n");
		return ;
	}
	eponOamExtDbg(DBG_OAM_L4, "\neponOamProcessAlarm Begin\n");
	pTxOamPkt->llidIdx = 0; //use base llid
	txPktBuf.llidIdx = 0;
	txPktBuf.data = pTxOamPkt->data+sizeof(oamPduHdr_t) + sizeof(unsigned short); // sizeof(sSeqNum)

	pOam = &(eponOam.eponOamLlid[0]);
	pOamHdr = (oamPduHdr_t*)pTxOamPkt->data;
	
	eponOamInitHeader(pOamHdr, pOam, EPON_OAM_HDR_CODE_EVT_NOTIFY);
	if((pOam->localCriticalEvent == TRUE)||(pOam->localDyingGasp == TRUE)||(pOam->localLinkStatus == FALSE)){
		pTxOamPkt->pduReq = EPON_OAM_CRITICAL;
	}else{
		pTxOamPkt->pduReq = EPON_OAM_NORMAL;
	}

	setU16(pTxOamPkt->data+sizeof(oamPduHdr_t) , sSeqNum++);

	// CTC Org Handler 
	ret = pExtHdlr->buildEventData(&txPktBuf, (u_char *)pAlarm, sizeof(OamAlarmMsg_t));
	if (ret < 0){
		free(pTxOamPkt);
		return ;
	}
	
	pTxOamPkt->pduLen = sizeof(oamPduHdr_t) + sizeof(unsigned short) + txPktBuf.usedLen + 1;	
	pTxOamPkt->data[pTxOamPkt->pduLen] = 0; // end TLV
	eponOamExtDbg(DBG_OAM_L4, "\neponOamProcessAlarm End to Tx\n");
	eponOamPktTx(pTxOamPkt , pTxOamPkt->llidIdx);	
	free(pTxOamPkt);
}

