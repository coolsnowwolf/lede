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
#ifndef _XPON_DEBUG_H
#define _XPON_DEBUG_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/jiffies.h>	

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
extern int drop_print_flag;
extern int xpon_mac_print_open;
extern int ng2_mon_not_gnt ;
extern int ng2_tun_resp_key;
extern int ng2_o4_to_09;
extern int ng2_o8_to_05;
extern int ng2_no_rollback;
extern int ng2_man_set_09;
extern int ng2_ignore_disable;

/* JIFFIES_PADDING can let printk show jiffies in mili-seconds */
#if 0//HZ == 100
	#define JIFFIES_PADDING		"0ms"
#else
	#define JIFFIES_PADDING		""
#endif
/*******************XPON************************/
#define PON_MSG(level, F, B...)	{ \
										if(gpPonSysData->debugLevel & level) { \
											printk("[%lu" JIFFIES_PADDING "]" F , jiffies, ##B) ; \
										} \
									}
#define XPON_DPRINT_MSG_RAW(F, B...) do{ \
                                            printk("%s:%d " F ,  __FUNCTION__, __LINE__, ##B) ;\
                                }while(0)
#define XPON_DPRINT_MSG(F, B...) if(xpon_mac_print_open)  XPON_DPRINT_MSG_RAW(F, ##B) ;
#define XPON_DROP_MSG(F, B...) if(drop_print_flag) XPON_DPRINT_MSG_RAW(F, ##B)

#define XPON_DROP_PRINT XPON_DROP_MSG("!!!!XPON SPLIT DEBUG!!!====>%s:%d\n", __FUNCTION__, __LINE__)

/*************************************************/

#define GPON_PLOAM_MSG_RAW(level, F,a, n)	{ \
												if(gpPonSysData->debugLevel & level) { \
												    uint i = 0; \
													printk("\n[%lu" JIFFIES_PADDING "]" F, jiffies) ; \
													for(i = 0 ;i < n ; i++){ \
														printk(" %.8X",htonl(a[i])); \
													} \
													printk("\n"); \
												} \
											}
/*************************************************/
#define XPON_ARR_PRINT(level,F,B,dat,len) { \
                if(gpPonSysData->debugLevel & level) { \
					uint i =0; \
					printk(F); \
					for(i = 0; i <len; i++){ \
						printk(B,dat[i]); \
					} \
					printk("\n"); \
				} \
}

/*************************************************/
#define XPON_ARR_PRINT_TEST(F,B,dat,len) { \
					uint i =0; \
					printk(F); \
					for(i = 0; i <len; i++){ \
						printk(B,dat[i]); \
					} \
					printk("\n"); \
}

/************************************************************************
*               M A C R O S
*************************************************************************
*/
#define XPON_SUCCESS 0
#define XPON_FAIL -1

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

#endif /*_XPON_DEBUG_H*/
