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
	oam_ctc_dba.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/
#include <string.h>
#include "libepon.h"
#include "ctc/oam_ctc_dba.h"

/* ----------------------<-- pData
 * |  1      dba code  |
 * ----------------------
 * |  X      dba data  |    length = X+1
 * ----------------------
 */

int procExtDBA(Buff_Ptr bfp, u_char *pData, int length)
{
	int ret = SUCCESS;
	u_char dbaCode = *pData;

	if (length < 1)
		return FAIL;

	PTR_MOVE(pData, 1);
	length -= 1;

	eponOamExtDbg(DBG_OAM_L2, ">> DBA code:0x%02X\n", dbaCode);

	switch(dbaCode){
	case OAM_CTC_GET_DBA_REQUEST:
		ret = buildGetDBAResponse(bfp);
		break;
	case OAM_CTC_SET_DBA_REQUEST:
		ret = buildSetDBAResponse(bfp, pData, length);
		break;
	default:
		return FAIL;
		break;
	}
	return ret;
}


/* ------------------------
 * | 1   DBA_code = 0x01  |
 * ------------------------
 * | 1  num_of_queue_sets |
 * ------------------------
 * | 1   report_bitmap    |
 * ------------------------
 * |0/2 queue #0 Threshold|
 * ------------------------
 * |0/2     ....          |
 * ------------------------
 * |0/2 queue #7 Threshold|
 * ------------------------
 * */
int buildGetDBAResponse(Buff_Ptr bfp)
{
	u_char llidIdx = bfp->llidIdx;
	u_char queueSetsNum = 0, i, j, reportQueueNum;
	u_char mask = 0xff, reportBitmap = 0;
	u_short thresholds[8], reportThresholds[8];

	putBuffU8(bfp, OAM_CTC_GET_DBA_RESPONSE);
	
	eponApiGetLlidDBAThrshldNum(llidIdx, &queueSetsNum);// from jq suggest, the threshold num is 0~3.

	queueSetsNum += 1; // queueset num = threshold +1 

	eponOamExtDbg(DBG_OAM_L4, ">> DBA queue_sets_num: %hhu\n", queueSetsNum);
	if (queueSetsNum >= 1 && queueSetsNum < 5){
		if (queueSetsNum == 1){
			putBuffU8(bfp, 2); // queuesetnum is 2~4
			putBuffU8(bfp, 0x00); // reportBitmap 0 = 0x00
		}else{
			putBuffU8(bfp, queueSetsNum);
			eponOamExtDbg(DBG_OAM_L3, "<< Build DBA queue_sets_num: %hhu\n", queueSetsNum);
			
			for (i = 0; i < queueSetsNum-1; i ++){
				reportBitmap = 0;
				reportQueueNum = 0;
				eponApiGetLlidDBAThrshld(llidIdx, i, mask, thresholds);
				
				for (j = 0; j < 8; j++){
					if (thresholds[j] > 0){
						reportBitmap |= (1<<j);
						reportThresholds[reportQueueNum++] = htons(thresholds[j]);
						eponOamExtDbg(DBG_OAM_L3, "<< Build DBA sets:%hhu queue:%hhu threshold:%hhu\n", i, j, thresholds[j]);
					}
				}

				putBuffU8(bfp, reportBitmap);
				eponOamExtDbg(DBG_OAM_L3, "<< Build DBA queue_set %hhu report_map: %hhu\n", i, reportBitmap);
				if (reportQueueNum > 0){
					putBuffData( bfp, (u_char *)reportThresholds, reportQueueNum*sizeof(u_short));
				}
			}
		}
		// last queueset 
		putBuffU8(bfp, 0x00); // reportBitmap queuesetnum-1 0x00
		addBuffFlag(bfp, BF_NEED_SEND);
	}

	return SUCCESS;
}


/* ------------------------          ------------------------
 * | 1   DBA_code = 0x02  |          | 1   DBA_code = 0x03  |
 * ------------------------          ------------------------
 * | 1  num_of_queue_sets |          | 1  set_ACK = 0/1     |
 * ------------------------          ------------------------
 * | 1   report_bitmap    |          | 1  num_of_queue_sets |
 * ------------------------   ====>  ------------------------
 * |0/2 queue #0 Threshold|          |0/2  report_bitmap    |
 * ------------------------          ------------------------
 * |0/2     ....          |          |0/2 queue #0 Threshold|
 * ------------------------          ------------------------
 * |0/2 queue #7 Threshold|          |0/2     ....          |
 * ------------------------          ------------------------
 *                                   |0/2 queue #7 Threshold|
 *                                   ------------------------
 * */
int buildSetDBAResponse(Buff_Ptr bfp, u_char *pData, int length)
{
	u_char queueSetsNum = GetU8(pData);
	u_char reportBitmap;
	u_short queueThreshold[8];
	u_char i,j, bfpOf, offset = 1;
	int ret, ack = OAM_CTC_DBA_ACK;
	u_char llid = bfp->llidIdx;

	eponOamExtDbg(DBG_OAM_L2, ">> DBA Num of QueueSets:0x%02X\n", queueSetsNum);
	if (queueSetsNum > 0x04 || queueSetsNum < 0x02)
		return FAIL;

	putBuffU8( bfp, OAM_CTC_SET_DBA_RESPONSE);
	
	bfpOf = bfp->usedLen; // for set ACK/NAK
	bfp->usedLen += 1;
	
	putBuffU8(bfp, queueSetsNum);
	
	if (FAIL == eponApiSetLlidDBAThrshldNum(llid, queueSetsNum - 1)) // threshold um = queuesetnum -1
		ack = OAM_CTC_DBA_NACK;


	for (i = 0; i < queueSetsNum; i++){
		reportBitmap = GetU8(pData+offset);
		offset += 1;
		
		eponOamExtDbg(DBG_OAM_L2, "  >> DBA ReportBitmap %d:0x%02X\n", i, reportBitmap);
		putBuffU8(bfp, reportBitmap);
		memset(queueThreshold, 0, sizeof(u_short)*8);
		for (j = 0; j < 8; j++){
			if ((reportBitmap>>j)&0x01){
				queueThreshold[j] = GetU16(pData+offset);
				offset += 2;
				
				eponOamExtDbg(DBG_OAM_L2, "  >>    Queue #%d Threshold: %d\n", j, queueThreshold[j]);
				putBuffU16(bfp, queueThreshold[j]);
			}
		}
		if (reportBitmap != 0){
			ret = eponApiSetLlidDBAThrshld(llid, i, reportBitmap, queueThreshold);
			if (ret == FAIL){
				ack = OAM_CTC_DBA_NACK;
			}
		}
	}
	
	eponOamExtDbg(DBG_OAM_L3, "<< Build DBA Code:0x%02X\n", OAM_CTC_SET_DBA_RESPONSE);
	eponOamExtDbg(DBG_OAM_L3, "<< Build DBA Num of QueueSets:0x%02X\n", queueSetsNum);
	eponOamExtDbg(DBG_OAM_L3, "<< Build DBA Set Ack:%s\n", (ack==OAM_CTC_DBA_NACK)?"NACK":"ACK");
	
	*(bfp->data + bfpOf) = ack;
	addBuffFlag(bfp, BF_NEED_SEND);
	return SUCCESS;
}


