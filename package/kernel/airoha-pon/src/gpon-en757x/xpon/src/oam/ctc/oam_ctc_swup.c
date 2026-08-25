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
	oam_ctc_swup.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "../../../../../../tools/trx/trx.h"
#include "libtcapi.h"
#include "epon_oam.h"
#include "epon_oam_timer.h"
#include "ctc/oam_ctc_swup.h"
#include "ctc/oam_ext_cmn.h"
#include "ctc/oam_ctc_dspch.h"


extern OamCtcDB_t gCtcDB;
u_char actflag = FAIL;

#define STATE_SWUP_WAITNEXT 1
#define STATE_SWUP_END    0

#define STATE_NULL      0
#define STATE_WRITING   1
#define STATE_WRITEDONE 2
#define STATE_CRC_ERR   3

typedef struct ImageState_s{
	int upState;
	int upTimer;	// the time of sw up 
	int upTimerId;
	int image2FlashState;
	pthread_mutex_t mutex;
}ImageState_t, *ImageState_p;

ImageMgr_t imageBuff = {{0}, 0, 0, 0, NULL, NULL};
ImageState_t imageState = {
	STATE_SWUP_END, 
	30000,	// 30sec for download image timeout
	-1,
	STATE_NULL,
	PTHREAD_MUTEX_INITIALIZER	// mutex for imageBuff
};

inline void cleanImageMgr(ImageMgr_Ptr imp, int lock)
{
	ImageData_Ptr idp = NULL;
	int ret = 0;
	
	eponOamExtDbg(DBG_OAM_L4, "-_-: %s clean image buf.\n", __FUNCTION__);
	if (imp){
		if (lock ==TRUE){
			ret = pthread_mutex_trylock(&imageState.mutex);
			if (ret != 0){
				eponOamExtDbg(DBG_OAM_L1, "%s: OAM image try lock failed\n", __FUNCTION__);
				return ;
			}
		}
		
		strcpy(imp->imageName, "");
		imp->imageState = FILE_TRANSFER_STATE_NULL;
		imp->nextBlock = 1;
		imp->imageSize = 0;
		imp->currDataPtr = NULL;

		idp = imp->dataList;
		while(idp){
			imp->dataList = idp->next;
			free(idp);
			idp = imp->dataList;
		}
		imp->dataList = NULL;
		
		if (lock ==TRUE ){
			pthread_mutex_unlock(&imageState.mutex);
		}
	}
}


void *image2Flash(void *data)
{
	ImageMgr_Ptr imp = (ImageMgr_Ptr)data;

	eponOamExtDbg(DBG_OAM_L4, "-_-: %s write image data to flash.\n", __FUNCTION__);
	if (imp){		
		pthread_mutex_lock(&imageState.mutex);

		if (FAIL == imageCrcCheck(imp)){
			imageState.image2FlashState = STATE_CRC_ERR;
			cleanImageMgr(imp, FALSE);
		}else{
			writeImageBuff2Flash(imp);
			imageState.image2FlashState = STATE_WRITEDONE;
		}
		
		pthread_mutex_unlock(&imageState.mutex);
	}
	return NULL;
}

/* ------------------------------------Begin Ext Software Upgrade------------------------*/
inline void buildEndDownloadRPSCode(Buff_Ptr bfp, u_char rspCode)
{
	u_short tid = getTID(bfp->llidIdx);
	putBuffU8(bfp, SOFTWARE_UPGRADE_END_DOWNLOAD);
	putBuffU16(bfp, 8);//RPSCode msg's length
	putBuffU16(bfp, tid);
	putBuffU16(bfp, SW_UG_OPCODE_END_DOWNLOAD_RESPONSE);
	putBuffU8(bfp, rspCode);
	addBuffFlag(bfp, BF_NEED_SEND);

	eponOamExtDbg(DBG_OAM_L3, "<< Build OAM EndDownload Response: Data_Type: 0x%02X, length: %d, "
		"TID: 0x%04X, OpCode: 0x%04X, RSPCode: 0x%02X\r\n", SOFTWARE_UPGRADE_END_DOWNLOAD, 8, tid, \
		SW_UG_OPCODE_END_DOWNLOAD_RESPONSE, rspCode);
}

