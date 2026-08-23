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
	oam_ctc_churning.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#include <stdlib.h>
#include <time.h>
#include "libepon.h"
#include "epon_oam.h"
#include "ctc/oam_ctc_churning.h"
#include "ctc/oam_sys_api.h"

int ignoreReqCnt = 0;
extern eponOam_t eponOam;

/* ----------------------
 * | 1    Ext.Opcode    |
 * ---------------------- <--pData
 * | 1   churning code  |  0x00
 * ----------------------
 * | 1 In-use_key index |
 * ----------------------
 * */
int procExtChurning(Buff_Ptr bfp, u_char *pData, int length)
{
	OamCtcChurning_Ptr ccp = (OamCtcChurning_Ptr)pData;

	if (length < sizeof(OamCtcChurning_t)){
		eponOamExtDbg(DBG_OAM_L2, "rcv a error churning OAM PDU!\t length is less!\r\n");
		return FAIL;
	}

	eponOamExtDbg(DBG_OAM_L2, ">> Churning: code = %x,"
		" In-use_key_index = 0x%02X\r\n", ccp->code, ccp->keyIndex);

	if (OAM_CTC_CHURNING_NEW_KEY_REQUEST == ccp->code)
	{
		if ((ccp->keyIndex == 0x00) || (ccp->keyIndex == 0x01)){
			buildCtcChurningKeyResponse(bfp, ccp->keyIndex);
			return  SUCCESS;
		}
	}

	return FAIL;
}

/* -----------------------
 * | 1   churning code   | 0x01
 * -----------------------
 * | 1   new key index   |
 * -----------------------
 * | 3   churning key    |  1G-EPON/10G-EPON
 * -----------------------
 * | 3   churning key2   |  10G-EPON
 * -----------------------
 * | 3   churning key3   |  10G-EPON
 * -----------------------
 * */
int buildCtcChurningKeyResponse(Buff_Ptr bfp, u_char keyIdx)
{
	int  isSetSameKey = 0;
	eponOamLlid_t *pOamLlid = &(eponOam.eponOamLlid[bfp->llidIdx]);

	keyIdx = abs(keyIdx-1); // for 
	
	putBuffU8(bfp, OAM_CTC_CHURNING_NEW_CHURING_KEY);
	putBuffU8(bfp, keyIdx);

	if(pOamLlid->lastKeyInUse >= 0)
	{
		if(pOamLlid->lastKeyInUse == keyIdx)
		{
			ignoreReqCnt ++;
			isSetSameKey = 1;
			eponOamExtDbg(DBG_OAM_L1, ">> Churning: set same key llid:%d new_key_index=%d, ignore=%d\n",bfp->llidIdx,keyIdx,ignoreReqCnt);
		}
	}
	pOamLlid->lastKeyInUse = keyIdx;

	if (EPON_MODE_1G == getEponMode()){
		u_char key[3];
		if(isSetSameKey)
		{
			isSetSameKey = 0;
			memcpy(key,pOamLlid->lastSettingKey,3);
		}
		else
		{
			getChurningKey(bfp->llidIdx, 1, key);
		}
		memcpy(pOamLlid->lastSettingKey,key,3);
		putBuffU24(bfp, key);

		eponApiSetLlidkey(bfp->llidIdx, keyIdx,key);
		
		eponOamExtDbg(DBG_OAM_L3, "<< Build CTC churning Key: llid:%d , code: 0x%02X, "
			"new_key_index: 0x%02X, key: 0x%02X%02X%02X\r\n", \
			bfp->llidIdx,OAM_CTC_CHURNING_NEW_CHURING_KEY, keyIdx, key[0], key[1], key[2]);
	}else if (EPON_MODE_10G == getEponMode()){
		u_char key1[3], key2[3], key3[3];
		if(isSetSameKey)
		{
			isSetSameKey = 0;
			memcpy(key1,pOamLlid->lastSettingKey,3);
			memcpy(key2,pOamLlid->lastSettingKey2,3);
			memcpy(key3,pOamLlid->lastSettingKey3,3);
		}
		else
		{
			getChurningKey(bfp->llidIdx, 1, key1);
			getChurningKey(bfp->llidIdx, 2, key2);
			getChurningKey(bfp->llidIdx, 3, key3);
		}
		memcpy(pOamLlid->lastSettingKey,key1,3);
		memcpy(pOamLlid->lastSettingKey2,key2,3);
		memcpy(pOamLlid->lastSettingKey3,key3,3);
		putBuffU24(bfp, key1);
		putBuffU24(bfp, key2);
		putBuffU24(bfp, key3);

		eponOamExtDbg(DBG_OAM_L3, "<< Build CTC churning Key: llid:%d , code: 0x%02X, "
			"new_key_index: 0x%02X, key1: 0x%02X%02X%02X, key2: 0x%02X%02X%02X, "
			"key3 = 0x%02X%02X%02X\r\n",bfp->llidIdx, OAM_CTC_CHURNING_NEW_CHURING_KEY,\
			keyIdx, key1[0], key1[1], key1[2], key2[0], key2[1], key2[2],\
			key3[0], key3[1], key3[2]);
	}
	addBuffFlag(bfp, BF_NEED_SEND);
	return SUCCESS;
}


/* TODO: need implement these functions below here */
inline u_char getEponMode()
{
	/*char buf[10] = {0};

	if (0 == tcapi_get("EPON_ONU", "Mode", buf)){
		if (0 == strcmp(buf, "10G")){
			return EPON_MODE_10G;
		}
	}*/
	return EPON_MODE_1G; // default value
}

void getChurningKey(u_char llid, u_char index, u_char key[3])
{
	unsigned int ukey;
	time_t t;
	
	srand((unsigned)time(&t));
	ukey = rand();
	
	key[0] = (ukey>>3) + index + llid + 8;
	key[1] = (ukey>>2) + index + llid + 45;
	key[2] = (ukey>>1) + index + llid + 12;
}


