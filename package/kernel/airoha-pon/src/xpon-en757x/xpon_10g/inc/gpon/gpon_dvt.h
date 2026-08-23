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
#ifndef _GPON_DVT_H
#define _GPON_DVT_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define PLOAM_TEST_QUEUE_LEN 128

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
typedef struct{
	uint data[PLOAM_TEST_QUEUE_LEN];
	int front,rear;
}ploam_queue_t;

typedef struct {
	ushort		gemPortId ;
	unchar		isEncrypted ;	//0:not encrypted, 1:encrypted
	unchar 		isType ;        /*0:broadcaset, 1:unicast*/
	unchar		isValid ;		//0:invalid, 1:valid	
} GPONDEV_GemPortInfo_T ;

typedef struct {	
	ushort		tableAddr;
	ushort		writeData ;	
	ushort 		readData ;   
} GPONDEV_GPIDIndexInfo_T ;

typedef struct {	
	unchar		isEncrypted ;	//0:not encrypted, 1:encrypted
	unchar 		isType ;        /*0:broadcaset, 1:unicast*/
	unchar		isValid ;		//0:invalid, 1:valid	
	unchar 		none;
} GPON_GemPortInfo_T ;

typedef struct {	
	uint hec_3err_enable ;
	uint hec_3err_hlend_old ;
	uint hec_3err_hlend_new ;
	uint hec_3err_alloc_old ;
	uint hec_3err_alloc_new ;
} GPON_10G_HEC3ERR_T;

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern ploam_queue_t *ploam_test_queue ;



/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
int gpon_dvt_gemport(int times);
void gpon_index_table(int times);

void gpon_aec_cmac_test(void);
void gponGoldenDsOmciMicCheck(unchar cmacIndex);
void gponGoldenDsPloamMicCheck(unchar cmacIndex);
void gponGoldenUsPloamMicCheck(unchar cmacIndex);
void gponGoldenKeyReportingCheck(unchar cmacIndex);
void gponGoldenNormalEncryptCheck(unchar cmacIndex);
void gpon_rxhec3err_statistic_test(void);


int gpon_dvt_init(void);
int xgpon_register_test(uint testNum);
int xgpon_power_saving_test(void);
int gpon_dvt_sw_reset(void);
int gponDevDumpCsr(void);

#endif /*_GPON_DVT_H*/