inline int procEndDownload(Buff_Ptr bfp, u_int fileSize)
{
	u_char rpsCode = RPS_CODE_OK;
	eponOamExtDbg(DBG_OAM_L2, ">> ImageUpgrade: End Download: File size = %d\r\n", fileSize);

	// process the last block length == 1400, if recv EndDownload info, start to write.
	if (imageState.upState == STATE_SWUP_WAITNEXT){
		pthread_t threadImage2Flash;

		imageState.upState = STATE_SWUP_END;
		eponOamCfg.maxPduPerSec= EPON_OAM_MAX_PDU_NUM_PER_SEC;
		imageBuff.imageState = FILE_TRANSFER_STATE_END;
		imageState.image2FlashState = STATE_WRITING;
		pthread_create(&threadImage2Flash, NULL, image2Flash, (void*)&imageBuff);
	}

	if (imageState.image2FlashState == STATE_WRITING){
		rpsCode = RPS_CODE_WRITING;
	}else if (imageState.image2FlashState == STATE_WRITEDONE){
		unlink(OAM_DOWNLOAD_SW_FILE);
		rpsCode = RPS_CODE_OK;
	}else if (imageState.image2FlashState == STATE_CRC_ERR){
		rpsCode = RPS_CODE_CRC_ERROR;
	}else{
		if (imageBuff.imageSize != fileSize){
			rpsCode = RPS_CODE_PARAM_ERROR;
			cleanImageMgr(&imageBuff, TRUE);
		}
	}
	buildEndDownloadRPSCode(bfp, rpsCode);
	return SUCCESS;
}

inline void buildActiveImageAck(Buff_Ptr bfp, u_char ack)
{
	u_short tid = getTID(bfp->llidIdx);
	putBuffU8(bfp, SOFTWARE_UPGRADE_ACTIVATE_IMG);
	putBuffU16(bfp, 8);//Active Ack msg's length
	putBuffU16(bfp, tid);
	putBuffU16(bfp, SW_UG_OPCODE_ACTIVATE_IMG_RESPONSE);
	putBuffU8(bfp, ack);
	addBuffFlag(bfp, BF_NEED_SEND);

	eponOamExtDbg(DBG_OAM_L3, "<< Build OAM ActiveImage ACK: Data_Type: 0x%02X, length: %d, "
		"TID: 0x%04X, OpCode: 0x%04X, ACK: 0x%02X\r\n", SOFTWARE_UPGRADE_ACTIVATE_IMG, 8, tid, \
		SW_UG_OPCODE_ACTIVATE_IMG_RESPONSE, ack);
}

inline int procActivateImage(Buff_Ptr bfp, u_char flag)
{
	u_char ack;
	eponOamExtDbg(DBG_OAM_L2, ">> ImageUpgrade: Activate Image: Flag = %d\r\n", flag);

	if (flag == 0x00){
		actflag = activateImage();
		if (actflag == SUCCESS){
			ack = SW_UP_ACK_OK;
			eponOamExtDbg(DBG_OAM_L0, ">> ImageUpgrade: activateImage success\r\n");
		}
		else{
			ack = SW_UP_ACK_LOAD_FAIL;
			eponOamExtDbg(DBG_OAM_L0, ">> ImageUpgrade: activateImage fail\r\n");
		}
	}else{
		ack = SW_UP_ACK_PARAM_ERROR;
	}

	buildActiveImageAck(bfp, ack);
	return SUCCESS;
}

