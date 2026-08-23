#ifndef _GPON_ACT_H_
#define _GPON_ACT_H_

#include "xmcs/xmcs_const.h"
#include "xpon_ioctl_if.h"
#include <linux/timer.h>
#include <linux/version.h>

#ifndef TIMER_FUN_PAAM
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#define TIMER_FUN_PAAM unsigned long
#else
#define TIMER_FUN_PAAM struct timer_list *
#endif
#endif

void gpon_act_deactive_onu(void) ;
void gpon_act_to1_timer_expires(TIMER_FUN_PAAM arg) ;
void gpon_act_to2_timer_expires(TIMER_FUN_PAAM arg) ;
void gpon_act_hardware_timer(TIMER_FUN_PAAM arg);
void gpon_act_change_gpon_state(const ENUM_GponState_t state) ;
uint gpon_act_get_gpon_state(void) ;
void gpon_act_led_config(unsigned long arg);
int gpon_act_init(void) ;
int gpon_act_deinit(void);


#endif /* _GPON_ACT_H_ */

