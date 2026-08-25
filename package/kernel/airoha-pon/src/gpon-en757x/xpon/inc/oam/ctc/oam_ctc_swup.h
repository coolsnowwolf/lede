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
	oam_ctc_swup.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef OAM_CTC_SOFTWARE_UPGRADE_H
#define OAM_CTC_SOFTWARE_UPGRADE_H
#include "../epon_oam_types.h"
#include "ctc/oam_ext_buff.h"

#define OAM_DOWNLOAD_SW_FILE  "/tmp/oamfw"
#define FW_WRITE_CMD "/userfs/bin/mtd -f write %s %d %d %s &"

#define EPON_SOFTIMAGE  	"EPON_SoftImage"


#define IMAGE_NAME_MAX_LEN  256

#define FILE_TRANSFER_STATE_NULL  0
#define FILE_TRANSFER_STATE_BEGIN 1
#define FILE_TRANSFER_STATE_END   2

/* OAM EXT CTC Software Upgrade DEFINE */
#define SOFTWARE_UPGRADE_FILE              0x01
#define SOFTWARE_UPGRADE_END_DOWNLOAD      0x02
#define SOFTWARE_UPGRADE_ACTIVATE_IMG      0x03
#define SOFTWARE_UPGRADE_COMMIT_IMG        0x04

/* Software upgrade opcode */
#define SW_UG_OPCODE_FILE_WRITE            0x0002
#define SW_UG_OPCODE_FILE_TRANSFER         0x0003
#define SW_UG_OPCODE_FILE_ACK              0x0004
#define SW_UG_OPCODE_FILE_ERROR            0x0005
#define SW_UG_OPCODE_END_DOWNLOAD_REQUEST  0x0006
#define SW_UG_OPCODE_END_DOWNLOAD_RESPONSE 0x0007
#define SW_UG_OPCODE_ACTIVATE_IMG_REQUEST  0x0008
#define SW_UG_OPCODE_ACTIVATE_IMG_RESPONSE 0x0009
#define SW_UG_OPCODE_COMMIT_IMG_REQUEST    0x000A
#define SW_UG_OPCODE_COMMIT_IMG_RESPONSE   0x000B

/* File Downlaod Error MSG */
#define MAX_ERR_MSG_LEN       128
#define ERR_OTHER             0x0000
#define ERR_BUFF_FULL         0x0003
#define ERR_ILLEGAL_SW_UG_MSG 0x0004
#define ERR_IMAGE_EXIST       0x0006

/* End Download Response RPSCode */
#define RPS_CODE_OK          0x00
#define RPS_CODE_WRITING     0x01
#define RPS_CODE_CRC_ERROR   0x02
#define RPS_CODE_PARAM_ERROR 0x03
#define RPS_CODE_NOT_SUPPORT 0x04

/* Activate/Commit Image Response */
#define SW_UP_ACK_OK          0x00
#define SW_UP_ACK_PARAM_ERROR 0x01
#define SW_UP_ACK_NOT_SUPPORT 0x02
#define SW_UP_ACK_LOAD_FAIL   0x03

#define SW_UP_FILE_DATA_LEN_MAX     1400
#define SW_UP_FILE_DATA_LEN_1400    1400

#pragma pack(push, 1)

/* OAM CTC Software Upgrade Header */
typedef struct oamSoftwareUpgrade_s
{
	u_char 	dataType;
	u_short  length;
	u_short  tid;
	u_short  opCode;
}OamSoftwareUpgrade_t, *OamSoftwareUpgrade_Ptr;

#pragma pack(pop)

typedef struct imageData_s
{
	struct imageData_s * next;
 	u_int   index;  // the block index of the data
	u_int   len;
	u_char  data[1400];
}ImageData_t, *ImageData_Ptr;

typedef struct imageMgr_s
{
	char   imageName[IMAGE_NAME_MAX_LEN]; // image file name
	u_char  imageState;   // the image file transfer state
	u_short nextBlock;	 // the next block num we want
	u_int   imageSize;
	ImageData_Ptr currDataPtr;
	ImageData_Ptr dataList;  // image data list
}ImageMgr_t, *ImageMgr_Ptr;


int procExtSoftwareUpgrade(Buff_Ptr bfp, u_char *pData, int length);
int procFileDownload(Buff_Ptr bfp, u_char opCode, u_char *pData, int length);
int procEndDownload(Buff_Ptr bfp, u_int fileSize);
int procActivateImage(Buff_Ptr bfp, u_char flag);
int procCommitImage(Buff_Ptr bfp, u_char flag);

void buildTransferAck(Buff_Ptr bfp, u_short block);
void buildError(Buff_Ptr bfp, u_short errCode, char *errMsg);
void buildEndDownloadRPSCode(Buff_Ptr bfp, u_char rspCode);
void buildActiveImageAck(Buff_Ptr bfp, u_char ack);
void buildCommitImageAck(Buff_Ptr bfp, u_char ack);

u_short getTID(u_char llidIdx);
void startDownloadImage(Buff_Ptr bfp, char* imageName);
int procFileTransferData(Buff_Ptr bfp, u_short block, u_char *data, int length);
int writeImageBuff2Flash(ImageMgr_Ptr imp);
int oamSwDownloadTimeout(void *param);

int imageCrcCheck(ImageMgr_Ptr imp);
int activateImage();
int commitImage();

#endif