inline void buildCommitImageAck(Buff_Ptr bfp, u_char ack)
{
	u_short tid = getTID(bfp->llidIdx);
	putBuffU8(bfp, SOFTWARE_UPGRADE_COMMIT_IMG);
	putBuffU16(bfp, 8);//Commit Ack msg's length
	putBuffU16(bfp, tid);
	putBuffU16(bfp, SW_UG_OPCODE_COMMIT_IMG_RESPONSE);
	putBuffU8(bfp, ack);
	addBuffFlag(bfp, BF_NEED_SEND);

	eponOamExtDbg(DBG_OAM_L3, "<< Build OAM CommitImage ACK: Data_Type: 0x%02X, length: %d, "
		"TID: 0x%04X, OpCode: 0x%04X, ACK: 0x%02X\r\n", SOFTWARE_UPGRADE_COMMIT_IMG, 8, tid, \
		SW_UG_OPCODE_COMMIT_IMG_RESPONSE, ack);
}

inline int procCommitImage(Buff_Ptr bfp, u_char flag)
{
	u_char ack;
	eponOamExtDbg(DBG_OAM_L2, ">> ImageUpgrade: Commit Image: Flag = %d\r\n", flag);

	if (flag == 0x00){
		if (commitImage() == SUCCESS)
			ack = SW_UP_ACK_OK;
		else
			ack = SW_UP_ACK_LOAD_FAIL;			
	}else
		ack = SW_UP_ACK_PARAM_ERROR;

	buildCommitImageAck(bfp, ack);
#if defined(TCSUPPORT_XPON_LED_UPGRADE)
	xpon_led("2");
#endif
	return SUCCESS;
}

inline void buildTransferAck(Buff_Ptr bfp, u_short block)
{
	u_short tid = getTID(bfp->llidIdx);
	putBuffU8(bfp, SOFTWARE_UPGRADE_FILE);
	putBuffU16(bfp, 9);//ACK msg's length
	putBuffU16(bfp, tid);
	putBuffU16(bfp, SW_UG_OPCODE_FILE_ACK);
	putBuffU16(bfp, block);
	addBuffFlag(bfp, BF_NEED_SEND);

	eponOamExtDbg(DBG_OAM_L3, "<< Build OAM FileTransfer ACK: Data_Type: 0x%02X, length: %d, "
		"TID: 0x%04X, OpCode: 0x%04X, ACK: %d\r\n", SOFTWARE_UPGRADE_FILE, 9, tid, \
		SW_UG_OPCODE_FILE_ACK, block);
}

inline void buildError(Buff_Ptr bfp, u_short errCode, char *errMsg)
{
	u_short tid = getTID(bfp->llidIdx);
	u_short len = 9;

	if (errCode == 0){
		len += strlen(errMsg) + 1;
	}

	putBuffU8(bfp, SOFTWARE_UPGRADE_FILE);
	putBuffU16(bfp, len);//error msg's length
	putBuffU16(bfp, tid);
	putBuffU16(bfp, SW_UG_OPCODE_FILE_ERROR);
	putBuffU16(bfp, errCode);
	addBuffFlag(bfp, BF_NEED_SEND);

	if (errCode == 0){
		putBuffData(bfp, (u_char*)errMsg, strlen(errMsg));
		putBuffU8(bfp, 0);
	}

	eponOamExtDbg(DBG_OAM_L3, "<< Build OAM FileTransfer Error: Data_Type: 0x%02X, length: %d, "
		"TID: 0x%04X, OpCode: 0x%04X, ErrorCode: %d\r\n", SOFTWARE_UPGRADE_FILE, len, tid, \
		SW_UG_OPCODE_FILE_ERROR, errCode);
}

/* ----------------------
 * |  1    Ext_Opcode=6 |
 * ----------------------  <-- data
 * |  1    Data_Type    |
 * ----------------------
 * |  2    length       |
 * ----------------------
 * |  2      TID        |
 * ----------------------
 * |  X    Msg Format   |
 * ----------------------  length = 5+X
 */
