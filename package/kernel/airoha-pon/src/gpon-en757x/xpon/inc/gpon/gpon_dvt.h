#ifndef _GPON_DVT_H_
#define _GPON_DVT_H_


int gpon_dvt_gemport(int times) ;
int gpon_dvt_onu_connection(int times) ;
int gpon_dvt_transceiver_on_off_connection(int times) ;
int gpon_dvt_pcp_auto_testing(void) ;
int gpon_dvt_pcp_check(unchar mode, struct sk_buff *skb) ;
int gpon_dvt_eqd_adjustment(uint neqEqd) ;
int gpon_dvt_tod(void);

#ifdef TCSUPPORT_CPU_EN7521
int gpon_dvt_aes_key_switch_by_sw(void);
int gpon_dvt_sw_reset(void);
int gpon_dvt_sw_resync(void);
#endif

#endif /* _GPON_DVT_H_ */

