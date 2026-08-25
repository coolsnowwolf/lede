/***************************************************************
Copyright Statement:

This software/firmware and related documentation (��EcoNet Software��) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (��EcoNet��) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (��ECONET SOFTWARE��) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ��AS IS�� 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER��S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER��S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/


#ifndef _ECNT_TIMER_H_
#define _ECNT_TIMER_H_

#define TIMER_ENABLE         1
#define TIMER_DISABLE        0
#define TIMER_TOGGLEMODE     1
#define TIMER_INTERVALMODE   0
#define TIMER_TICKENABLE     1
#define TIMER_TICKDISABLE    0
#define TIMER_WDENABLE       1
#define TIMER_WDDISABLE      0
#define TIMER_HALTENABLE     1
#define TIMER_HALTDISABLE    0

#define TIMERTICKS_1MS       1
#define TIMERTICKS_10MS      10  // set timer ticks as 10 ms
#define TIMERTICKS_100MS     100
#define TIMERTICKS_1S        1000
#define TIMERTICKS_10S       10000

#define ENABLE          1
#define DISABLE         0

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_AN7552)
#define NUM_TIMERS      (8)
#define NUM_WDOGS       (4)
#else
#define NUM_TIMERS      (4)
#define NUM_WDOGS       (1)
#endif

extern void wdog_kick(void);
#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_CPU_AN7552)
#include <linux/interrupt.h>

extern int arht_timer_request_irq(int tmr_no, irq_handler_t handler);
extern void arht_timer_free_irq(int tmr_no);
extern int arht_timer_intr_sts_check(int tmr_no);
extern int arht_timer_intr_sts_clear(int tmr_no);
extern int arht_irq_to_tmrNo(int irq, void* dev_id);
extern int is_arht_timer_en(int tmr_no);
extern void arht_timer_en_set(int tmr_no, int tmr_en);
extern void arht_wdog_en_set(int wdog_no, int wdog_en);
extern unsigned int arht_msec_to_ticks (unsigned int msec, unsigned int clk_freq/*MHz*/);
extern void arht_timer_load_ms_set(int tmr_no, unsigned int msec);
extern void arht_wdog_thsld_ms_set(int wdog_no, unsigned int msec);
extern void arht_wdog_reload(int wdog_no);
#endif

#endif /* _ECNT_TIMER_H_ */