// we should add timeout
int procExtSoftwareUpgrade(Buff_Ptr bfp, u_char *pData, int length)
{
	u_char *ptr = pData;
	OamSoftwareUpgrade_Ptr swup = (OamSoftwareUpgrade_Ptr)pData;

	if (length < sizeof(OamSoftwareUpgrade_t)){
		return FAIL;
	}
	swup->length = ntohs(swup->length);
	swup->tid    = ntohs(swup->tid);
	swup->opCode = ntohs(swup->opCode);

	if ((length < swup->length) || (swup->length < sizeof(OamSoftwareUpgrade_t))){
		return FAIL;
	}

	// add save LLID for TID
	if (eponOam.eponOamLlid[bfp->llidIdx].llid == 0){
		eponOam.eponOamLlid[bfp->llidIdx].llid = swup->tid;
	}

	eponOamExtDbg(DBG_OAM_L2, ">> ImageUpgrade pdu: data type = 0x%02X,"
		"length = %d, TID = 0x%04X, opCode = 0x%04X\r\n", swup->dataType, \
		swup->length, swup->tid, swup->opCode);

	PTR_MOVE(ptr, sizeof(OamSoftwareUpgrade_t));
	//length -= sizeof(OamSoftwareUpgrade_t);
	length = (swup->length - sizeof(OamSoftwareUpgrade_t)); // length >= swup->length

	switch(swup->dataType){
		case SOFTWARE_UPGRADE_FILE:
		{
			procFileDownload(bfp, swup->opCode, ptr, length);
		}
		break;
		case SOFTWARE_UPGRADE_END_DOWNLOAD:
		{
			if (swup->opCode == SW_UG_OPCODE_END_DOWNLOAD_REQUEST){
				u_int nFileSize = GetU32(ptr);
				procEndDownload(bfp, nFileSize);
			}/*else{
				buildEndDownloadRPSCode(bfp, RPS_CODE_NOT_SUPPORT);
			}*/
		}
		break;
		case SOFTWARE_UPGRADE_ACTIVATE_IMG:
		{
			u_char  flag = GetU8(ptr);
			if (swup->opCode == SW_UG_OPCODE_ACTIVATE_IMG_REQUEST){
				procActivateImage(bfp, flag);
			}/*else{
				goto ERROR_RUN;
			}*/
		}
		break;
		case SOFTWARE_UPGRADE_COMMIT_IMG:
		{
			u_char  flag = GetU8(ptr);
			if (swup->opCode == SW_UG_OPCODE_COMMIT_IMG_REQUEST){
				procCommitImage(bfp, flag);
			}/*else{
				goto ERROR_RUN;
			}*/
		}
		break;
	}
	return SUCCESS;
/*ERROR_RUN:
	return FAIL;*/
}

/* ----------------------
 * |  2    Opcode       |
 * ----------------------  <-- pData
 * |  X    Data         |
 * ----------------------
 */
int procFileDownload(Buff_Ptr bfp, u_char opCode, u_char *pData, int length)
{
	u_char *ptr = pData;

	switch(opCode){
		case SW_UG_OPCODE_FILE_WRITE:
		{
			char *cName = (char*)ptr, *cMode = NULL;

			PTR_MOVE(ptr, strlen(cName)+1);
			cMode = (char*)ptr;

			eponOamExtDbg(DBG_OAM_L2, ">> ImageUpgrade: File Write: FileName = %s, Mode = %s\r\n", cName, cMode);

			if (strcmp(cMode, "Octet") != 0){
				buildError(bfp, ERR_ILLEGAL_SW_UG_MSG, NULL);
				break;
			}

			startDownloadImage(bfp, cName);
		}
		break;
		case SW_UG_OPCODE_FILE_TRANSFER:
		{
			u_short rcvBlock = GetU16(ptr);
			PTR_MOVE(ptr, 2);
			length -= 2;

			eponOamExtDbg(DBG_OAM_L2, ">> ImageUpgrade: File Transfer: Block = %d, Data length = %d\r\n", rcvBlock, length);

			procFileTransferData(bfp, rcvBlock, ptr, length);
		}
		break;
		default:
		buildError(bfp, ERR_ILLEGAL_SW_UG_MSG, NULL);
		break;
	}
	return SUCCESS;
}


