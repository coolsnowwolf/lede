#ifndef _GPON_ACT_H_
#define _GPON_ACT_H_

#include <xmcs/xmcs_const.h>

void gpon_act_deactive_onu(void) ;
void gpon_act_to1_timer_expires(unsigned long arg) ;
void gpon_act_to2_timer_expires(unsigned long arg) ;
void gpon_act_change_gpon_state(const ENUM_GponState_t state) ;
uint gpon_act_get_gpon_state(void) ;
void gpon_act_led_config(void);
int gpon_act_init(void) ;
int gpon_act_deinit(void);


#endif /* _GPON_ACT_H_ */

