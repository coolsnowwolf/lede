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
#include <linux/timer.h>
#include "../pwan/xpon_netif.h"

#ifndef _OMCI_OAM_MONITOR_H_
#define _OMCI_OAM_MONITOR_H_

typedef enum{
    MONITOR_STOP        ,
    MONITOR_RUNNING     ,
    MONITOR_DONE        ,
}OMCI_OAM_Monitor_State_t ;

typedef struct {
	unsigned int max_diff_cnt   ;
	unsigned int rx_pkt_lmt    ;
	unsigned int timer_interval;
    unsigned int max_wait_time ;
    unsigned int max_timeout_cnt ;
    unsigned int max_no_oam_wait_time ;
    unsigned int max_no_oam_timeout_cnt ;
}Omci_Oam_Monitor_Params_t;

#define MAX_ARRAY_NUM					30

typedef struct {
    Omci_Oam_Monitor_Params_t gpon_params;
    Omci_Oam_Monitor_Params_t epon_params;

    unsigned char pkt_ever_arrived;
    unsigned int  timeout_cnt;
    unsigned int  diff_idx;
    unsigned int  pkt_cnt_diff[MAX_ARRAY_NUM];
	OMCI_OAM_Monitor_State_t run_state;
	unsigned long last_pkt_cnt;
    unsigned long total_diff;
	struct timer_list xponCntRxPkt;
} Omci_Oam_Monitor_t;

void stop_omci_oam_monitor(void) ;
void start_omci_oam_monitor(void);
void omci_oam_monitor_init(Omci_Oam_Monitor_t * monitor_data_ptr) ;
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
ssize_t xpon_omci_oam_monitor_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t xpon_omci_oam_monitor_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
#else
int xpon_omci_oam_monitor_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
int xpon_omci_oam_monitor_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
#endif
int is_really_up(void);
#endif /* _OMCI_OAM_MONITOR_H_ */