/* ------------------------------------End Ext Software Upgrade--------------------------*/
void startDownloadImage(Buff_Ptr bfp, char* imageName)
{
	int ret = SUCCESS;
	u_short errCode = ERR_OTHER;
	char errMsg[MAX_ERR_MSG_LEN] = {0};

	cleanImageMgr(&imageBuff, TRUE);
	strcpy(imageBuff.imageName, imageName);

	// TODO: we need to check if the image exist

	if (ret == SUCCESS){
		buildTransferAck(bfp, 0);
		imageState.image2FlashState = STATE_NULL;
		eponOamCfg.maxPduPerSec = 10000; // when start to download fw by oam, not to limit to 10p/s
		eponOamExtDbg(DBG_OAM_L0, ">> OAM: Start to download image: %s\n", imageName);
#if defined(TCSUPPORT_XPON_LED_UPGRADE)
	xpon_led("3");
#endif
		
	}else{
		strcpy(errMsg, "unknown");
		buildError(bfp, errCode, errMsg);
	}
}

int procFileTransferData(Buff_Ptr bfp, u_short block, u_char *data, int length)
{
	ImageData_Ptr idp = NULL;

	if (block == 1){
		imageBuff.imageState = FILE_TRANSFER_STATE_BEGIN;
	}
	if (imageBuff.imageState == FILE_TRANSFER_STATE_END){
		eponOamExtDbg(DBG_OAM_L1,">> Warning: Recv block:%u at STATE_END!\n", block);

		if (imageBuff.nextBlock-1 == block){
			buildTransferAck(bfp, block);
		}else{
			buildError(bfp, ERR_OTHER, "Recv data when transfer end!");
		}
		//cleanImageMgr(&imageBuff, TRUE); // ReadMe: need ?
		return SUCCESS;
	}

	if (block == imageBuff.nextBlock){
		// if the last block length == 1400, then when recv EndDownload msg to write.
		if (length == SW_UP_FILE_DATA_LEN_1400){
			imageBuff.nextBlock ++;
		}else if (length < SW_UP_FILE_DATA_LEN_1400){
			imageBuff.imageState = FILE_TRANSFER_STATE_END;
			imageBuff.nextBlock ++;
		}else{
			buildError(bfp, ERR_ILLEGAL_SW_UG_MSG, NULL);
			cleanImageMgr(&imageBuff, TRUE); // ReadMe: need ?
			return SUCCESS;
		}

		idp = (ImageData_Ptr)malloc(sizeof(ImageData_t));
		if (idp == NULL){
			buildError(bfp, ERR_BUFF_FULL, NULL);
			cleanImageMgr(&imageBuff, TRUE); // ReadMe: need ?
			return SUCCESS;
		}
		memset(idp, sizeof(ImageData_t), 0);
		idp->index = block;
		idp->len   = length;
		idp->next  = NULL;
		memcpy(idp->data, data, length);
		imageBuff.imageSize += length;

		if (imageBuff.currDataPtr == NULL){
			imageBuff.dataList    = idp;
			imageBuff.currDataPtr = idp;
		}else{
			imageBuff.currDataPtr->next = idp;
			imageBuff.currDataPtr       = idp;
		}

		buildTransferAck(bfp, block);
	}else if (block == imageBuff.nextBlock - 1){
		buildTransferAck(bfp, block);
	}else{
		// skip this data? block > nextBlock? block < nextBlock-1 ?
		eponOamExtDbg(DBG_OAM_L1,">>Warning: Recv block:%u, nextBlock:%u, skiped!\n", block, imageBuff.nextBlock);
		return SUCCESS;
	}
	
	if (imageState.upTimerId > 0){
		timerStopS(imageState.upTimerId);
		imageState.upTimerId = 0;
	}
	if (imageBuff.imageState == FILE_TRANSFER_STATE_BEGIN){
		imageState.upState = STATE_SWUP_WAITNEXT;
		
		imageState.upTimerId= timerStartS(imageState.upTimer, oamSwDownloadTimeout, &imageBuff);
		
		if (imageState.upTimerId < 0){
			eponOamExtDbg(DBG_OAM_L1, ">> procFileTransferData: timerStartS fail!.\n");
		}
	}else if (imageBuff.imageState == FILE_TRANSFER_STATE_END){
		// write file & flash is slow, write before end download call.
		pthread_t threadImage2Flash;
		imageState.image2FlashState = STATE_WRITING;
		imageState.upState = STATE_SWUP_END; // to skip end download's write flash thread
		eponOamCfg.maxPduPerSec = EPON_OAM_MAX_PDU_NUM_PER_SEC;
		pthread_create(&threadImage2Flash, NULL, image2Flash, (void*)&imageBuff);
	}
	return SUCCESS;
}

