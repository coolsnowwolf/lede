/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _GPON_PLOAM_H
#define _GPON_PLOAM_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include "gpon_ploam_raw.h"

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define PLOAM_UP_MSG_LENGTH						(11)
#define PLOAM_DOWN_MSG_LENGTH					(13)

#define PLOAM_DISABLE_DENIED_SPECIFIC			(0xFF)
#define PLAOM_DISABLE_ALLOWED_SPECIFIC			(0X00)
#define PLOAM_DISABLE_DENIED_ALL 				(0x0F)
#define PLOAM_DISABLE_DSICOVERY 				(0x3F)
#define PLAOM_DISABLE_ALLOWED_ALL				(0xF0)

#define PLOAM_EQD_RELATIVE 0
#define PLOAM_EQD_ABSOLUTE 1
#define PLOAM_EQD_POSITIVE 0
#define PLOAM_EQD_NEGATIVE 1

#define PLOAM_ALLOC_ID_ASSIGN     				1
#define PLOAM_ALLOC_ID_DEALLOCATE 				(0xFF)

#define NGPON2_PROF_VER_INVALID    0xff

#define TUNING_CTRL_REQUEST        0x00
#define TUNING_CTRL_COMPLETE_D     0x01

#define TUNING_RSP_ACK             0x00
#define TUNING_RSP_NACK            0x01
#define TUNING_RSP_COMPLETE_U      0x03
#define TUNING_RSP_ROLLBACK        0x04

#if defined(TCSUPPORT_CPU_AN7583)
#define EQD_DIFF_THRESHOLD		65535
#else
#define EQD_DIFF_THRESHOLD		255
#endif

typedef enum {
	NG2_ROLLBACK_COM_DS = 0x0001,
	NG2_ROLLBACK_DS_ALBL = 0x0002,
	NG2_ROLLBACK_DS_LKTP = 0x0004,
	NG2_ROLLBACK_US_ALBL = 0x0008,
	NG2_ROLLBACK_US_VOID = 0x0010,
	NG2_ROLLBACK_US_TUNR = 0x0020,
	NG2_ROLLBACK_US_LKTP = 0x0040,
	NG2_ROLLBACK_US_LNRT = 0x0080,
	NG2_ROLLBACK_US_LNCD = 0x0100,
} NG2_PLOAM_TURNING_ROLLBACK_CODE_t ;

#define PROTECT_ENABLE   0
#define PROTECT_DISABLE  1

typedef enum {
	XGPON_PLOAM_ACK_OK = 0,
	XGPON_PLOAM_ACK_NO_MSG,
	XGPON_PLOAM_ACK_BUSY,
	XGPON_PLOAM_ACK_UNKNOWN_MSG,
	XGPON_PLAOM_ACK_PARAM_ERR,
	XGPON_PLOAM_ACK_PROCES_ERR,
} XGPON_PLOAM_ACK_CODE_t ;

typedef struct {	
	uint            allocId;       /* assigned allocId value*/
	unchar          allocIdType;   /* allocId config type*/	
} AllocId_Config_t;

typedef struct {	
    uint            keyIndex;
    unchar          keyControl;	
    unchar          seqNo ;
    unchar          dataEncryptedKey[GPON_DATA_ENCRYPT_KEY_LENS];
} Key_Report_Config_t;


typedef int (*ploam_recv_handler_t)(PLOAM_RAW_General_T *pPloamMsg) ;

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
int  ploam_parser_down_message(PLOAM_RAW_General_T *pGenPloamMsg);
void ploam_init(void);
void ploam_deinit(void);
void ploam_send_serial_number_msg(void);
void ploam_send_registration_msg(unchar seqNo, unchar *regId);
void ploam_send_key_report_msg(unchar seqNo, unchar type, unchar index, unchar num, unchar *key, unchar keyLen);
void ploam_send_acknowledge_msg(unchar seqNo, unchar completionCode);
void ploam_send_tuning_resp_msg(unchar seqNo, unchar operCode,uint respCode);
int ploam_recv_asb_priv_msg(PLOAM_RAW_General_T *pGenPloamMsg);

#endif /*_GPON_PLOAM_H*/
