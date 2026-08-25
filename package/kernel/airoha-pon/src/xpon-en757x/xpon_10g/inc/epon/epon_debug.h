/***************************************************************
Copyright Statement:

This software/firmware and related documentation (“EcoNet Software”) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (“EcoNet”) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (“ECONET SOFTWARE”) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN “AS IS” 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER’S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER’S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _EPON_DEBUG_H_
#define _EPON_DEBUG_H_


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/types.h>



/*************************PUBLIC PRINT******************************************/

#define XPON_DEBUG_PRINT_MAX_CHARS 128	
#define XPON_DEBUG_PRINT_MAX_LINE  5024	

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CHECK_DEBUG_LEVEL(level_current, level_modules) ((level_current)<=(level_modules)?TRUE:FALSE)
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

typedef struct
{
	ktime_t time_info;
	char   	debug_info[XPON_DEBUG_PRINT_MAX_CHARS];
}XPON_PRINT, *pXPON_PRINT;

typedef struct
{
    XPON_PRINT 	*pInput;
    XPON_PRINT 	*pOutput;
    struct timer_list 		xpon_debug_output_timer;
    unsigned long 			xpon_debug_print_flag;
    char 					xpon_debug_temp[XPON_DEBUG_PRINT_MAX_CHARS];
    XPON_PRINT 				xpon_debug_total_mem[XPON_DEBUG_PRINT_MAX_LINE];
}XPON_DEBUG_PRINT_DATA,*pXPON_DEBUG_PRINT_DATA;


typedef enum
{
    XPON_DEBUG_COMMON,
	XPON_DEBUG_GPON,
    XPON_DEBUG_EPON,
    XPON_DEBUG_XMCS,
    XPON_DEBUG_PWAN
}XPON_DEBUG_MODULES_TYPE;


typedef enum
{
	MSG_LEVEL_ERROR,
    MSG_LEVEL_INTERRUPT,
    MSG_LEVEL_NOTIFY,
    MSG_LEVEL_DEBUG,
    MSG_LEVEL_TRACE
}EPON_DEBUG_LEVEL;

typedef struct {
		char *name;
		void (*func)(uint, uint, uint);
} epon_cmds_t;


void epon_debug_print_init(void);
void  xpon_debug_print(char *fmt,...);

extern char epon_debug_level;
#define DEBUG_MSG_EPON(level,F,B...)	{ \
											if(epon_debug_level >= level) { \
												xpon_debug_print(F ,##B) ; \
											} \
										}
void show_onu_mac_addr(__u8 *addr);
void show_oam_counters(void);
void show_mpcp_tx_rx_counters(void);
void show_current_hw_disc_state(__u8 llid_index);
void epon_mac_table_init(void);
void show_all_mac_regs(void);

extern char hw_llid_state[3][16];
extern struct timer_list eponTxRate;

void epon_Tx_Rate_expires(TIMER_FUN_PAAM data);

#endif