int oamSwDownloadTimeout(void *param)
{
	int ret = SUCCESS;
	ImageMgr_Ptr pImageBuff = (ImageMgr_Ptr)param;
	
	if (imageState.upState == STATE_SWUP_WAITNEXT){
		// the timer out 
		eponOamExtDbg(DBG_OAM_L1, ">> oam SwDownload Timeout !\n");
		imageState.upState = STATE_SWUP_END;
		eponOamCfg.maxPduPerSec = EPON_OAM_MAX_PDU_NUM_PER_SEC;
		if (pImageBuff){
			cleanImageMgr(pImageBuff, TRUE);
		}
	}
#if defined(TCSUPPORT_XPON_LED_UPGRADE)
	xpon_led("2");
#endif
		
	return ret;
}

/* from here need to implement these functions 
 * reference: CONFIG_DUAL_IMAGE & TCSUPPORT_DUAL_IMAGE_ENHANCE */

// TODO: MultiLLID
u_short getTID(u_char llidIdx)
{
	u_short baseLlid = 0;

	if (eponOam.eponOamLlid[0].enableFlag == 1)
		baseLlid = eponOam.eponOamLlid[0].llid;
	else
		baseLlid = eponOam.eponOamLlid[llidIdx].llid;
	return baseLlid;
}


