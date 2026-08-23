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
	oam_ext_cmn.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include "epon_oam.h"
#include "ctc/oam_ext_cmn.h"

int eponOamCtcDbgLvl = DBG_OAM_L0;

inline void setU8(u_char *dp, u_char value)
{
	*dp = value;
}
inline void setU16(u_char *dp, u_short value)
{
	*((u_short*)dp) = htons(value);
}
inline void setU24(u_char *dp, u_char *value)
{
	*dp++ = *value++;
	*dp++ = *value++;
	*dp = *value;
}
inline void setU32(u_char *dp, u_int value)
{
	*((u_int*)dp) = htonl(value);
}

inline int match(u_char *dp1, u_char *dp2, int length)
{
	int i = 0;
	for (; i < length; i++)
	{
		if (dp1[i] != dp2[i])
			return FALSE;
	}
	return TRUE;
}

/*
 * *to: the string copyed to
 * *from: the string copyed from
 *  lenTo: the length (*to)
 */
inline int lowAlignStrcpy(char *to, char *from, int lenTo)
{
	int rl = lenTo - strlen(from);
	int s = MAX(0, rl), i;

    memset(to, 0, lenTo);
	for(i = s; i < lenTo; i++){
		to[i] = from[i - s];
	}
	return lenTo - s;
}


inline int eponOamCtcGetDbgLevel(void){
	return eponOamCtcDbgLvl;
}

inline void eponOamCtcSetDbgLevel(int lvl){
	eponOamCtcDbgLvl = lvl;
}

void eponOamDumpHexString(int level, u_char *pData, int len){
#ifdef CONFIG_DEBUG
	int i = 0;
	if (level > eponOamCtcDbgLvl)
		return ;

	for (i = 0; i < len; i++){
		if (i %16 == 0){
			tcdbg_printf("\n   ");
		}
		tcdbg_printf("%.2x ", pData[i]);
	}
	tcdbg_printf("\n");
#endif	
}


int isSameOui(const u_char *oui1, const u_char *oui2)
{
	return ((oui1[0] == oui2[0]) &&
			(oui1[1] == oui2[1]) &&
			(oui1[2] == oui2[2]) );
}

void handleBuffNotEnough(INOUT Buff_Ptr bfp, IN int revLen)
{
	oamPacket_t *pTxOamPkt = NULL;
	eponOamLlid_t *pOam = NULL;
	oamPduHdr_t *pOamHdr_p;

	if (bfp->flag & BF_NEED_SEND){
		pOam = &(eponOam.eponOamLlid[bfp->llidIdx]);
		pTxOamPkt = eponOamAllocPkt();
		if (pTxOamPkt == NULL){
			eponOamExtDbg(DBG_OAM_L0, "\r\neponOamAllocPkt return NULL!\n");
			return;
		}
		pTxOamPkt->llidIdx = bfp->llidIdx;
		pOamHdr_p = (oamPduHdr_t*)pTxOamPkt->data;
		eponOamInitHeader(pOamHdr_p, pOam ,EPON_OAM_HDR_CODE_ORG_SPEC);
		
		if((pOam->localCriticalEvent == TRUE)||(pOam->localDyingGasp == TRUE)||(pOam->localLinkStatus == FALSE)){
			pTxOamPkt->pduReq = EPON_OAM_CRITICAL;
		}else{
			pTxOamPkt->pduReq = EPON_OAM_NORMAL;
		}
		memcpy(pTxOamPkt->data+sizeof(oamPduHdr_t), bfp->data, bfp->usedLen);
#if 0
		pTxOamPkt->pduLen = sizeof(oamPduHdr_t) + bfp->usedLen+1; // 1 is for end tlv
		pTxOamPkt->data[pTxOamPkt->pduLen] = 0; // end TLV
#else			
		pTxOamPkt->pduLen = sizeof(oamPduHdr_t) + bfp->usedLen;
#endif
		
#ifdef TCSUPPORT_EPON_OAM_LAN_DBG
		if (bfp->flag & BF_FROM_LAN)
			pTxOamPkt->fromLan = TRUE;
#endif
		
		eponOamPktTx(pTxOamPkt , pTxOamPkt->llidIdx);
		free(pTxOamPkt);
	}
	bfp->usedLen = revLen;
	delBuffFlag(bfp, BF_NEED_SEND);
}


