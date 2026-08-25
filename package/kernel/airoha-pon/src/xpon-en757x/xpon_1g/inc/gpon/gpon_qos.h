#ifndef _GPON_QOS_H_
#define _GPON_QOS_H_

void gpon_save_cpu_protection(void);
void gpon_enable_cpu_protection(uint pkt_limit);
void gpon_restore_cpu_protection(void);

int gpon_qos_init(void) ;
int gpon_qos_deinit(void) ;

#endif /* _GPON_QOS_H_ */