/* CRC Computer */
/* 
  Table of CRC-32's of all single-byte values (made by make_crc_table) 
*/  
static unsigned long crcTable[256] = {  
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

#define UPDC32(octet,crc) (crcTable[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))

int imageCrcCheck(ImageMgr_Ptr imp)
{
	int ret = FAIL;
	ImageData_Ptr idp = imp->dataList;
	unsigned long crc = 0xFFFFFFFF, crcImg = 0, i;
	struct trx_header *trx_p = NULL;

	// get crc from firmware file and to check it.
	if (idp && (idp->len > sizeof(struct trx_header))){
		trx_p = (struct trx_header *)(idp->data);
		
		if (trx_p->magic == TRX_MAGIC2 && trx_p->len == imp->imageSize){
			crcImg = trx_p->crc32;
			
			// crc check not contain trx_header
			for(i = sizeof(struct trx_header); i < idp->len; i++)
				crc = UPDC32(idp->data[i], crc);
			idp = idp->next;
			
			while(idp){
				for (i = 0; i < idp->len; i++){
					crc = UPDC32(idp->data[i], crc);
				}
				idp = idp->next;
			}
			
			eponOamExtDbg(DBG_OAM_L0, ">>OAM: ImageCrcCheck: image crc: %x; real crc: %x.\n", crcImg, crc);
			
			if (crcImg == crc){
				ret = SUCCESS;
			}
		}		
	}
	
	eponOamExtDbg(DBG_OAM_L0, ">>OAM: ImageCrcCheck:%s.\n", (ret==SUCCESS)?"OK":"Error");
	return ret;
}

/* Make the backup image to the boot */
int activateImage()
{
	int image0_valid, image1_valid, main_image;

	image0_valid = gCtcDB.swup.valid0;	
	image1_valid = gCtcDB.swup.valid1;
	main_image= gCtcDB.swup.main;
	
	if (image0_valid == 1 && image1_valid == 1){
		if (main_image == 0 || main_image == 1){
			gCtcDB.swup.active = 1;
			if (TCAPI_SUCCESS != tcapi_set(EPON_SOFTIMAGE, "Active", "1"))
				return FAIL;
		}else{
			return FAIL;
		}
	}else
		return FAIL;
	
	return SUCCESS;
}

/* Make the backup image to main image */
int commitImage()
{
	int image0_valid, image1_valid, main_image;
	char buf[5] = {0};

	image0_valid = gCtcDB.swup.valid0;	
	image1_valid = gCtcDB.swup.valid1;
	main_image= gCtcDB.swup.main;
	
	if (image0_valid == 1 && image1_valid == 1){
		if (main_image == 0)
			strcpy(buf, "1");
		else if (main_image == 1)
			strcpy(buf, "0");
		else 
			return FAIL;
		
		if (TCAPI_SUCCESS != tcapi_set(EPON_SOFTIMAGE, "Main", buf))
			return FAIL;
		
		gCtcDB.swup.main = atoi(buf);
		
		// no need to restart cpe
		if (TCAPI_SUCCESS != tcapi_save())
			return FAIL;
		if (TCAPI_SUCCESS != tcapi_commit(EPON_SOFTIMAGE))
			return FAIL;
	}else 
		return FAIL;
	
	return SUCCESS;
}

/* Write to the second image flash!
 * So we need get the second image addr. */
int writeImageBuff2Flash(ImageMgr_Ptr imp)
{
	FILE *fp = NULL;
	ImageData_Ptr idp;
	char mtdLabel[20] = {0}, buf[5] = {0}, mtdCmd[256] = {0}, version[256] = {0};
	u_int offset = 0, length = 0;

	length = imp->imageSize;
	fp = fopen(OAM_DOWNLOAD_SW_FILE, "w+");
	if (fp == NULL){
		eponOamExtDbg(DBG_OAM_L0, ">> OAM: Can't create FILE: %s! And clean Image Buff!\n", OAM_DOWNLOAD_SW_FILE);
		cleanImageMgr(imp, FALSE);
		return FAIL;
	}
	eponOamExtDbg(DBG_OAM_L0, ">> OAM: Start to Write firmware to file %s...\r\n", OAM_DOWNLOAD_SW_FILE);

	idp = imp->dataList;
	while(idp){
		fwrite(idp->data, sizeof(u_char), idp->len, fp);
		idp = idp->next;
		
		free(imp->dataList);
		imp->dataList = idp;
	}
	fclose(fp);

	strcpy(version, imp->imageName);
	cleanImageMgr(imp, FALSE);
	
	eponOamExtDbg(DBG_OAM_L0, ">> OAM: Write firmware to File OK.\n");

	eponOamExtDbg(DBG_OAM_L4, ">> OAM: tcapi get SoftImage_Main %d.\n",  gCtcDB.swup.main);
	
	if ( gCtcDB.swup.main == 0){
		gCtcDB.swup.valid1 = 1;
		strcpy(mtdLabel, "tclinux_slave");
		tcapi_set(EPON_SOFTIMAGE, "Valid1", "1");
		tcapi_set(EPON_SOFTIMAGE, "Version1", version);
	}
	else /*if ( gCtcDB.swup.main == 1)*/{
		gCtcDB.swup.valid0 = 1;
		strcpy(mtdLabel, "tclinux");
		tcapi_set(EPON_SOFTIMAGE, "Valid0", "1");
		tcapi_set(EPON_SOFTIMAGE, "Version0", version);
	}
	
	eponOamExtDbg(DBG_OAM_L4, ">> OAM: tcapi set SoftImage_Valid && mtdLabel=%s.\n", mtdLabel);

	sprintf(mtdCmd, FW_WRITE_CMD, OAM_DOWNLOAD_SW_FILE, length, offset, mtdLabel);
	eponOamExtDbg(DBG_OAM_L4, ">> OAM: %s\n", mtdCmd);
	system(mtdCmd);

	// unlink(OAM_DOWNLOAD_SW_FILE);

	eponOamExtDbg(DBG_OAM_L0, ">> OAM: Write firmware to flash OK!\r\n");

	return SUCCESS;
}


